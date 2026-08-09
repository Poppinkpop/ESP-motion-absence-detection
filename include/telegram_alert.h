// telegram_alert.h — verstuurt alarmberichten naar de Telegram Bot API
#pragma once

#include <Arduino.h>

void telegramAlertSetup();

// Verstuurt één bericht. Retourneert false bij geen WiFi of een
// HTTP-fout (zie debug-log voor details). Geen retry/escalatie hier —
// dat is bewust: 1 bericht, klaar (zie CLAUDE.md kernontwerp punt 8).
bool telegramSendMessage(const String &text);
