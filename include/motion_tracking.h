// motion_tracking.h — kernlogica van het project.
//
// Sensor-onafhankelijk opgezet: de rest van dit systeem werkt met
// "er is een beweging geregistreerd" (via de PIR-ISR). Een toekomstige
// LD2410 kan dezelfde interne interface aanroepen zonder de
// leer-/alarmlogica te wijzigen.
//
// Blijft zelf ontkoppeld van Telegram: dit bestand bepaalt WANNEER er
// een melding moet, main.cpp bepaalt de exacte berichttekst en
// verstuurt 'm (zie motion.hasPendingNotification /
// motion.pendingNotificationKind hieronder).
#pragma once
#include <Arduino.h>
#include "pinout.h"
#define LIVE_LOG_EVENTS 50
struct LiveEvent {
    time_t timestamp = 0;
    bool used = false;
};

// Welk type conditie de eerstvolgende melding veroorzaakt — main.cpp
// gebruikt dit om de juiste berichttekst te kiezen. Bepaalt NIET de
// cap/cooldown-logica zelf, dat blijft hier in motion_tracking.
enum class NotificationKind : uint8_t {
    NONE,
    BLOCK_ALARM,
    FLAT_SAFETY_NET,
    BOOTSTRAP_FALLBACK
};

struct MotionState {
    // Live telling van het huidige, nog niet afgesloten blok.
    uint16_t currentBlockCount = 0;
    int currentDay = -1;    // -1 = nog niet geïnitialiseerd
    int currentBlock = -1;

    // Alarmopbouw (afwijkende/"stille" blokken op rij, severity-som).
    uint16_t silentStreakSum = 0;
    uint8_t silentStreakLen = 0;
    bool blockAlarmActive = false;

    // Vlak vangnet (permanent, ALGORITHM.md §8).
    time_t lastMovementEpoch = 0;
    bool flatAlarmActive = false;

    // Bootstrap-fallback (tijdelijk per weekdag, ALGORITHM.md §7).
    uint8_t consecutiveZeroBlocks = 0;
    bool bootstrapFallbackActive = false;

    // Onrust: welke van de laatste 7 (verwerkte) dagen afweken.
    bool deviatingDayFlags[NUM_DAYS] = {false};
    bool onrustActive = false;
    uint32_t runningDayTotal = 0;

    // --- Meldingspoort: cap van 3, cooldown, rest mode (ALGORITHM.md §5b/§5d) ---
    uint8_t notificationCount = 0;          // 0-3, meldingen binnen de huidige episode
    uint16_t cooldownBlocksRemaining = 0;   // afgeteld per bloksluiting
    bool restMode = false;
    time_t lastWeeklyReassuranceEpoch = 0;  // voor de wekelijkse rest-mode-melding

    // Reboot-/hervat-fix (ALGORITHM.md §6/§5d): het eerstvolgende af te
    // sluiten blok telt niet mee voor de baseline — waar bij setup()
    // altijd, en opnieuw gezet zodra rest mode wordt verlaten.
    bool firstBlockAfterBoot = true;

    // Uitgaande melding, door main.cpp op te halen en te versturen.
    bool hasPendingNotification = false;
    NotificationKind pendingNotificationKind = NotificationKind::NONE;
    bool hasPendingWeeklyReassurance = false;
};
extern MotionState motion;
extern LiveEvent liveLog[LIVE_LOG_EVENTS];

void motionTrackingSetup();
void motionTrackingLoop();   // detecteert PIR-events, blok-overgangen, vangnet, LED

// Automatisch bepaalde of overschreven severity (0-3) voor een blok,
// gerangschikt binnen dezelfde weekdag (ALGORITHM.md §4A).
int severityForBlock(int day, int block);

// Severity (0-3) van een blok gerangschikt over ALLE dagen samen,
// ongeacht weekdag (ALGORITHM.md §4B) — puur diagnostisch, voor
// naast-elkaar-weergave op de Status-tab. Past geen severityOverride
// toe (die geldt alleen voor de per-weekday-view die het alarm stuurt).
int severityForBlockAllDays(int day, int block);

// Voor de webinterface: geaggregeerd gemiddelde van een cel (0 als
// nog geen enkele week gevuld is).
float baselineAverage(int day, int block);

// Formatteert de live-eventlog als tekst (nieuwste bovenaan), voor de
// Logging-tab.
String liveLogGetRecent();