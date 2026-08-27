#include "telegram_alert.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_heap_caps.h>
#endif
#include "secrets.h"
#include "debug_log.h"

static String urlEncode(const String &s) {
    String out;
    char buf[4];
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else if (c == ' ') {
            out += '+';
        } else {
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            out += buf;
        }
    }
    return out;
}

void telegramAlertSetup() {
    debugLog("telegram_alert gereed");
}

bool telegramSendMessage(const String &text) {
    if (WiFi.status() != WL_CONNECTED) {
        debugLog("Telegram-bericht overgeslagen: geen WiFi");
        return false;
    }
    debugLog("Vrije heap voor Telegram-poging: " + String(ESP.getFreeHeap()));
#if defined(ESP8266)
    debugLog("Grootste aaneengesloten blok: " + String(ESP.getMaxFreeBlockSize()));
#elif defined(ESP32)
    debugLog("Grootste aaneengesloten blok: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)));
#endif
    // setInsecure(): geen certificaatvalidatie. Zelfde afweging als bij
    // esp-price-led (ESP8266 kan lastig een actuele CA-store bijhouden),
    // maar let op: dit is hier minder onschuldig dan daar — dit is een
    // uitgaand bericht met je bot-token, niet alleen het lezen van
    // publieke prijzen. Risico is beperkt (een aanvaller op hetzelfde
    // netwerk zou in theorie het token kunnen onderscheppen), maar niet
    // nul. Kan later verscherpt worden met certificate pinning als
    // gewenst — voor nu bewust KIS, zoals besproken.
    WiFiClientSecure client;
    client.setInsecure();
#if defined(ESP8266)
    // setBufferSizes(512, 512): verkleint de BearSSL RX/TX-buffers van
    // standaard ~16KB elk naar 512 bytes elk. Bevestigde fix voor een
    // heapfragmentatie-probleem waarbij het grootste aaneengesloten vrije
    // blok na een tijdje draaien te klein werd voor de standaardbuffers
    // (HTTP-code -1, lege response). Zie CLAUDE.md kernontwerpbeslissing 21.
    // ESP32-specifiek: WiFiClientSecure gebruikt daar mbedTLS i.p.v.
    // BearSSL en heeft deze methode niet — ook niet nodig, gezien de
    // veel ruimere RAM op ESP32.
    client.setBufferSizes(512, 512);
#endif
    HTTPClient https;
    String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
    if (!https.begin(client, url)) {
        debugLog("Telegram: HTTPS begin() mislukt");
        return false;
    }
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Persoonsgegevens-veld staat vooraan in elk bericht, zodat bij
    // meerdere sensoren/cliënten altijd duidelijk is om wie het gaat
    // (zie DETECTION_METHOD.md §12a, secrets.h voor het veld zelf).
    String fullText = String(PERSOONSGEGEVENS) + "\n" + text;
    String body = "chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + urlEncode(fullText);
    int httpCode = https.POST(body);
    bool ok = (httpCode == 200);
    if (ok) {
        debugLog("Telegram-bericht verstuurd");
    } else {
        debugLog("Telegram-bericht mislukt, HTTP-code " + String(httpCode));
        debugLogVerbose("Telegram response: " + https.getString());
    }
    https.end();
    return ok;
}