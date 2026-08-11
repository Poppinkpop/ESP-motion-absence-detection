#include "webinterface.h"
#include <ESP8266WebServer.h>
#include "config.h"
#include "motion_tracking.h"
#include "time_sync.h"
#include "wifi_connection.h"
#include "debug_log.h"
#include "telegram_alert.h"

static ESP8266WebServer server(80);

static const char *DAY_NAMES[NUM_DAYS] = {
    "maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag", "zondag"
};

// --- Sensitivity-presets (ALGORITHM.md §5) --------------------------
// De alarmdrempel zelf (config.alarmThreshold) blijft de enige bron
// van waarheid; deze presets zijn puur een gebruiksvriendelijke manier
// om 'm te zetten (KIS: geen los preset-veld ernaast opslaan).
static const uint16_t THRESHOLD_MORE_SENSITIVE = 5;
static const uint16_t THRESHOLD_NORMAL = 6;
static const uint16_t THRESHOLD_LESS_SENSITIVE = 7;

static String sensitivityLabelFor(uint16_t threshold) {
    if (threshold == THRESHOLD_MORE_SENSITIVE) return "More sensitive";
    if (threshold == THRESHOLD_NORMAL) return "Normal";
    if (threshold == THRESHOLD_LESS_SENSITIVE) return "Less sensitive";
    return "Aangepast (" + String(threshold) + ")";
}

static String htmlHeader(const String &title, const String &activeTab) {
    String h = "<!DOCTYPE html><html lang='nl'><head><meta charset='utf-8'>";
    h += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    h += "<title>" + title + "</title><style>";
    h += "body{font-family:sans-serif;margin:1em;background:#f4f4f4;color:#222}";
    h += "table{border-collapse:collapse;width:100%;margin-bottom:1em}";
    h += "th,td{border:1px solid #ccc;padding:4px 8px;text-align:center;font-size:0.9em}";
    h += "th{background:#ddd}";
    h += ".tabs a{margin-right:1em;text-decoration:none;color:#06c;font-weight:bold}";
    h += ".tabs a.active{color:#000;text-decoration:underline}";
    h += ".ok{color:#2a2}.warn{color:#c22;font-weight:bold}.rest{color:#a80;font-weight:bold}";
    h += "pre{background:#fff;border:1px solid #ccc;padding:0.5em;max-height:400px;overflow:auto;font-size:0.85em}";
    h += "fieldset{background:#fff;border:1px solid #ccc;margin-bottom:1em;padding:0.8em}";
    h += "legend{font-weight:bold}label{display:block;margin:0.3em 0}";
    h += "input[type=number]{width:6em}button{margin-top:0.6em;padding:0.4em 1em}";
    h += "small.hint{color:#666}";
    h += "</style></head><body>";
    h += "<div class='tabs'>";
    h += "<a href='/status'" + String(activeTab == "status" ? " class='active'" : "") + ">Status</a>";
    h += "<a href='/settings'" + String(activeTab == "settings" ? " class='active'" : "") + ">Settings</a>";
    h += "<a href='/log'" + String(activeTab == "log" ? " class='active'" : "") + ">Log</a>";
    h += "</div>";
    h += "<h2>" + title + "</h2>";
    return h;
}

static String htmlFooter() {
    return "</body></html>";
}

// --- Status-tab -------------------------------------------------------

static void handleStatus() {
    String h = htmlHeader("esp-motion-absence-detection — Status", "status");
    h += "<meta http-equiv='refresh' content='10'>";

    h += "<p>Tijd: " + (timeAvailable ? currentTimeString() : String("nog geen NTP-sync")) + "</p>";

    // WiFi-status incl. ruwe signaalsterkte (dBm) — alleen op deze tab,
    // geen aparte melding hier; de zwak-signaal-waarschuwing zelf komt
    // via de Log-tab en de Telegram-opstartmelding (zie wifi_connection.cpp
    // resp. main.cpp).
    h += "<p>WiFi: " + String(wifiConnected ? "verbonden" : "NIET verbonden");
    if (wifiConnected) {
        h += " (" + String(wifiRssi()) + " dBm)";
    }
    h += "</p>";

    if (motion.currentDay >= 0) {
        h += "<p>Huidige weekdag/blok: " + String(DAY_NAMES[motion.currentDay]) +
             ", blok " + String(motion.currentBlock) + " (" +
             String(motion.currentBlock * BLOCK_DURATION_HOURS) + "-" +
             String((motion.currentBlock + 1) * BLOCK_DURATION_HOURS) + "u)</p>";
        h += "<p>Live telling dit blok: <b>" + String(motion.currentBlockCount) + "</b></p>";
    }

    unsigned long silentMin = (unsigned long)(time(nullptr) - motion.lastMovementEpoch) / 60;
    h += "<p>Laatste beweging: " + String(silentMin) + " minuten geleden</p>";

    // --- Rest mode: bovenaan en duidelijk, want dit overstemt alle
    // andere statussen hieronder (geen meldingen, geen leren totdat
    // er weer beweging is — ALGORITHM.md §5d).
    if (motion.restMode) {
        h += "<p class='rest'>RUST MODUS actief — 3 meldingen verstuurd zonder reactie op beweging. "
             "Live telling gaat door, maar wordt niet in het geleerde patroon opgeslagen. "
             "Systeem hervat automatisch zodra er weer beweging is.</p>";
    }

    h += "<p>Meldingen in huidige episode: " + String(motion.notificationCount) + "/3";
    if (motion.notificationCount > 0 && !motion.restMode) {
        h += " (volgende over " + String(motion.cooldownBlocksRemaining) + " blok(ken))";
    }
    h += "</p>";

    h += "<p>Alarmopbouw: som=" + String(motion.silentStreakSum) +
         " (" + String(motion.silentStreakLen) + " afwijkende blokken op rij), drempel=" +
         String(config.alarmThreshold) + " (" + sensitivityLabelFor(config.alarmThreshold) + ")</p>";

    h += "<p>Blok-alarm: <span class='" + String(motion.blockAlarmActive ? "warn" : "ok") + "'>" +
         String(motion.blockAlarmActive ? "ACTIEF" : "geen") + "</span></p>";
    h += "<p>Vlak vangnet (" + String(config.flatSafetyNetHours) + "u): <span class='" +
         String(motion.flatAlarmActive ? "warn" : "ok") + "'>" +
         String(motion.flatAlarmActive ? "ACTIEF" : "geen") + "</span></p>";
    h += "<p>Bootstrap-fallback (" + String(config.bootstrapFallbackHours) +
         "u in blokken, tijdelijk per weekdag): <span class='" +
         String(motion.bootstrapFallbackActive ? "warn" : "ok") + "'>" +
         String(motion.bootstrapFallbackActive ? "ACTIEF" : "geen") + "</span></p>";
    h += "<p>Onrust in dagpatronen: <span class='" + String(motion.onrustActive ? "warn" : "ok") + "'>" +
         String(motion.onrustActive ? "ACTIEF" : "geen") + "</span></p>";

    h += "<p><a href='/test_telegram'>Stuur testbericht naar Telegram</a></p>";
    if (server.hasArg("telegram_test")) {
        bool ok = (server.arg("telegram_test") == "ok");
        h += "<p class='" + String(ok ? "ok" : "warn") + "'>" +
             String(ok ? "Testbericht verstuurd — check Telegram." :
                         "Versturen mislukt — check de Log-tab voor de foutmelding.") +
             "</p>";
    }

    // --- Dubbele severity-weergave (ALGORITHM.md §4): per-weekday
    // (stuurt het alarm aan) naast alle-dagen-samen (diagnostisch).
    h += "<h3>Geleerd patroon (gemiddelde per blok, laatste " + String(NUM_WEEKS) + " weken)</h3>";
    h += "<p><small class='hint'>Severity: eerste getal = binnen deze weekdag, tweede = over alle dagen samen (zie ALGORITHM.md §4).</small></p>";
    h += "<table><tr><th>Dag</th>";
    for (int b = 0; b < NUM_BLOCKS; b++) {
        h += "<th>" + String(b * BLOCK_DURATION_HOURS) + "-" + String((b + 1) * BLOCK_DURATION_HOURS) + "u</th>";
    }
    h += "</tr>";
    for (int d = 0; d < NUM_DAYS; d++) {
        h += "<tr><td>" + String(DAY_NAMES[d]) + "</td>";
        for (int b = 0; b < NUM_BLOCKS; b++) {
            float avg = baselineAverage(d, b);
            int sevDay = severityForBlock(d, b);
            int sevAll = severityForBlockAllDays(d, b);
            h += "<td>" + String(avg, 1) + "<br><small>sev " + String(sevDay) +
                 " / " + String(sevAll) + "</small></td>";
        }
        h += "</tr>";
    }
    h += "</table>";
    h += "<p><small>Gemiddelden zijn geaggregeerd over de laatste " + String(NUM_WEEKS) +
         " weken — er wordt bewust geen dag-voor-dag geschiedenis getoond (privacy).</small></p>";

    h += htmlFooter();
    server.send(200, "text/html", h);
}

// --- Settings-tab -------------------------------------------------------

static void handleSettings() {
    String h = htmlHeader("esp-motion-absence-detection — Settings", "settings");

    if (server.hasArg("saved")) {
        h += "<p class='ok'>Instellingen opgeslagen.</p>";
    }

    uint16_t t = config.alarmThreshold;
    h += "<form method='POST' action='/settings/save'>";

    h += "<fieldset><legend>Gevoeligheid</legend>";
    h += "<label><input type='radio' name='sensitivity' value='more'" +
         String(t == THRESHOLD_MORE_SENSITIVE ? " checked" : "") +
         "> More sensitive (drempel " + String(THRESHOLD_MORE_SENSITIVE) + ")</label>";
    h += "<label><input type='radio' name='sensitivity' value='normal'" +
         String(t == THRESHOLD_NORMAL ? " checked" : "") +
         "> Normal (drempel " + String(THRESHOLD_NORMAL) + ", standaard)</label>";
    h += "<label><input type='radio' name='sensitivity' value='less'" +
         String(t == THRESHOLD_LESS_SENSITIVE ? " checked" : "") +
         "> Less sensitive (drempel " + String(THRESHOLD_LESS_SENSITIVE) + ")</label>";
    if (t != THRESHOLD_MORE_SENSITIVE && t != THRESHOLD_NORMAL && t != THRESHOLD_LESS_SENSITIVE) {
        h += "<p><small class='hint'>Huidige drempel is een niet-standaard waarde (" + String(t) +
             "); kies een van bovenstaande opties om terug te gaan naar een preset.</small></p>";
    }
    h += "</fieldset>";

    h += "<fieldset><legend>Vlak vangnet (permanente achtervang)</legend>";
    h += "<label>Uren zonder beweging voordat er sowieso gewaarschuwd wordt:<br>";
    h += "<input type='number' name='flatSafetyNetHours' min='1' max='72' value='" +
         String(config.flatSafetyNetHours) + "'></label>";
    h += "<p><small class='hint'>Standaard 12. Geldt altijd, ook na de leerperiode.</small></p>";
    h += "</fieldset>";

    h += "<fieldset><legend>Bootstrap-fallback (tijdelijk, eerste weken per weekdag)</legend>";
    h += "<label>Uren zonder beweging, in blokken van " + String(BLOCK_DURATION_HOURS) +
         "u, zolang een weekdag nog geen 3 gevulde weken heeft:<br>";
    h += "<input type='number' name='bootstrapFallbackHours' min='4' max='48' step='" +
         String(BLOCK_DURATION_HOURS) + "' value='" + String(config.bootstrapFallbackHours) + "'></label>";
    h += "<p><small class='hint'>Standaard 16 (= 4 blokken). Stopt vanzelf zodra een weekdag een geleerd patroon heeft.</small></p>";
    h += "</fieldset>";

    h += "<button type='submit'>Opslaan</button>";
    h += "</form>";

    h += htmlFooter();
    server.send(200, "text/html", h);
}

static uint16_t clampU16(long v, uint16_t lo, uint16_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return (uint16_t)v;
}

static void handleSettingsSave() {
    String sensitivity = server.arg("sensitivity");
    if (sensitivity == "more") {
        config.alarmThreshold = THRESHOLD_MORE_SENSITIVE;
    } else if (sensitivity == "less") {
        config.alarmThreshold = THRESHOLD_LESS_SENSITIVE;
    } else if (sensitivity == "normal") {
        config.alarmThreshold = THRESHOLD_NORMAL;
    }
    // Onbekende/ontbrekende waarde: bestaande drempel blijft ongewijzigd.

    if (server.hasArg("flatSafetyNetHours")) {
        config.flatSafetyNetHours = clampU16(server.arg("flatSafetyNetHours").toInt(), 1, 72);
    }
    if (server.hasArg("bootstrapFallbackHours")) {
        config.bootstrapFallbackHours = clampU16(server.arg("bootstrapFallbackHours").toInt(), 4, 48);
    }

    configSave();
    debugLog("Instellingen gewijzigd via webinterface: drempel=" + String(config.alarmThreshold) +
              " vlakVangnet=" + String(config.flatSafetyNetHours) +
              "u bootstrapFallback=" + String(config.bootstrapFallbackHours) + "u");

    server.sendHeader("Location", "/settings?saved=1");
    server.send(303);
}

// --- Log-tab -------------------------------------------------------

static void handleLog() {
    String h = htmlHeader("esp-motion-absence-detection — Log", "log");
    h += "<meta http-equiv='refresh' content='3'>";

    h += "<h3>Live PIR-events (vandaag, laatste " + String(LIVE_LOG_EVENTS) + ")</h3>";
    h += "<pre>" + liveLogGetRecent() + "</pre>";

    h += "<h3>Systeemlog</h3>";
    h += "<p>Verbose logging: " + String(verboseLogging ? "AAN" : "UIT") +
         " — <a href='/toggle_verbose'>omschakelen</a></p>";
    h += "<pre>" + debugLogGetRecent() + "</pre>";

    h += htmlFooter();
    server.send(200, "text/html", h);
}

static void handleToggleVerbose() {
    debugLogSetVerbose(!verboseLogging);
    server.sendHeader("Location", "/log");
    server.send(303);
}

static void handleTestTelegram() {
    bool ok = telegramSendMessage(
        "Testbericht vanaf esp-motion-absence-detection — als je dit ontvangt, werkt de Telegram-koppeling correct.");
    server.sendHeader("Location", String("/status?telegram_test=") + (ok ? "ok" : "fail"));
    server.send(303);
}

static void handleRoot() {
    server.sendHeader("Location", "/status");
    server.send(303);
}

void webinterfaceSetup() {
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/settings/save", HTTP_POST, handleSettingsSave);
    server.on("/log", handleLog);
    server.on("/toggle_verbose", handleToggleVerbose);
    server.on("/test_telegram", handleTestTelegram);
    server.begin();
    debugLog("Webinterface gestart op poort 80");
}

void webinterfaceLoop() {
    server.handleClient();
}