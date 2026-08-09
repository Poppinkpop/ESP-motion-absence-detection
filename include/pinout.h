// pinout.h — hardware pin- en tijdblok-configuratie
// Board: Wemos D1 mini (ESP8266EX)
#pragma once
#include <Arduino.h>

// PIR-sensor output pin.
// D2 = GPIO4: vrije, interrupt-geschikte pin, geen bootstrapping-
// functie. (D5/D6 vermeden na een vermoedelijke soldeerbrug tussen
// die twee tijdens het bedraden van een eerder testbordje.)
#define PIR_PIN D2

// Status-LED (nieuw): licht kort op bij elke geregistreerde tick,
// 1-op-1 met de live telling — puur een "systeem leeft/registreert"-
// signaal, geen alarm- of rustmodus-indicator (zie ALGORITHM.md §11).
// D1 = GPIO5: vrije pin, geen bootstrapping-functie, niet in gebruik
// door PIR_PIN of D5/D6. De uitgang zit altijd in de firmware; of de
// LED fysiek wordt ingebouwd is aan de bouwer van het kastje.
// AANNAME: externe LED (met serieweerstand) actief-HOOG aangesloten.
// Zo niet (bv. actief-laag zoals ONBOARD_LED_PIN hieronder), dan de
// HIGH/LOW-waarden in motion_tracking.cpp omdraaien.
#define STATUS_LED_PIN D1

// Ingebouwde LED (actief laag) — apart van STATUS_LED_PIN, geen
// functionele rol, alleen ooit handmatig gebruikt tijdens het testen.
#define ONBOARD_LED_PIN LED_BUILTIN

// --- Tijdblok-indeling ---
// 6 blokken van 4 uur, 7 weekdagen (index 0 = maandag ... 6 = zondag),
// 6 weken voortschrijdend gemiddelde (round-robin).
#define NUM_DAYS 7
#define NUM_BLOCKS 6
#define NUM_WEEKS 6
#define BLOCK_DURATION_HOURS 4