#include "debug_log.h"

bool verboseLogging = false;

static String logLines[DEBUG_LOG_LINES];
static uint8_t logHead = 0;   // volgende te overschrijven index
static uint8_t logCount = 0;  // aantal gevulde regels (max DEBUG_LOG_LINES)

static void pushLine(const String &line) {
    String stamped = "[" + String(millis() / 1000) + "s] " + line;
    logLines[logHead] = stamped;
    logHead = (logHead + 1) % DEBUG_LOG_LINES;
    if (logCount < DEBUG_LOG_LINES) logCount++;
    Serial.println(stamped);
}

void debugLogSetup() {
    Serial.begin(115200);
    delay(50);
    Serial.println();
    debugLog("debug_log gestart");
}

void debugLog(const String &line) {
    pushLine(line);
}

void debugLogVerbose(const String &line) {
    if (verboseLogging) pushLine("[verbose] " + line);
}

void debugLogSetVerbose(bool enabled) {
    verboseLogging = enabled;
    debugLog(enabled ? "Verbose logging AAN" : "Verbose logging UIT");
}

String debugLogGetRecent() {
    String out;
    // oudste eerst, nieuwste onderaan
    uint8_t start = (logCount < DEBUG_LOG_LINES) ? 0 : logHead;
    for (uint8_t i = 0; i < logCount; i++) {
        uint8_t idx = (start + i) % DEBUG_LOG_LINES;
        out += logLines[idx];
        out += "\n";
    }
    return out;
}
