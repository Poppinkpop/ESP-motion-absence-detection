#include "motion_tracking.h"
#include "config.h"
#include "time_sync.h"
#include "debug_log.h"

MotionState motion;
LiveEvent liveLog[LIVE_LOG_EVENTS];
static uint8_t liveLogHead = 0;

// Een blok telt als "afwijkend/stil" voor de severity-alarmsom als de
// telling onder deze fractie van het historisch gemiddelde van die
// cel blijft (DETECTION_METHOD.md §5a — bewust géén strikte 0-eis).
static const float DEVIATION_RATIO_THRESHOLD = 0.3f;

// Onrust: hoeveel standaarddeviaties een dagtotaal van het gemiddelde
// mag afwijken voordat die dag als "afwijkend" telt.
static const float ONRUST_STDDEV_FACTOR = 1.5f;

// Bootstrap-fallback: aantal opeenvolgende blokken met exact 0 ticks
// dat nodig is (DETECTION_METHOD.md §7 — dit IS wel een strikte 0-eis,
// anders dan §5a, want dit is de eenvoudige tijdelijke vangnetregel
// voor een weekdag zonder genoeg geschiedenis).
static const uint8_t BOOTSTRAP_FALLBACK_BLOCKS = 4;

// Cap: maximaal aantal meldingen per episode, en cooldown-lengte
// daartussen (DETECTION_METHOD.md §5b).
static const uint8_t MAX_NOTIFICATIONS_PER_EPISODE = 3;
static const uint16_t NOTIFICATION_COOLDOWN_BLOCKS = 3;

// Wekelijkse geruststellingsmelding tijdens rest mode (DETECTION_METHOD.md §5d).
static const time_t WEEKLY_REASSURANCE_INTERVAL_SEC = 7L * 24 * 3600;

// Status-LED: hoe lang ze aanblijft per tick (puur visueel, geen
// invloed op de telling zelf).
static const unsigned long STATUS_LED_ON_MS = 150;

volatile uint32_t pirIsrCount = 0;
static uint32_t lastSeenIsrCount = 0;
static volatile unsigned long lastIsrMillis = 0;
static unsigned long ledOffAtMillis = 0;

// Bewaakt of lastMovementEpoch al eenmalig is geïnitialiseerd op een
// betrouwbare (NTP-gesynchroniseerde) tijd. Vóór NTP-sync heeft
// time(nullptr) een zinloze waarde (vlak na 1-1-1970); zou die waarde
// in motion.lastMovementEpoch belanden, dan springt silentSeconds in
// motionTrackingLoop() bij de eerste geldige tijd ineens naar
// "decennia geleden" en vuurt het vlakke vangnet meteen na boot af.
// Daarom wordt lastMovementEpoch hier pas gezet, niet in
// motionTrackingSetup() (die draait vóórdat NTP klaar kan zijn).
static bool timeInitializedOnce = false;

// Minimale tijd tussen twee geregistreerde bewegingen. Zonder deze
// debounce kan elektrische ruis/snelle retriggering op de PIR-uitgang
// honderden tellingen opleveren voor één enkele fysieke beweging
// (waargenomen: ~1000 tellingen bij 3x met de hand zwaaien).
static const unsigned long PIR_DEBOUNCE_MS = 1000;

// --- PIR interrupt: zo kort mogelijk, geen logging/String-werk hier ---
static void IRAM_ATTR pirIsr() {
    unsigned long now = millis();
    if (now - lastIsrMillis >= PIR_DEBOUNCE_MS) {
        pirIsrCount++;
        lastIsrMillis = now;
    }
}

void motionTrackingSetup() {
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirIsr, RISING);
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    // lastMovementEpoch wordt hier bewust NIET gezet — dat gebeurt pas
    // in motionTrackingLoop() zodra timeAvailable voor het eerst true
    // is (zie timeInitializedOnce hierboven).
    debugLog("motion_tracking gestart (PIR op pin " + String(PIR_PIN) +
             ", status-LED op pin " + String(STATUS_LED_PIN) + ")");
}

static void pushLiveEvent(time_t t) {
    liveLog[liveLogHead].timestamp = t;
    liveLog[liveLogHead].used = true;
    liveLogHead = (liveLogHead + 1) % LIVE_LOG_EVENTS;
}

// Geeft severity 0-3 voor een blok, of de handmatige override.
// Rangorde binnen dezelfde weekdag (DETECTION_METHOD.md §4, variant A)
// — dit is de severity die de alarmsom aanstuurt.
int severityForBlock(int day, int block) {
    CellBaseline &cell = config.cells[day][block];
    if (cell.severityOverride >= 0 && cell.severityOverride <= 3) {
        return cell.severityOverride;
    }

    // Kwartiel van dit blok t.o.v. de andere 5 blokken van dezelfde
    // weekdag, op basis van het gemiddelde aantal bewegingen.
    float averages[NUM_BLOCKS];
    for (int b = 0; b < NUM_BLOCKS; b++) {
        averages[b] = baselineAverage(day, b);
    }

    // Simpele rangorde-bepaling (NUM_BLOCKS is klein: 6, dus O(n^2) prima).
    int rank = 0;
    for (int b = 0; b < NUM_BLOCKS; b++) {
        if (b != block && averages[b] < averages[block]) rank++;
    }
    // rank 0..5 -> severity 0..3 (kwartielen)
    int severity = (rank * 4) / NUM_BLOCKS;
    if (severity > 3) severity = 3;
    return severity;
}

// Severity (0-3) gerangschikt over ALLE 42 cellen samen, ongeacht
// weekdag (DETECTION_METHOD.md §4, variant B) — alleen voor de
// diagnostische naast-elkaar-weergave op de Status-tab, stuurt geen
// alarm aan en past geen severityOverride toe.
int severityForBlockAllDays(int day, int block) {
    float target = baselineAverage(day, block);
    int rank = 0;
    const int totalCells = NUM_DAYS * NUM_BLOCKS;
    for (int d = 0; d < NUM_DAYS; d++) {
        for (int b = 0; b < NUM_BLOCKS; b++) {
            if (d == day && b == block) continue;
            if (baselineAverage(d, b) < target) rank++;
        }
    }
    int severity = (rank * 4) / totalCells;
    if (severity > 3) severity = 3;
    return severity;
}

float baselineAverage(int day, int block) {
    CellBaseline &cell = config.cells[day][block];
    uint32_t sum = 0;
    uint8_t n = 0;
    for (int w = 0; w < NUM_WEEKS; w++) {
        if (cell.filled[w]) {
            sum += cell.weekCounts[w];
            n++;
        }
    }
    if (n == 0) return 0.0f;
    return (float)sum / (float)n;
}

// Zolang een weekdag nog geen 3 weken heeft waarin alle 6 blokken
// gevuld zijn, zit die weekdag nog in de bootstrapperiode
// (DETECTION_METHOD.md §3/§7).
static bool weekdayInBootstrap(int day) {
    uint8_t filledWeeks = 0;
    for (int w = 0; w < NUM_WEEKS; w++) {
        bool allFilled = true;
        for (int b = 0; b < NUM_BLOCKS; b++) {
            if (!config.cells[day][b].filled[w]) { allFilled = false; break; }
        }
        if (allFilled) filledWeeks++;
    }
    return filledWeeks < 3;
}

// Berekent gemiddelde + standaarddeviatie van de dagtotalen van een
// weekdag over de gevulde weken (alleen weken waarin alle 6 blokken
// gevuld zijn tellen mee, anders is de vergelijking scheef).
static void dayTotalStats(int day, float &meanOut, float &stddevOut, uint8_t &countOut) {
    float weekTotals[NUM_WEEKS];
    uint8_t n = 0;

    for (int w = 0; w < NUM_WEEKS; w++) {
        bool allFilled = true;
        uint32_t total = 0;
        for (int b = 0; b < NUM_BLOCKS; b++) {
            CellBaseline &cell = config.cells[day][b];
            if (!cell.filled[w]) { allFilled = false; break; }
            total += cell.weekCounts[w];
        }
        if (allFilled) {
            weekTotals[n] = (float)total;
            n++;
        }
    }

    countOut = n;
    if (n == 0) { meanOut = 0; stddevOut = 0; return; }

    float sum = 0;
    for (uint8_t i = 0; i < n; i++) sum += weekTotals[i];
    float mean = sum / n;

    float varSum = 0;
    for (uint8_t i = 0; i < n; i++) varSum += (weekTotals[i] - mean) * (weekTotals[i] - mean);
    float stddev = (n > 1) ? sqrt(varSum / (n - 1)) : 0;

    meanOut = mean;
    stddevOut = stddev;
}

// --- Meldingspoort (DETECTION_METHOD.md §5b/§5d) --------------------
//
// Bepaalt welk type conditie op dit moment de episode zou aansturen
// (prioriteit: blok-alarm > bootstrap-fallback > vlak vangnet). Puur
// om main.cpp te vertellen welke berichttekst te kiezen — bepaalt zelf
// niets over of er nu wel/niet verstuurd wordt.
static NotificationKind currentEpisodeKind() {
    if (motion.blockAlarmActive) return NotificationKind::BLOCK_ALARM;
    if (motion.bootstrapFallbackActive) return NotificationKind::BOOTSTRAP_FALLBACK;
    if (motion.flatAlarmActive) return NotificationKind::FLAT_SAFETY_NET;
    return NotificationKind::NONE;
}

// Meldingsnummer 1 van de episode: direct versturen zodra een van de
// alarmvlaggen voor het eerst aangaat (geen cooldown vooraf nodig).
// Retourneert true als hij daadwerkelijk een melding heeft ingepland.
static bool fireFirstNotificationIfNeeded() {
    if (motion.restMode) return false;
    if (motion.notificationCount != 0) return false; // al een episode bezig
    NotificationKind kind = currentEpisodeKind();
    if (kind == NotificationKind::NONE) return false;

    motion.pendingNotificationKind = kind;
    motion.hasPendingNotification = true;
    motion.notificationCount = 1;
    motion.cooldownBlocksRemaining = NOTIFICATION_COOLDOWN_BLOCKS;
    debugLog("*** MELDING 1/" + String(MAX_NOTIFICATIONS_PER_EPISODE) + " (episode start) ***");
    return true;
}

// Aangeroepen bij elke bloksluiting (dus in "blokken" geteld, zoals
// DETECTION_METHOD.md §5b voorschrijft): telt de cooldown af, en stuurt
// de volgende melding zodra die op nul staat — tot de cap van 3, waarna
// het systeem in rest mode gaat i.p.v. een 4e melding te sturen.
static void tickCooldownAndMaybeEscalate() {
    if (motion.restMode || motion.notificationCount == 0) return;
    NotificationKind kind = currentEpisodeKind();
    if (kind == NotificationKind::NONE) return; // episode al voorbij zonder motion-reset; niets te doen

    if (motion.cooldownBlocksRemaining > 0) {
        motion.cooldownBlocksRemaining--;
        return;
    }

    if (motion.notificationCount < MAX_NOTIFICATIONS_PER_EPISODE) {
        motion.pendingNotificationKind = kind;
        motion.hasPendingNotification = true;
        motion.notificationCount++;
        motion.cooldownBlocksRemaining = NOTIFICATION_COOLDOWN_BLOCKS;
        debugLog("*** MELDING " + String(motion.notificationCount) + "/" +
                  String(MAX_NOTIFICATIONS_PER_EPISODE) + " ***");
    } else {
        motion.restMode = true;
        motion.lastWeeklyReassuranceEpoch = time(nullptr);
        debugLog("*** REST MODE: " + String(MAX_NOTIFICATIONS_PER_EPISODE) +
                  " meldingen verstuurd zonder reactie — leren gepauzeerd tot eerste beweging ***");
    }
}

// Sluit het huidige blok af: slaat de telling op in de round-robin
// baseline (tenzij dit blok wordt uitgesloten, zie hieronder),
// beoordeelt of het blok afwijkend was, werkt de alarmteller en de
// bootstrap-fallback-teller bij, voedt de meldingspoort, en bij het
// laatste blok van de dag: onrustcheck.
static void finalizeBlock(int day, int block, uint16_t count) {
    CellBaseline &cell = config.cells[day][block];

    // Gemiddelde vóór het eventueel overschrijven, want we willen dit
    // blok vergelijken met de geschiedenis, niet met zichzelf.
    float avgBefore = baselineAverage(day, block);
    bool haveHistory = avgBefore > 0.0f;

    bool deviating = haveHistory && (count < avgBefore * DEVIATION_RATIO_THRESHOLD);

    // --- Opslaan in round-robin slot — twee onafhankelijke
    // uitsluitingsredenen (DETECTION_METHOD.md §5d/§6), allebei
    // beoordeeld met de state zoals die STOND VÓÓR de evaluatie van dit
    // blok hieronder (dus vóór eventuele alarm-/meldingsupdates die dit
    // blok zelf veroorzaakt):
    //
    // 1. Eerste blok na een boot of na het verlaten van rest mode —
    //    een deelmeting, mag de baseline niet vertekenen.
    // 2. Er liep al een episode (notificationCount>0 of restMode) vóór
    //    dit blok begon — dit blok hoort dus bij een reeds bevestigde
    //    afwezigheidsepisode, niet bij normaal gedrag. Het blok (of de
    //    2 blokken) dat een episode voor het eerst laat afgaan, telt
    //    zelf nog gewoon mee: op het moment van opslaan was er nog geen
    //    bevestigde episode om te wantrouwen. Er wordt nooit iets
    //    achteraf teruggedraaid — de uitsluiting wordt één keer, bij
    //    het schrijven zelf, bepaald (KIS, geen extra state nodig).
    bool episodeAlreadyActive = (motion.notificationCount > 0) || motion.restMode;
    if (!motion.firstBlockAfterBoot && !episodeAlreadyActive) {
        uint8_t slot = currentEpochWeek() % NUM_WEEKS;
        cell.weekCounts[slot] = count;
        cell.filled[slot] = true;
    } else if (motion.firstBlockAfterBoot) {
        debugLog("Blok NIET opgeslagen voor baseline (eerste blok na boot/hervatting, mogelijk deelmeting)");
        motion.firstBlockAfterBoot = false;
    } else {
        debugLog("Blok NIET opgeslagen voor baseline (viel binnen een reeds lopende episode/rustmodus)");
    }

    debugLog("Blok afgesloten: dag=" + String(day) + " blok=" + String(block) +
              " telling=" + String(count) +
              " gem.voor=" + String(avgBefore, 1) +
              (deviating ? " -> AFWIJKEND" : ""));

    // --- Alarmteller (severity-som, DETECTION_METHOD.md §5/§5a) ---
    if (deviating) {
        int sev = severityForBlock(day, block);
        motion.silentStreakSum += sev;
        motion.silentStreakLen++;

        bool triggerNow = false;
        if (motion.silentStreakLen >= 2 && motion.silentStreakSum >= config.alarmThreshold) {
            triggerNow = true;
        }
        // Bij 2 blokken nog niet over de drempel: wacht op het 3e
        // blok (silentStreakLen loopt vanzelf door naar 3).

        if (triggerNow && !motion.blockAlarmActive) {
            motion.blockAlarmActive = true;
            debugLog("*** ALARM: bewegingspatroon wijkt af (som=" +
                      String(motion.silentStreakSum) + ", drempel=" +
                      String(config.alarmThreshold) + ") ***");
        }
    } else {
        // Niet-afwijkend blok (of geen historie om mee te vergelijken)
        // reset de opbouw, zoals bij écht gedetecteerde beweging.
        motion.silentStreakSum = 0;
        motion.silentStreakLen = 0;
        motion.blockAlarmActive = false;
    }

    // --- Bootstrap-fallback (tijdelijk per weekdag, DETECTION_METHOD.md §7) ---
    if (count == 0) {
        motion.consecutiveZeroBlocks++;
    } else {
        motion.consecutiveZeroBlocks = 0;
    }
    if (weekdayInBootstrap(day) &&
        motion.consecutiveZeroBlocks >= BOOTSTRAP_FALLBACK_BLOCKS &&
        !motion.bootstrapFallbackActive) {
        motion.bootstrapFallbackActive = true;
        debugLog("*** BOOTSTRAP-FALLBACK ALARM: " + String(BOOTSTRAP_FALLBACK_BLOCKS) +
                  " blokken op rij zonder beweging, weekdag nog in leerperiode ***");
    } else if (!weekdayInBootstrap(day)) {
        // Deze weekdag heeft nu een severity-profiel; de tijdelijke
        // fallback voor déze weekdag is niet meer van toepassing.
        motion.bootstrapFallbackActive = false;
    }

    // --- Meldingspoort voeden ---
    if (!fireFirstNotificationIfNeeded()) {
        tickCooldownAndMaybeEscalate();
    }

    // --- Onrust: dagtotaal bijwerken en, bij laatste blok, checken ---
    motion.runningDayTotal += count;

    if (block == NUM_BLOCKS - 1) {
        float mean, stddev;
        uint8_t n;
        dayTotalStats(day, mean, stddev, n);

        bool dayDeviating = false;
        if (n >= 2) { // pas zinvol met minimaal een paar referentieweken
            float diff = fabs((float)motion.runningDayTotal - mean);
            float threshold = (stddev > 0) ? (stddev * ONRUST_STDDEV_FACTOR) : (mean * 0.5f);
            dayDeviating = diff > threshold;
        }
        motion.deviatingDayFlags[day] = dayDeviating;

        uint8_t deviatingCount = 0;
        for (int d = 0; d < NUM_DAYS; d++) if (motion.deviatingDayFlags[d]) deviatingCount++;
        motion.onrustActive = (deviatingCount >= config.onrustMinDeviatingDays);

        if (dayDeviating) {
            debugLog("Dagpatroon dag=" + String(day) + " wijkt af (totaal=" +
                      String(motion.runningDayTotal) + " gem=" + String(mean, 1) + ")");
        }
        if (motion.onrustActive) {
            debugLog("*** ONRUST: " + String(deviatingCount) + " van de laatste " +
                      String(NUM_DAYS) + " dagen wijken af ***");
        }

        motion.runningDayTotal = 0;
    }

    configSave();
}

void motionTrackingLoop() {
    // Status-LED uitschakelen staat los van de tijd-check hieronder,
    // zodat een korte flits nooit blijft "hangen" als NTP nog niet
    // gesynchroniseerd is.
    if (ledOffAtMillis != 0 && (long)(millis() - ledOffAtMillis) >= 0) {
        digitalWrite(STATUS_LED_PIN, LOW);
        ledOffAtMillis = 0;
    }

    if (!timeAvailable) return; // zonder tijd geen zinvolle blok-indeling

    // Eenmalige initialisatie zodra de tijd voor het eerst betrouwbaar
    // is: pas hier lastMovementEpoch zetten, niet in
    // motionTrackingSetup() (die draait vóórdat NTP-sync klaar kan
    // zijn). Zonder deze guard zou lastMovementEpoch op de kleine
    // boot-tijd-waarde blijven staan, waardoor silentSeconds hieronder
    // bij de eerste geldige tijd ineens "decennia" zou zijn en het
    // vlakke vangnet meteen na boot afvuurt.
    if (!timeInitializedOnce) {
        motion.lastMovementEpoch = time(nullptr);
        timeInitializedOnce = true;
        debugLog("Tijd beschikbaar; laatste-beweging-tijdstip geïnitialiseerd op " + currentTimeString());
    }

    // --- Nieuwe PIR-events verwerken ---
    noInterrupts();
    uint32_t isrCountNow = pirIsrCount;
    interrupts();

    if (isrCountNow != lastSeenIsrCount) {
        uint32_t newEvents = isrCountNow - lastSeenIsrCount;
        lastSeenIsrCount = isrCountNow;

        time_t now = time(nullptr);
        motion.currentBlockCount += newEvents;
        motion.lastMovementEpoch = now;
        motion.flatAlarmActive = false;

        // Status-LED: 1-op-1 met elke geregistreerde (gedebouncte)
        // tick, simpelste mogelijke gedrag (DETECTION_METHOD.md §11).
        digitalWrite(STATUS_LED_PIN, HIGH);
        ledOffAtMillis = millis() + STATUS_LED_ON_MS;

        // Beweging gedetecteerd -> evaluatie direct resetten (zelfde
        // regel als in het originele projectdocument), inclusief de
        // meldingspoort en, indien van toepassing, rest mode verlaten.
        if (motion.silentStreakLen > 0 || motion.blockAlarmActive ||
            motion.bootstrapFallbackActive || motion.notificationCount > 0 ||
            motion.restMode) {
            motion.silentStreakSum = 0;
            motion.silentStreakLen = 0;
            motion.blockAlarmActive = false;
            motion.bootstrapFallbackActive = false;
            motion.consecutiveZeroBlocks = 0;
            motion.notificationCount = 0;
            motion.cooldownBlocksRemaining = 0;
            if (motion.restMode) {
                motion.restMode = false;
                // Zelfde regel als een reboot: het blok waarin de
                // eerste beweging valt telt niet mee voor de baseline
                // (DETECTION_METHOD.md §5d, hergebruikt §6).
                motion.firstBlockAfterBoot = true;
                debugLog("Rest mode beëindigd: beweging gedetecteerd, systeem hervat");
            }
            debugLogVerbose("Beweging gedetecteerd, alarmopbouw/meldingspoort gereset");
        }

        for (uint32_t i = 0; i < newEvents; i++) {
            pushLiveEvent(now);
        }
        debugLogVerbose("PIR-event(s): " + String(newEvents) + ", blokteller nu " + String(motion.currentBlockCount));
    }

    // --- Blok-/dagovergang detecteren ---
    int day = currentWeekday();
    int block = currentBlock();

    if (motion.currentDay == -1) {
        // Eerste keer na boot: geen vorig blok om af te sluiten.
        motion.currentDay = day;
        motion.currentBlock = block;
    } else if (day != motion.currentDay || block != motion.currentBlock) {
        finalizeBlock(motion.currentDay, motion.currentBlock, motion.currentBlockCount);
        motion.currentBlockCount = 0;
        motion.currentDay = day;
        motion.currentBlock = block;
    }

    // --- Vlak vangnet (permanent, DETECTION_METHOD.md §8) ---
    time_t now = time(nullptr);
    uint32_t silentSeconds = (uint32_t)(now - motion.lastMovementEpoch);
    uint32_t thresholdSeconds = (uint32_t)config.flatSafetyNetHours * 3600UL;
    if (!motion.flatAlarmActive && silentSeconds > thresholdSeconds) {
        motion.flatAlarmActive = true;
        debugLog("*** VLAK VANGNET ALARM: " + String(silentSeconds / 3600) +
                  " uur geen beweging (drempel " + String(config.flatSafetyNetHours) + "u) ***");
        // Vlak vangnet kan mid-blok ontstaan (tijdsgebaseerd, niet
        // blokgebaseerd) — direct de eerste melding aanbieden i.p.v.
        // te wachten op de volgende bloksluiting.
        fireFirstNotificationIfNeeded();
    }

    // --- Wekelijkse geruststellingsmelding tijdens rest mode (§5d) ---
    if (motion.restMode &&
        (now - motion.lastWeeklyReassuranceEpoch) >= WEEKLY_REASSURANCE_INTERVAL_SEC) {
        motion.hasPendingWeeklyReassurance = true;
        motion.lastWeeklyReassuranceEpoch = now;
    }
}

String liveLogGetRecent() {
    String out;
    // Nieuwste eerst.
    for (uint8_t i = 0; i < LIVE_LOG_EVENTS; i++) {
        uint8_t idx = (liveLogHead + LIVE_LOG_EVENTS - 1 - i) % LIVE_LOG_EVENTS;
        if (!liveLog[idx].used) continue;
        struct tm t;
        localtime_r(&liveLog[idx].timestamp, &t);
        char buf[9];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
        out += String(buf) + "\n";
    }
    return out;
 }
