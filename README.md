# esp-motion-absence-detection

Privacy-first bewegingsbewaking voor mensen die alleen wonen. Eén
PIR-sensor in de woonkamer op een ESP8266 (Wemos D1 mini) leert het
normale bewegingspatroon per weekdag en tijdsblok, en waarschuwt de
familie via Telegram wanneer dat patroon significant afwijkt — zonder
camera, geluid, identificatie, of enige externe dataverbinding buiten
Telegram zelf.

Zie [`PHILOSOPHY.md`](PHILOSOPHY.md) voor de achtergrond en
vergelijkbare projecten, en [`ALGORITHM.md`](ALGORITHM.md) voor de
exacte rekenmethode.

## Wat het doet

- Eén PIR-bewegingssensor registreert uitsluitend **of** er beweging
  is — geen beeld, geen geluid, geen locatie binnen de woning, en geen
  enkele externe dataverbinding behalve de Telegram-melding zelf.
- Het systeem leert per weekdag en tijdsblok (6 blokken van 4 uur) wat
  normaal is, op basis van een voortschrijdend gemiddelde over de
  laatste 6 weken.
- Wijkt het huidige bewegingspatroon significant af — te weinig
  beweging in een normaal actief tijdsblok — dan gaat er een
  Telegram-bericht naar de familie.
- Een vlak vangnet vangt ook de eerste weken op, voordat er genoeg
  geschiedenis is opgebouwd, en blijft daarna als permanente
  achtervang actief.
- Maximaal 3 meldingen per aaneengesloten episode (met een cooldown
  van 3 tijdsblokken ertussen). Blijft de familie daarna niet
  reageren, dan stopt het systeem met waarschuwen én met leren
  (rustmodus), en laat het zich wekelijks kort horen om te bevestigen
  dat het nog werkt.
- Een apart "onrust"-signaal herkent wanneer het hele dagpatroon
  onregelmatiger wordt dan gebruikelijk — in beide richtingen (te
  weinig, maar ook ongebruikelijk veel beweging in normaal rustige
  uren, zoals 's nachts ronddwalen).
- Een optionele, gedimde groene LED geeft een korte visuele
  bevestiging bij elke geregistreerde beweging.

Alle drempels en tijdsinstellingen hierboven zijn instelbaar via de
**Settings**-tab van de webinterface — zie [`ALGORITHM.md`](ALGORITHM.md)
voor de exacte rekenmethode en defaultwaarden.

## Privacy

Het systeem is bewust zo ontworpen dat het **nooit** een
dag-voor-dag-logboek van aanwezigheid toont of verstuurt, en dat er
geen enkele data het huis verlaat behalve de Telegram-melding zelf
(geen cloud-dashboard, geen externe integraties). Alleen het
geaggregeerde, geleerde patroon is zichtbaar in de webinterface, nooit
herleidbaar naar een specifieke datum.

## Webinterface

De webinterface bestaat uit drie tabs:

- **Status** — huidig tijdsblok, live tick-telling, het geleerde
  patroon (zowel per weekdag als over alle dagen samen), actuele
  alarm-/onrust-status, het aantal meldingen in de huidige episode
  (0-3), en of het systeem in rustmodus staat.
- **Settings** — gevoeligheid (Less sensitive / Normal / More
  sensitive, standaard Normal — zie [`ALGORITHM.md`](ALGORITHM.md) voor
  de exacte drempelwaarden per keuze), vlak-vangnet-uren, en de
  bootstrap-fallback-uren voor de eerste weken.
- **Log** — live PIR-events en het debug-log.

## Installatie

1. Kopieer `include/secrets.h.example` naar `include/secrets.h` en vul
   in:
   - je WiFi-gegevens
   - een Telegram bot-token en chat-id (zie hieronder)
   - een **persoonsgegevens-veld**: een korte naam of aanduiding (bv.
     "Vader", "mevr. van Dam", of een cliëntnummer bij een organisatie)
     die wordt meegestuurd in elk Telegram-bericht, zodat bij meerdere
     sensoren altijd duidelijk is om wie het gaat. **Let op:** vul hier
     geen direct herleidbare gegevens in zoals een volledig adres — een
     naam, relatie of cliëntnummer is voldoende en veiliger.
2. Sluit een PIR-sensor aan (standaardpin **D2**, in `pinout.h` te
   wijzigen), gevoed door **5V** (niet 3.3V — de meeste PIR-modules
   hebben dat nodig om te functioneren).
3. Sluit optioneel een gedimde groene LED aan als visuele bevestiging
   dat het systeem beweging registreert (standaardpin **D1**, eveneens
   in `pinout.h` te wijzigen, los van de PIR-pin) — zie
   [Optionele LED](#optionele-led) hieronder. De LED-uitgang zit altijd
   in de firmware; of de LED fysiek wordt ingebouwd is aan de bouwer
   van het kastje.
4. Open het project in VS Code met de PlatformIO-extensie. Kies de
   juiste `default_envs` in `platformio.ini` (`d1_mini` of
   `esp8266dev`, afhankelijk van je exacte bordje) en flash.
5. Bij elke opstart stuurt het systeem één keer een Telegram-bericht
   dat de sensor voor het ingevulde persoonsgegevens-veld verbonden is,
   met het IP-adres waarop de instellingen te wijzigen zijn (alleen te
   bereiken vanaf het eigen huisnetwerk). Deze melding telt niet mee
   voor de meldingslimiet hieronder.
6. Open het IP-adres van het board in een browser voor de Status-,
   Settings- en Log-tab.
7. Kies eventueel op de **Settings**-tab een andere gevoeligheid dan de
   standaard "Normal", of pas de vangnet-uren aan.

### Een Telegram-bot aanmaken

1. Zoek in Telegram naar **BotFather**, stuur `/newbot`, volg de
   stappen. Je krijgt een bot-token terug.
2. Stuur een bericht naar je bot (privé, of voeg 'm toe aan een groep
   en stuur daar een bericht).
3. Open `https://api.telegram.org/bot<JOUW_TOKEN>/getUpdates` in een
   browser en zoek het `chat.id`-veld — dat is je chat-id.
4. Zet beide waarden in `secrets.h`.

## Hardware

- Wemos D1 mini (ESP8266EX, 4MB flash)
- PIR-sensor (bv. HC-SR501) op standaardpin D2 (GPIO4), gevoed door 5V —
  pin instelbaar in `pinout.h` (compile-time)
- Optioneel: gedimde groene LED op standaardpin D1 (GPIO5), eveneens
  instelbaar in `pinout.h`

### Optionele LED

Een gedimde groene LED kan worden aangesloten als eenvoudige visuele
bevestiging dat het systeem actief beweging registreert: de LED
weerspiegelt 1-op-1 elke geregistreerde tick (niet als alarmindicator,
en niet zichtbaar/relevant voor de bewoner tijdens afwezigheid — puur
een "systeem leeft"-signaal voor wie er wél bij staat, bijvoorbeeld
tijdens installatie of onderhoud). De LED-uitgang zit standaard in de
firmware; volledig optioneel om ook echt fysiek in te bouwen.

## Een kanttekening: WiFi- en stroomuitval

Als het board zelf offline gaat (WiFi weg, stroomuitval, crash) merkt
dit project dat niet actief op naar de familie toe — er is dan geen
aparte "het systeem is uitgevallen"-melding. Hetzelfde uitgangspunt
geldt hier als overal in dit project: family first. Als de omgeving
niet merkt dat het systeem al langere tijd stil is, is dat een signaal
op zich.

## Status

Functioneel compleet (v1): WiFi, tijd, PIR-detectie, de 3-tabs
webinterface, Telegram-meldingen (inclusief opstartmelding, cap van 3
met cooldown, rustmodus met wekelijkse geruststellingsmelding), en de
status-LED werken allemaal. De alarmdrempel en severity-indeling zijn
nog niet in de praktijk gevalideerd — het systeem moet nog een aantal
weken draaien om een zinvolle baseline op te bouwen.

## Licentie

Zie `LICENSE.txt`.
