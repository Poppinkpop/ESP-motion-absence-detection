#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "debug_log.h"
AppConfig config;
static const char *CONFIG_PATH = "/config.json";
bool configSetup() {
    if (!LittleFS.begin()) {
        debugLog("LittleFS mount mislukt, formatteren...");
        if (!LittleFS.format() || !LittleFS.begin()) {
            debugLog("LittleFS formatteren/mounten mislukt");
            return false;
        }
    }
    if (!LittleFS.exists(CONFIG_PATH)) {
        debugLog("Geen config.json gevonden, default wordt aangemaakt");
        return configSave();
    }
    File f = LittleFS.open(CONFIG_PATH, "r");
    if (!f) {
        debugLog("config.json openen mislukt");
        return false;
    }
    // Ruime capaciteit: 42 cellen x (6 tellingen + 6 filled + severity)
    // plus instellingen. JsonDocument (ArduinoJson v7) groeit dynamisch.
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) {
        debugLog("config.json parse-fout: " + String(err.c_str()) + " — default wordt gebruikt");
        return configSave();
    }
    config.alarmThreshold = doc["alarmThreshold"] | 6;
    config.flatSafetyNetHours = doc["flatSafetyNetHours"] | 12;
    config.bootstrapFallbackHours = doc["bootstrapFallbackHours"] | 16;
    config.onrustMinDeviatingDays = doc["onrustMinDeviatingDays"] | 3;
    strlcpy(config.timezone, doc["timezone"] | "CET-1CEST,M3.5.0,M10.5.0/3", sizeof(config.timezone));
    JsonArray days = doc["cells"];
    if (!days.isNull()) {
        for (int d = 0; d < NUM_DAYS && d < (int)days.size(); d++) {
            JsonArray blocks = days[d];
            for (int b = 0; b < NUM_BLOCKS && b < (int)blocks.size(); b++) {
                JsonObject cell = blocks[b];
                config.cells[d][b].severityOverride = cell["sev"] | -1;
                JsonArray counts = cell["counts"];
                JsonArray filled = cell["filled"];
                for (int w = 0; w < NUM_WEEKS; w++) {
                    config.cells[d][b].weekCounts[w] = (w < (int)counts.size()) ? (uint16_t)counts[w] : 0;
                    config.cells[d][b].filled[w] = (w < (int)filled.size()) ? (bool)filled[w] : false;
                }
            }
        }
    }
    debugLog("config.json geladen");
    return true;
}
bool configSave() {
    JsonDocument doc;
    doc["alarmThreshold"] = config.alarmThreshold;
    doc["flatSafetyNetHours"] = config.flatSafetyNetHours;
    doc["bootstrapFallbackHours"] = config.bootstrapFallbackHours;
    doc["onrustMinDeviatingDays"] = config.onrustMinDeviatingDays;
    doc["timezone"] = config.timezone;
    JsonArray days = doc["cells"].to<JsonArray>();
    for (int d = 0; d < NUM_DAYS; d++) {
        JsonArray blocks = days.add<JsonArray>();
        for (int b = 0; b < NUM_BLOCKS; b++) {
            JsonObject cell = blocks.add<JsonObject>();
            cell["sev"] = config.cells[d][b].severityOverride;
            JsonArray counts = cell["counts"].to<JsonArray>();
            JsonArray filled = cell["filled"].to<JsonArray>();
            for (int w = 0; w < NUM_WEEKS; w++) {
                counts.add(config.cells[d][b].weekCounts[w]);
                filled.add(config.cells[d][b].filled[w]);
            }
        }
    }
    File f = LittleFS.open(CONFIG_PATH, "w");
    if (!f) {
        debugLog("config.json schrijven mislukt (open)");
        return false;
    }
    size_t written = serializeJson(doc, f);
    f.close();
    if (written == 0) {
        debugLog("config.json schrijven mislukt (serialize)");
        return false;
    }
    debugLogVerbose("config.json opgeslagen (" + String(written) + " bytes)");
    return true;
}