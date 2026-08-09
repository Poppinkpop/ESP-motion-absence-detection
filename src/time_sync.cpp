#include "time_sync.h"
#include <time.h>
#include "pinout.h"
#include "config.h"
#include "debug_log.h"

bool timeAvailable = false;

static bool loggedSyncOnce = false;

void timeSyncSetup() {
    // configTzTime() combineert de POSIX TZ-string en NTP-servers in
    // één aanroep — betrouwbaarder op deze ESP8266-core dan de losse
    // combinatie van setenv("TZ",...)/tzset() met de oude
    // configTime(gmtOffset, daylightOffset, ...), die de tijd in de
    // praktijk als UTC bleef behandelen (2 uur achter in de zomer).
    configTzTime(config.timezone, "pool.ntp.org", "nl.pool.ntp.org");
    debugLog("NTP-sync gestart (tijdzone: " + String(config.timezone) + ")");
}

void timeSyncLoop() {
    time_t now = time(nullptr);
    bool nowAvailable = (now > 8 * 3600 * 2); // sanity check: na 1-1-1970 + ruim marge

    if (nowAvailable && !timeAvailable) {
        timeAvailable = true;
        if (!loggedSyncOnce) {
            debugLog("NTP-tijd beschikbaar: " + currentTimeString());
            loggedSyncOnce = true;
        }
    } else if (!nowAvailable && timeAvailable) {
        // Zou niet moeten gebeuren zolang de ESP niet reboot, maar
        // defensief: als tijd ooit weer onbeschikbaar lijkt, opnieuw
        // als "niet beschikbaar" markeren i.p.v. met stale data verder
        // rekenen.
        timeAvailable = false;
        debugLog("NTP-tijd lijkt niet meer beschikbaar");
    }
}

int currentWeekday() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    // tm_wday: 0=zondag..6=zaterdag -> omzetten naar 0=maandag..6=zondag
    return (t.tm_wday + 6) % 7;
}

int currentBlock() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    int block = t.tm_hour / BLOCK_DURATION_HOURS;
    if (block >= NUM_BLOCKS) block = NUM_BLOCKS - 1; // veiligheidsklem
    return block;
}

uint32_t currentEpochWeek() {
    time_t now = time(nullptr);
    return (uint32_t)(now / (7UL * 24UL * 3600UL));
}

String currentTimeString() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    return String(buf);
}