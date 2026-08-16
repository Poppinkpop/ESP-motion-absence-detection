// main.cpp — esp-motion-absence-detection
// Orkestreert de modules. Bevat zelf geen alarmlogica — dat leeft in
// motion_tracking. Wel eigenaar van de exacte berichtteksten (zie
// CLAUDE.md/DETECTION_METHOD.md: motion_tracking bepaalt WANNEER,
// main.cpp bepaalt WAT er in het bericht staat — de daadwerkelijke
// vertaalde tekst komt sinds deze sessie uit lang.h).
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "debug_log.h"
#include "config.h"
#include "wifi_connection.h"
#include "time_sync.h"
#include "motion_tracking.h"
#include "webinterface.h"
#include "telegram_alert.h"
#include "lang.h"

static String buildAlarmMessage(const String &reason) {
    String msg = reason;

    // Melding 2/3 en 3/3 krijgen een aankondiging over de naderende/
    // ingaande rustmodus. motion.notificationCount is op het moment van
    // versturen al opgehoogd naar het huidige meldingnummer (zie
    // motion_tracking.cpp: fireFirstNotificationIfNeeded() /
    // tickCooldownAndMaybeEscalate() zetten hem vóór hasPendingNotification).
    if (motion.notificationCount == 2) {
        msg += "\n" + Lang::telegramRestModeUpcomingNote();
    } else if (motion.notificationCount == 3) {
        msg += "\n" + Lang::telegramRestModeEnteringNote();
    }

    if (motion.onrustActive) {
        msg += "\n" + Lang::telegramOnrustNote();
    }
    return msg;
}

static String reasonTextFor(NotificationKind kind) {
    switch (kind) {
        case NotificationKind::BLOCK_ALARM:
            return Lang::telegramReasonBlockAlarm();
        case NotificationKind::FLAT_SAFETY_NET:
            return Lang::telegramReasonFlatSafetyNet(config.flatSafetyNetHours);
        case NotificationKind::BOOTSTRAP_FALLBACK:
            return Lang::telegramReasonBootstrapFallback(config.bootstrapFallbackHours);
        default:
            return Lang::telegramReasonDefault();
    }
}

static String weeklyReassuranceMessage() {
    struct tm t;
    localtime_r(&motion.lastMovementEpoch, &t);
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
    return Lang::telegramWeeklyReassurance(String(buf));
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
    // meldingscap (DETECTION_METHOD.md §12b) — puur "ik ben verbonden".
    // Bevat, indien van toepassing, ook eenmalig een zwak-signaal-notitie
    // (WIFI_WEAK_RSSI_DBM, gedeeld met de log-melding in wifi_connection.cpp) —
    // zodat een zwakke plek meteen zichtbaar is zonder dat er los in de
    // Log-tab gekeken hoeft te worden.
    String ip = WiFi.localIP().toString();
    String startupMsg = Lang::telegramStartupMessage(ip);
    if (wifiConnected && wifiRssi() <= WIFI_WEAK_RSSI_DBM) {
        startupMsg += "\n" + Lang::telegramWeakSignalNote(wifiRssi());
    }
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
