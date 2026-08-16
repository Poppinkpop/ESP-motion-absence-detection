// lang.h — vertaalde teksten voor de webinterface en de Telegram-
// berichten. De taal wordt gekozen in lang_config.h (compile-time,
// één LANG_-macro). Elke functie hier geeft de tekst in de gekozen
// taal terug; webinterface.cpp en main.cpp bevatten zelf geen
// taal-specifieke tekst meer.
//
// Debug-logging (debugLog/debugLogVerbose) valt hier NIET onder en
// blijft altijd Nederlands — zie lang_config.h.
#pragma once

#include <Arduino.h>

namespace Lang {

// --- Algemeen ---
const char* htmlLangCode();     // "en", "nl", "de", "fr", "es" — <html lang='..'>
const char* tabStatus();
const char* tabSettings();
const char* tabLog();
const char* dayName(int day);   // 0=maandag..6=zondag
const char* blockHourSuffix();  // "u" (NL) of "h" (overig), bv. "8-12u"/"8-12h"

const char* sensitivityMoreWord();
const char* sensitivityNormalWord();
const char* sensitivityLessWord();
String sensitivityCustomLabel(uint16_t threshold);

const char* active();
const char* inactive();

// --- Status-tab ---
String statusTimeLine(bool timeAvailableFlag, const String &timeStr);
String statusWifiLine(bool connected, int rssiDbm);
String statusCurrentBlockLine(const String &dayName, int block, int startHour, int endHour);
String statusLiveCountLine(uint16_t count);
String statusLastMovementLine(unsigned long minutesAgo);
String statusRestModeBanner();
String statusNotificationLine(uint8_t count, uint16_t cooldownBlocksRemaining, bool showCooldown);
String statusAlarmBuildupLine(uint16_t sum, uint8_t streakLen, uint16_t threshold, const String &sensitivityLabel);
const char* labelBlockAlarm();
String labelFlatSafetyNet(uint16_t hours);
String labelBootstrapFallback(uint16_t hours);
const char* labelOnrust();
String statusLearnedPatternHeading(uint8_t weeks);
String statusSeverityHint();
const char* statusTableDayHeader();
const char* statusSeverityAbbrev();
String statusAveragesHint(uint8_t weeks);

// --- Settings-tab ---
const char* settingsSaved();
const char* settingsSensitivityLegend();
String settingsSensitivityMore(uint16_t threshold);
String settingsSensitivityNormal(uint16_t threshold);
String settingsSensitivityLess(uint16_t threshold);
String settingsNonStandardHint(uint16_t threshold);
const char* settingsFlatSafetyNetLegend();
const char* settingsFlatSafetyNetLabel();
const char* settingsFlatSafetyNetHint();
const char* settingsBootstrapLegend();
String settingsBootstrapLabel(uint8_t blockDurationHours);
const char* settingsBootstrapHint();
const char* settingsSaveButton();
const char* settingsTelegramLegend();
const char* settingsTelegramTestButton();
const char* settingsTelegramTestOk();
const char* settingsTelegramTestFail();

// --- Log-tab ---
String logLiveEventsHeading(uint8_t maxEvents);
const char* logSystemLogHeading();
String logVerboseStatus(bool on);
const char* logVerboseToggleLink();

// --- Telegram ---
String telegramTestMessage();
String telegramStartupMessage(const String &ip);
String telegramWeakSignalNote(int rssiDbm);
String telegramReasonBlockAlarm();
String telegramReasonFlatSafetyNet(uint16_t hours);
String telegramReasonBootstrapFallback(uint16_t hours);
String telegramReasonDefault();
String telegramRestModeUpcomingNote();
String telegramRestModeEnteringNote();
String telegramOnrustNote();
String telegramWeeklyReassurance(const String &lastMovementTimestamp);

} // namespace Lang
