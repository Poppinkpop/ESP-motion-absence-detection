// wifi_connection.h — hardcoded WiFi-verbinding via secrets.h
#pragma once

#include <Arduino.h>

extern bool wifiConnected;

// Drempel voor "zwak signaal", gedeeld tussen de eenmalige log-melding
// in wifiLoop() en de opstartmelding in main.cpp — één bron van
// waarheid, geen los tweede getal ergens anders (KIS).
static const int WIFI_WEAK_RSSI_DBM = -80;

void wifiSetup();     // blokkerend: scan + begrensde pogingen
void wifiLoop();      // in loop(): bewaakt status, herverbindt indien nodig

int wifiRssi();       // huidige signaalsterkte in dBm; 0 als niet verbonden
