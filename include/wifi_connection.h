// wifi_connection.h — hardcoded WiFi-verbinding via secrets.h
#pragma once

#include <Arduino.h>

extern bool wifiConnected;

void wifiSetup();     // blokkerend: scan + begrensde pogingen, verlaagd
                       // zendvermogen tegen brownouts (zie wifi_connection.cpp)
void wifiLoop();      // in loop(): bewaakt status, herverbindt indien nodig