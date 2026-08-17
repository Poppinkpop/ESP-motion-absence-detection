// config.h — instellingen en geleerde baseline, opgeslagen als
// config.json op LittleFS. Overleeft een reboot.
#pragma once
#include <Arduino.h>
#include "pinout.h"
// Eén cel = één (weekdag, blok)-combinatie.
// weekCounts[i] = ruwe PIR-telling van dat blok in een van de laatste
// NUM_WEEKS weken; round-robin, slot = (epochWeek % NUM_WEEKS).
// filled[i] geeft aan of dat slot al een echte meting bevat (i.p.v.
// een ongebruikte 0 bij een nog niet volledig gevulde geschiedenis).
struct CellBaseline {
    uint16_t weekCounts[NUM_WEEKS] = {0};
    bool filled[NUM_WEEKS] = {false};
    // -1 = automatisch bepalen (kwartiel-gebaseerd), 0-3 = door
    // gebruiker overschreven severity voor dit blok.
    int8_t severityOverride = -1;
};
struct AppConfig {
    // Alarmdrempel: som van severity van de laatste 2 (of 3)
    // opeenvolgende afwijkende blokken moet dit overschrijden (>=).
    // Wordt in de webinterface ingesteld via 3 sensitivity-presets
    // (less sensitive=7, normal=6, more sensitive=5) — zie
    // DETECTION_METHOD.md §5. Dit veld blijft de enige bron van waarheid;
    // de presets schrijven er gewoon in.
    uint16_t alarmThreshold = 6;
    // Vlak vangnet: aantal uur zonder beweging dat sowieso een alarm
    // veroorzaakt, los van de geleerde logica. Permanente achtervang
    // (DETECTION_METHOD.md §8).
    uint16_t flatSafetyNetHours = 12;
    // Bootstrap-fallback: aantal uur (in blokken van
    // BLOCK_DURATION_HOURS, dus block-aligned, geen doorlopend
    // venster) zonder beweging dat een alarm veroorzaakt zolang de
    // huidige weekdag nog in de leerperiode zit (< 3 gevulde weken
    // voor die weekdag). Zie DETECTION_METHOD.md §7.
    uint16_t bootstrapFallbackHours = 16;
    // Onrust: minimum aantal afwijkende dagen in een week voordat de
    // onrust-vlag wordt gezet.
    uint8_t onrustMinDeviatingDays = 3;
    // Tijdzone (POSIX TZ-string), default Europe/Amsterdam.
    char timezone[64] = "CET-1CEST,M3.5.0,M10.5.0/3";
    CellBaseline cells[NUM_DAYS][NUM_BLOCKS];
};
extern AppConfig config;
bool configSetup();          // mount LittleFS, laad config.json (of maak default)
bool configSave();           // schrijf huidige config naar config.json