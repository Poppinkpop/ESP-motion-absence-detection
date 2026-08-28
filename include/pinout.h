// pinout.h — hardware pin- en tijdblok-configuratie
// Boards: Wemos D1 mini (ESP8266EX) / ESP-WROOM-32 DEVKIT V1 (ESP32)
#pragma once
#include <Arduino.h>

#if defined(ESP8266)

// PIR-sensor output pin.
// D2 = GPIO4: vrije, interrupt-geschikte pin, geen bootstrapping-
// functie. (D5/D6 vermeden na een vermoedelijke soldeerbrug tussen
// die twee tijdens het bedraden van een eerder testbordje.)
#define PIR_PIN D2

// Status-LED: licht kort op bij elke geregistreerde tick, 1-op-1 met
// de live telling — puur een "systeem leeft/registreert"-signaal,
// geen alarm- of rustmodus-indicator (zie DETECTION_METHOD.md §11).
// D1 = GPIO5: vrije pin, geen bootstrapping-functie, niet in gebruik
// door PIR_PIN of D5/D6.
// AANNAME: externe LED (met serieweerstand) actief-HOOG aangesloten.
// Zo niet, dan de HIGH/LOW-waarden in motion_tracking.cpp omdraaien.
#define STATUS_LED_PIN D1

// Ingebouwde LED (actief laag) — geen functionele rol, alleen ooit
// handmatig gebruikt tijdens het testen.
#define ONBOARD_LED_PIN LED_BUILTIN

#elif defined(ESP32)

// PIR-sensor output pin.
// GPIO27: vrije, interrupt-geschikte pin, geen bootstrapping-functie,
#define PIR_PIN GPIO_NUM_27

// Status-LED — zelfde functie/aanname (actief-HOOG) als op ESP8266.
// GPIO26: vrije, interrupt-geschikte pin, geen bootstrapping-functie,
// niet in gebruik door PIR_PIN.
#define STATUS_LED_PIN GPIO_NUM_26

// Ingebouwde LED (blauw, tweede onboard-LED naast de rode hardwired
// power-LED) — bevestigd via blink-test (deze sessie): GPIO2.
#define ONBOARD_LED_PIN GPIO_NUM_2

#endif

// --- Tijdblok-indeling (board-onafhankelijk) ---
// 6 blokken van 4 uur, 7 weekdagen (index 0 = maandag ... 6 = zondag),
// 6 weken voortschrijdend gemiddelde (round-robin).
#define NUM_DAYS 7
#define NUM_BLOCKS 6
#define NUM_WEEKS 6
#define BLOCK_DURATION_HOURS 4