// debug_log.h — twee-niveau logging (normaal / verbose)
#pragma once

#include <Arduino.h>

// Ringbuffer met de laatste N regels, zodat de webinterface het
// opstartlog/verloop kan tonen zonder een bestand bij te houden.
#define DEBUG_LOG_LINES 40

extern bool verboseLogging;

void debugLogSetup();
void debugLog(const String &line);          // normaal, altijd actief
void debugLogVerbose(const String &line);    // alleen als verboseLogging aan staat
void debugLogSetVerbose(bool enabled);

// Geeft de laatste regels als één string terug (nieuwste onderaan),
// voor weergave in de Logging-tab van de webinterface.
String debugLogGetRecent();
