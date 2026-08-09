// time_sync.h — NTP-synchronisatie en weekdag/tijdsblok-berekening
#pragma once

#include <Arduino.h>

extern bool timeAvailable;

void timeSyncSetup();   // configTime() + tijdzone instellen
void timeSyncLoop();    // periodieke check of NTP inmiddels gelukt is

// Huidige weekdag: 0 = maandag ... 6 = zondag (Dutch/ISO-achtige
// telling, NIET de POSIX tm_wday die met zondag=0 begint).
int currentWeekday();

// Huidig tijdsblok: 0-5, elk BLOCK_DURATION_HOURS uur lang.
int currentBlock();

// Aantal volledige weken sinds Unix-epoch (voor de round-robin
// baseline-slot-berekening) — gebaseerd op wall-clock tijd, dus
// consistent na een reboot zolang NTP beschikbaar is.
uint32_t currentEpochWeek();

// Leesbare tijd voor logging, bv. "14:32:07".
String currentTimeString();
