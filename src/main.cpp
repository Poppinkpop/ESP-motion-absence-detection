// main.cpp — esp-motion-absence-detection
// Orkestreert de modules. Bevat zelf geen alarmlogica — dat leeft in
// motion_tracking. Wel eigenaar van de exacte berichtteksten (zie
// CLAUDE.md/ALGORITHM.md: motion_tracking bepaalt WANNEER, main.cpp
// bepaalt WAT er in het bericht staat).
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "debug_log.h"
#include "config.h"
#include "wifi_connection.h"
#include "time_sync.h"
#include "motion_tracking.h"
#include "webinterface.h"
#include "telegram_alert.h"

static String buildAlarmMessage(const String &reason) {
    String msg = reason;
    if (motion.onrustActive) {
        msg += "\nDaarnaast vertoont het bewegingspatroon van de afgelopen dagen meer onrust dan gebruikelijk (minder regelmatig dan voorheen).";
    }
    return msg;
}

static String reasonTextFor(NotificationKind kind) {
    switch (kind) {
        case NotificationKind::BLOCK_ALARM:
            return "Bewegingsalarm: het bewegingspatroon wijkt significant af van het normale patroon.";
        case NotificationKind::FLAT_SAFETY_NET:
            return "Waarschuwing: al " + String(config.flatSafetyNetHours) + " uur geen beweging waargenomen.";
        case NotificationKind::BOOTSTRAP_FALLBACK:
            return "Waarschuwing: geen beweging waargenomen gedurende " +
                   String(config.bootstrapFallbackHours) +
                   " uur — dit tijdsblok-gebaseerde vangnet geldt tijdelijk, zolang deze weekdag nog geen volledig geleerd patroon heeft.";
        default:
            return "Waarschuwing: afwijkend bewegingspatroon gedetecteerd.";
    }
}

static String weeklyReassuranceMessage() {
    struct tm t;
    localtime_r(&motion.lastMovementEpoch, &t);
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
    return "Het systeem staat in rustmodus omdat er sinds " + String(buf) +
           " geen beweging is waargenomen. Er wordt niet meer gealarmeerd en er wordt niet meer geleerd totdat er weer beweging is — het systeem werkt verder nog gewoon.";
}

void setup() {
    debugLogSetup();
    debugLog("=== esp-motion-absence-detection opstarten ===");
    configSetup();
    wifiSetup();
    timeSyncSetup();
    motionTrackingSetup();
    webinterfaceSetup();
    telegramAlertSetup();

    // Opstartmelding: precies 1x per boot, telt niet mee voor de
    // meldingscap (ALGORITHM.md §12b) — puur "ik ben verbonden".
    String ip = WiFi.localIP().toString();
    String startupMsg = "De sensor is aangesloten en actief. U kunt de "
        "instellingen wijzigen als u in de ruimte bent waar de sensor is "
        "geplaatst en verbonden bent met het huisnetwerk, via http://" + ip;
    telegramSendMessage(startupMsg);

    debugLog("Setup voltooid");
}

void loop() {
    wifiLoop();
    timeSyncLoop();
    motionTrackingLoop();
    webinterfaceLoop();

    if (motion.hasPendingNotification) {
        telegramSendMessage(buildAlarmMessage(reasonTextFor(motion.pendingNotificationKind)));
        motion.hasPendingNotification = false;
        motion.pendingNotificationKind = NotificationKind::NONE;
    }

    if (motion.hasPendingWeeklyReassurance) {
        telegramSendMessage(weeklyReassuranceMessage());
        motion.hasPendingWeeklyReassurance = false;
    }
}