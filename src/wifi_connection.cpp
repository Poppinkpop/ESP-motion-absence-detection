#include "wifi_connection.h"
#include <ESP8266WiFi.h>
#include "secrets.h"
#include "debug_log.h"

bool wifiConnected = false;

static unsigned long lastAttempt = 0;
static const unsigned long RETRY_INTERVAL_MS = 15000;
static const unsigned long ATTEMPT_TIMEOUT_MS = 15000;
static const int INITIAL_MAX_ATTEMPTS = 5;

// Vlag: is de signaalsterkte al gelogd voor de huidige verbinding?
// Voorkomt herhaling zolang de verbinding intact blijft; wordt
// gereset zodra de verbinding wegvalt, zodat een volgende (her)connect
// weer één keer gelogd wordt (KIS/ROBUUST — geen aparte timer nodig).
static bool wifiWasWeak = false;

static String buildHostname() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String suffix = mac.substring(6);
    return "esp-motion-" + suffix;
}

static const char *wifiStatusText(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:     return "idle (nog niet begonnen)";
        case WL_NO_SSID_AVAIL:   return "SSID niet gevonden (netwerknaam onjuist, buiten bereik, of 5GHz-only)";
        case WL_SCAN_COMPLETED:  return "scan voltooid";
        case WL_CONNECTED:       return "verbonden";
        case WL_CONNECT_FAILED:  return "verbinden mislukt (waarschijnlijk verkeerd wachtwoord)";
        case WL_CONNECTION_LOST: return "verbinding verloren";
        case WL_WRONG_PASSWORD:  return "verkeerd wachtwoord";
        case WL_DISCONNECTED:    return "niet verbonden";
        default:                 return "onbekende status";
    }
}

// Toont welke netwerken zichtbaar zijn en of het doelnetwerk ertussen
// zit — voorkomt gissen als iets niet verbindt (bv. verkeerd
// getypte SSID, of router die op 5GHz zit).
static bool scanAndCheckVisible(const char *ssid) {
    debugLog("WiFi-scan...");
    int found = WiFi.scanNetworks();
    bool visible = false;
    for (int i = 0; i < found; i++) {
        bool isTarget = (WiFi.SSID(i) == ssid);
        if (isTarget) visible = true;
        debugLogVerbose("  '" + WiFi.SSID(i) + "' RSSI=" + String(WiFi.RSSI(i)) +
                          "dBm" + String(isTarget ? "  <-- doelnetwerk" : ""));
    }
    if (!visible) {
        debugLog("WAARSCHUWING: doelnetwerk '" + String(ssid) + "' niet gezien in de scan "
                  "(kan alsnog werken bij een verborgen SSID, maar eerste verdachte)");
    }
    return visible;
}

// Eén poging: verse WiFi.begin(), wacht tot verbonden of timeout.
static bool attemptConnect(const char *ssid, const char *password, unsigned long timeoutMs) {
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
        delay(500);
        debugLogVerbose("  status=" + String(wifiStatusText(WiFi.status())) +
                          " RSSI=" + String(WiFi.RSSI()) + "dBm" +
                          " vrij heap=" + String(ESP.getFreeHeap()));
    }
    return WiFi.status() == WL_CONNECTED;
}

// Blokkerende, uitgebreide eerste verbindingspoging — bedoeld om
// eenmalig vanuit setup() aangeroepen te worden. Scan + verlaagd
// zendvermogen + tot INITIAL_MAX_ATTEMPTS verse pogingen.
static bool wifiConnectInitial() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(buildHostname());

    scanAndCheckVisible(WiFi_SSID);

    for (int attempt = 1; attempt <= INITIAL_MAX_ATTEMPTS; attempt++) {
        debugLog("WiFi verbindpoging " + String(attempt) + "/" + String(INITIAL_MAX_ATTEMPTS) + "...");
        WiFi.disconnect(true);
        delay(100);
        if (attemptConnect(WiFi_SSID, WiFi_password, ATTEMPT_TIMEOUT_MS)) {
            debugLog("WiFi verbonden, IP: " + WiFi.localIP().toString() +
                      ", signaal: " + String(WiFi.RSSI()) + " dBm");
            return true;
        }
        debugLog("Poging " + String(attempt) + " mislukt: " + wifiStatusText(WiFi.status()));
        WiFi.disconnect(true);
        delay(2000);
    }
    debugLog("WiFi niet verbonden na " + String(INITIAL_MAX_ATTEMPTS) + " pogingen — ga door zonder, blijft op de achtergrond proberen");
    return false;
}

void wifiSetup() {
    wifiConnected = wifiConnectInitial();
    lastAttempt = millis();
}

int wifiRssi() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
}

// Lichte, niet-blokkerende bewaking + herverbinden voor als de
// verbinding later (na een succesvolle start) wegvalt. Gebruikt ook
// een verse WiFi.begin() i.p.v. WiFi.reconnect() — zelfde reden als
// bij de initiële verbinding.
void wifiLoop() {
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    if (nowConnected && !wifiConnected) {
        wifiConnected = true;
        debugLog("WiFi (opnieuw) verbonden, IP: " + WiFi.localIP().toString());
    } else if (!nowConnected && wifiConnected) {
        wifiConnected = false;
        debugLog("WiFi verbinding verloren (" + String(wifiStatusText(WiFi.status())) + ")");
    }

    // Eenmalige log-melding van de signaalsterkte bij het (opnieuw)
    // verbinden — in alle gevallen, niet alleen bij een zwak signaal.
    // wifiWasWeak fungeert hier als "al gelogd voor deze verbinding"-vlag.
    if (nowConnected && !wifiWasWeak) {
        wifiWasWeak = true;
        int rssi = WiFi.RSSI();
        debugLog("WiFi-signaalsterkte: " + String(rssi) + " dBm" +
                  (rssi <= WIFI_WEAK_RSSI_DBM ? " (zwak, drempel " + String(WIFI_WEAK_RSSI_DBM) + " dBm)" : ""));
    } else if (!nowConnected) {
        wifiWasWeak = false;
    }

    if (!nowConnected && (millis() - lastAttempt > RETRY_INTERVAL_MS)) {
        debugLog("WiFi reconnect poging (verse WiFi.begin())...");
        WiFi.disconnect(true);
        WiFi.begin(WiFi_SSID, WiFi_password);
        lastAttempt = millis();
    }
}