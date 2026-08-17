#include "webinterface.h"
#include <ESP8266WebServer.h>
#include "config.h"
#include "motion_tracking.h"
#include "time_sync.h"
#include "wifi_connection.h"
#include "debug_log.h"
#include "telegram_alert.h"
#include "lang.h"

static ESP8266WebServer server(80);

// --- Sensitivity-presets (DETECTION_METHOD.md §5) --------------------------
// De alarmdrempel zelf (config.alarmThreshold) blijft de enige bron
// van waarheid; deze presets zijn puur een gebruiksvriendelijke manier
// om 'm te zetten (KIS: geen los preset-veld ernaast opslaan).
static const uint16_t THRESHOLD_MORE_SENSITIVE = 5;
static const uint16_t THRESHOLD_NORMAL = 6;
static const uint16_t THRESHOLD_LESS_SENSITIVE = 7;

static String sensitivityLabelFor(uint16_t threshold) {
    if (threshold == THRESHOLD_MORE_SENSITIVE) return Lang::sensitivityMoreWord();
    if (threshold == THRESHOLD_NORMAL) return Lang::sensitivityNormalWord();
    if (threshold == THRESHOLD_LESS_SENSITIVE) return Lang::sensitivityLessWord();
    return Lang::sensitivityCustomLabel(threshold);
}

static String htmlHeader(const String &title, const String &activeTab) {
    String h = "<!DOCTYPE html><html lang='" + String(Lang::htmlLangCode()) + "'><head><meta charset='utf-8'>";
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
    // --- Nieuw voor de Status-tab: cards/badges/banner ---
    h += ".card{background:#fff;border:1px solid #ccc;border-radius:6px;padding:0.8em 1em;margin-bottom:0.8em}";
    h += ".card p{margin:0.3em 0}";
    h += ".card-row{display:flex;flex-wrap:wrap;gap:0.8em;margin-bottom:0.8em}";
    h += ".card-row .card{flex:1;min-width:220px;margin-bottom:0}";
    h += ".badges{display:flex;flex-wrap:wrap;gap:0.5em;margin-bottom:1em}";
    h += ".badge{display:inline-block;padding:0.3em 0.7em;border-radius:12px;font-size:0.85em;font-weight:bold}";
    h += ".badge.ok{background:#e6f6e6;color:#2a2}.badge.warn{background:#fde8e8;color:#c22}";
    h += ".banner{padding:0.7em 1em;border-radius:6px;margin-bottom:1em;font-weight:bold}";
    h += ".banner.rest{background:#fff3e0;color:#a80;border:1px solid #e0b070}";
    h += ".banner.episode{background:#fff8e1;color:#8a6d00;border:1px solid #e0c85a}";
    h += "</style></head><body>";
    h += "<div class='tabs'>";
    h += "<a href='/status'" + String(activeTab == "status" ? " class='active'" : "") + ">" + Lang::tabStatus() + "</a>";
    h += "<a href='/settings'" + String(activeTab == "settings" ? " class='active'" : "") + ">" + Lang::tabSettings() + "</a>";
    h += "<a href='/log'" + String(activeTab == "log" ? " class='active'" : "") + ">" + Lang::tabLog() + "</a>";
    h += "</div>";
    h += "<h2>" + title + "</h2>";
    return h;
}

static String htmlFooter() {
    return "</body></html>";
}

// --- Status-tab -------------------------------------------------------

static void handleStatus() {
    String h = htmlHeader(String("esp-motion-absence-detection — ") + Lang::tabStatus(), "status");
    h += "<meta http-equiv='refresh' content='10'>";

    // --- Banners bovenaan: rust-modus overstemt alles (ALGORITHM/
    // DETECTION_METHOD §5d), een lopende meldings-episode krijgt anders
    // ook een banner zodat dat meteen opvalt zonder te scrollen.
    if (motion.restMode) {
        h += "<div class='banner rest'>" + Lang::statusRestModeBanner() + "</div>";
    } else if (motion.notificationCount > 0) {
        h += "<div class='banner episode'>" +
             Lang::statusNotificationLine(motion.notificationCount, motion.cooldownBlocksRemaining, true) +
             "</div>";
    }

    // --- Info-kaarten ---
    h += "<div class='card-row'>";
    h += "<div class='card'>";
    h += "<p>" + Lang::statusTimeLine(timeAvailable, timeAvailable ? currentTimeString() : String()) + "</p>";
    h += "<p>" + Lang::statusWifiLine(wifiConnected, wifiConnected ? wifiRssi() : 0) + "</p>";
    h += "</div>";

    h += "<div class='card'>";
    if (motion.currentDay >= 0) {
        // Weergave-only vertaalslag: motion.currentBlock is intern 0-indexed
        // (array-index), maar wordt hier voor de gebruiker als blok 1..6
        // getoond — de +1 gebeurt uitsluitend hier, nooit in motion_tracking
        // of lang.cpp, om verwarring in de rekenlogica te voorkomen.
        h += "<p>" + Lang::statusCurrentBlockLine(Lang::dayName(motion.currentDay), motion.currentBlock + 1,
                 motion.currentBlock * BLOCK_DURATION_HOURS, (motion.currentBlock + 1) * BLOCK_DURATION_HOURS) + "</p>";
        h += "<p>" + Lang::statusLiveCountLine(motion.currentBlockCount) + "</p>";
    }
    unsigned long silentMin = (unsigned long)(time(nullptr) - motion.lastMovementEpoch) / 60;
    h += "<p>" + Lang::statusLastMovementLine(silentMin) + "</p>";
    h += "</div>";
    h += "</div>"; // card-row

    // Rust-modus toont de episode-regel als platte tekst i.p.v. banner
    // (de rest-banner hierboven vertelt het verhaal al); cooldown-tekst
    // is dan niet van toepassing.
    if (motion.restMode && motion.notificationCount > 0) {
        h += "<p>" + Lang::statusNotificationLine(motion.notificationCount, motion.cooldownBlocksRemaining, false) + "</p>";
    }

    h += "<p>" + Lang::statusAlarmBuildupLine(motion.silentStreakSum, motion.silentStreakLen,
             config.alarmThreshold, sensitivityLabelFor(config.alarmThreshold)) + "</p>";

    // --- Status-badges ---
    h += "<div class='badges'>";
    h += "<span class='badge " + String(motion.blockAlarmActive ? "warn" : "ok") + "'>" +
         Lang::labelBlockAlarm() + String(motion.blockAlarmActive ? Lang::active() : Lang::inactive()) + "</span>";
    h += "<span class='badge " + String(motion.flatAlarmActive ? "warn" : "ok") + "'>" +
         Lang::labelFlatSafetyNet(config.flatSafetyNetHours) + String(motion.flatAlarmActive ? Lang::active() : Lang::inactive()) + "</span>";
    h += "<span class='badge " + String(motion.bootstrapFallbackActive ? "warn" : "ok") + "'>" +
         Lang::labelBootstrapFallback(config.bootstrapFallbackHours) + String(motion.bootstrapFallbackActive ? Lang::active() : Lang::inactive()) + "</span>";
    h += "<span class='badge " + String(motion.onrustActive ? "warn" : "ok") + "'>" +
         Lang::labelOnrust() + String(motion.onrustActive ? Lang::active() : Lang::inactive()) + "</span>";
    h += "</div>";

    // --- Dubbele severity-weergave (DETECTION_METHOD.md §4): per-weekday
    // (stuurt het alarm aan) naast alle-dagen-samen (diagnostisch).
    h += "<h3>" + Lang::statusLearnedPatternHeading(NUM_WEEKS) + "</h3>";
    h += "<p><small class='hint'>" + Lang::statusSeverityHint() + "</small></p>";
    h += "<table><tr><th>" + String(Lang::statusTableDayHeader()) + "</th>";
    for (int b = 0; b < NUM_BLOCKS; b++) {
        h += "<th>" + String(b * BLOCK_DURATION_HOURS) + "-" + String((b + 1) * BLOCK_DURATION_HOURS) +
             Lang::blockHourSuffix() + "</th>";
    }
    h += "</tr>";
    for (int d = 0; d < NUM_DAYS; d++) {
        h += "<tr><td>" + String(Lang::dayName(d)) + "</td>";
        for (int b = 0; b < NUM_BLOCKS; b++) {
            float avg = baselineAverage(d, b);
            int sevDay = severityForBlock(d, b);
            int sevAll = severityForBlockAllDays(d, b);
            h += "<td>" + String(avg, 1) + "<br><small>" + String(Lang::statusSeverityAbbrev()) + " " +
                 String(sevDay) + " / " + String(sevAll) + "</small></td>";
        }
        h += "</tr>";
    }
    h += "</table>";
    h += "<p><small>" + Lang::statusAveragesHint(NUM_WEEKS) + "</small></p>";

    h += htmlFooter();
    server.send(200, "text/html", h);
}

// --- Settings-tab -------------------------------------------------------

static void handleSettings() {
    String h = htmlHeader(String("esp-motion-absence-detection — ") + Lang::tabSettings(), "settings");

    if (server.hasArg("saved")) {
        h += "<p class='ok'>" + String(Lang::settingsSaved()) + "</p>";
    }

    uint16_t t = config.alarmThreshold;
    h += "<form method='POST' action='/settings/save'>";

    h += "<fieldset><legend>" + String(Lang::settingsSensitivityLegend()) + "</legend>";
    h += "<label><input type='radio' name='sensitivity' value='more'" +
         String(t == THRESHOLD_MORE_SENSITIVE ? " checked" : "") + "> " +
         Lang::settingsSensitivityMore(THRESHOLD_MORE_SENSITIVE) + "</label>";
    h += "<label><input type='radio' name='sensitivity' value='normal'" +
         String(t == THRESHOLD_NORMAL ? " checked" : "") + "> " +
         Lang::settingsSensitivityNormal(THRESHOLD_NORMAL) + "</label>";
    h += "<label><input type='radio' name='sensitivity' value='less'" +
         String(t == THRESHOLD_LESS_SENSITIVE ? " checked" : "") + "> " +
         Lang::settingsSensitivityLess(THRESHOLD_LESS_SENSITIVE) + "</label>";
    if (t != THRESHOLD_MORE_SENSITIVE && t != THRESHOLD_NORMAL && t != THRESHOLD_LESS_SENSITIVE) {
        h += "<p><small class='hint'>" + Lang::settingsNonStandardHint(t) + "</small></p>";
    }
    h += "</fieldset>";

    h += "<fieldset><legend>" + String(Lang::settingsFlatSafetyNetLegend()) + "</legend>";
    h += "<label>" + String(Lang::settingsFlatSafetyNetLabel()) + "<br>";
    h += "<input type='number' name='flatSafetyNetHours' min='1' max='72' value='" +
         String(config.flatSafetyNetHours) + "'></label>";
    h += "<p><small class='hint'>" + String(Lang::settingsFlatSafetyNetHint()) + "</small></p>";
    h += "</fieldset>";

    h += "<fieldset><legend>" + String(Lang::settingsBootstrapLegend()) + "</legend>";
    h += "<label>" + Lang::settingsBootstrapLabel(BLOCK_DURATION_HOURS) + "<br>";
    h += "<input type='number' name='bootstrapFallbackHours' min='4' max='48' step='" +
         String(BLOCK_DURATION_HOURS) + "' value='" + String(config.bootstrapFallbackHours) + "'></label>";
    h += "<p><small class='hint'>" + String(Lang::settingsBootstrapHint()) + "</small></p>";
    h += "</fieldset>";

    h += "<button type='submit'>" + String(Lang::settingsSaveButton()) + "</button>";
    h += "</form>";

    // --- Telegram-testbericht (verplaatst hierheen vanaf Status) ---
    h += "<fieldset><legend>" + String(Lang::settingsTelegramLegend()) + "</legend>";
    h += "<p><a href='/test_telegram'>" + String(Lang::settingsTelegramTestButton()) + "</a></p>";
    if (server.hasArg("telegram_test")) {
        bool ok = (server.arg("telegram_test") == "ok");
        h += "<p class='" + String(ok ? "ok" : "warn") + "'>" +
             String(ok ? Lang::settingsTelegramTestOk() : Lang::settingsTelegramTestFail()) + "</p>";
    }
    h += "</fieldset>";

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
    // Debug-logging blijft Nederlands (developer-facing, los van lang_config.h).
    debugLog("Instellingen gewijzigd via webinterface: drempel=" + String(config.alarmThreshold) +
              " vlakVangnet=" + String(config.flatSafetyNetHours) +
              "u bootstrapFallback=" + String(config.bootstrapFallbackHours) + "u");

    server.sendHeader("Location", "/settings?saved=1");
    server.send(303);
}

// --- Log-tab -------------------------------------------------------

static void handleLog() {
    String h = htmlHeader(String("esp-motion-absence-detection — ") + Lang::tabLog(), "log");
    h += "<meta http-equiv='refresh' content='3'>";

    h += "<h3>" + Lang::logLiveEventsHeading(LIVE_LOG_EVENTS) + "</h3>";
    h += "<pre>" + liveLogGetRecent() + "</pre>";

    h += "<h3>" + String(Lang::logSystemLogHeading()) + "</h3>";
    h += "<p>" + Lang::logVerboseStatus(verboseLogging) + " — <a href='/toggle_verbose'>" +
         String(Lang::logVerboseToggleLink()) + "</a></p>";
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
    bool ok = telegramSendMessage(Lang::telegramTestMessage());
    // Redirect nu naar /settings (was /status) — het testbericht-linkje
    // staat sinds deze sessie op de Settings-tab.
    server.sendHeader("Location", String("/settings?telegram_test=") + (ok ? "ok" : "fail"));
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
