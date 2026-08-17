# esp-motion-absence-detection

Privacy-first movement monitoring for people who live alone. A single
PIR sensor in the living room, on an ESP8266 (Wemos D1 mini), learns the
normal movement pattern per weekday and time block, and notifies family
via Telegram when that pattern deviates significantly — no camera, no
audio, no identification, and no external data connection of any kind
other than Telegram itself.

See [`PHILOSOPHY.md`](PHILOSOPHY.md) for the background and related
projects, and [`DETECTION_METHOD.md`](DETECTION_METHOD.md) for the exact
calculation method.

## What it does

- A single PIR motion sensor registers only **whether** there is
movement — no image, no audio, no location within the home, and no
external data connection other than the Telegram notification itself.
- The system learns what's normal per weekday and time block (6 blocks
of 4 hours each), based on a rolling average over the last 6 weeks.
- If the current movement pattern deviates significantly — too little
movement during a normally active block — a Telegram message goes out
to the family.
- A flat safety net also covers the first few weeks, before enough
history has built up, and then stays active afterwards as a permanent
backstop.
- A maximum of 3 notifications per continuous episode (with a cooldown
of 3 time blocks in between). If the family still doesn't respond
after that, the system stops notifying *and* stops learning (rest
mode), and checks in briefly once a week to confirm it's still
running.
- A separate "pattern instability" signal picks up on the whole daily
rhythm becoming more irregular than usual — in both directions (too
little movement, but also unusually much movement during normally
quiet hours, such as nighttime wandering).
- An optional, dimmed green LED gives a brief visual confirmation on
every registered movement.

All thresholds and timing settings above are configurable via the
**Settings** tab of the web interface — see
[`DETECTION_METHOD.md`](DETECTION_METHOD.md) for the exact calculation
method and default values.

## Privacy

The system is deliberately designed to **never** display or send a
day-by-day presence log, and no data leaves the house other than the
Telegram notification itself (no cloud dashboard, no external
integrations). Only the aggregated, learned pattern is visible in the
web interface, never traceable back to a specific date.

## Language

The web interface and all Telegram messages are available in **Dutch,
English, German, French, and Spanish**. The language is chosen at
**compile time**, not as a runtime setting — every deployment is a
single device someone builds for themselves or their organization, so
there is no need for a language switch in the web interface itself,
and it keeps the firmware simple and small.

To choose a language, open `include/lang_config.h` and uncomment the
line for your language (leaving exactly one active):

```cpp
#define LANG_EN
// #define LANG_NL
// #define LANG_DE
// #define LANG_FR
// #define LANG_ES
```

Then rebuild and flash. The repository defaults to `LANG_EN`.

The debug log (visible on the **Log** tab, intended for development and
tuning rather than everyday use) always stays in Dutch, regardless of
this setting.

Translations are contributed and maintained on a best-effort basis —
corrections and additional languages are welcome via a pull request;
see `include/lang.h` / `src/lang.cpp` for how the existing languages are
structured.

## Web interface

The web interface has three tabs:

- **Status** — current time block, live tick count, the learned pattern
(both per weekday and combined across all days), current alarm/
instability status, the number of notifications in the current episode
(0-3), and whether the system is in rest mode.
- **Settings** — sensitivity (Less sensitive / Normal / More sensitive,
default Normal — see [`DETECTION_METHOD.md`](DETECTION_METHOD.md) for
the exact threshold values per option), flat-safety-net hours, the
bootstrap-fallback hours for the first few weeks, and a button to send
a Telegram test message.
- **Log** — live PIR events and the debug log.

## Installation

1. Copy `include/secrets.h.example` to `include/secrets.h` and fill in:
  - your WiFi credentials
  - a Telegram bot token and chat id (see below)
  - a **person identifier field**: a short name or label (e.g.
"Father", "Mrs. Smith", or a client number for an organization)
that is included in every Telegram message, so that with multiple
sensors it's always clear who the message concerns. **Note:** do
not enter directly identifying information here such as a full
address — a name, relation, or client number is sufficient and
safer.
2. Open `include/lang_config.h` and choose the language for the web
interface and Telegram messages (see [Language](#language) above;
defaults to English).
3. Connect a PIR sensor (default pin **D2**, changeable in `pinout.h`),
powered by **5V** (not 3.3V — most PIR modules need that to
function).
4. Optionally connect a dimmed green LED as visual confirmation that the
system is registering movement (default pin **D1**, also changeable
in `pinout.h`, independent of the PIR pin) — see
[Optional LED](#optional-led) below. The LED output is always present
in the firmware; whether the LED is actually installed physically is
up to whoever builds the enclosure.
5. Open the project in VS Code with the PlatformIO extension. Choose the
right `default_envs` in `platformio.ini` (`d1_mini` or `esp8266dev`,
depending on your exact board) and flash.
6. On every boot, the system sends one Telegram message stating that the
sensor for the configured person identifier is connected, along with
the IP address where settings can be changed (reachable only from the
home network). This message does not count toward the notification
limit described below.
7. Open the board's IP address in a browser for the Status, Settings,
and Log tabs.
8. Optionally choose a different sensitivity than the default "Normal"
on the **Settings** tab, or adjust the safety-net hours. A "send test
message" button on the same tab lets you verify the Telegram
connection at any time.

### Creating a Telegram bot

1. Search Telegram for **BotFather**, send `/newbot`, follow the steps.
You'll get a bot token back.
2. Send a message to your bot (privately, or add it to a group and send
a message there).
3. Open `https://api.telegram.org/bot<YOUR_TOKEN>/getUpdates` in a
browser and look for the `chat.id` field — that's your chat id.
4. Put both values in `secrets.h`.

## Hardware

- Wemos D1 mini (ESP8266EX, 4MB flash)
- PIR sensor (e.g. HC-SR501) on default pin D2 (GPIO4), powered by 5V —
pin configurable in `pinout.h` (compile-time)
- Optional: dimmed green LED on default pin D1 (GPIO5), also
configurable in `pinout.h`

### Optional LED

A dimmed green LED can be connected as a simple visual confirmation that
the system is actively registering movement: the LED mirrors every
registered tick 1:1 (not an alarm indicator, and not meant to be
visible/relevant to the resident during their absence — purely a
"system is alive" signal for whoever happens to be nearby, e.g. during
installation or maintenance). The LED output is present in the firmware
by default; installing it physically is entirely optional.

## A note on WiFi and power outages

If the board itself goes offline (WiFi down, power outage, crash), this
project does not actively report that to the family — there is no
separate "the system has gone down" notification. The same principle
applies here as everywhere else in this project: family first. If the
surroundings don't notice that the system has been silent for a while,
that is itself a signal.

## Status

Functionally complete (v1): WiFi, time sync, PIR detection, the 3-tab
web interface, multi-language support (Dutch/English/German/French/
Spanish), and Telegram notifications (including the startup message,
the cap of 3 with cooldown, and rest mode with its weekly reassurance
message) all work, as does the status LED. The alarm threshold and
severity breakdown have not yet been validated in practice — the system
still needs to run for a number of weeks to build up a meaningful
baseline.

## Why is this project public?

This project is deliberately open.

The goal is not to build a proprietary care-monitoring system, but to make a simple and inexpensive approach available to anyone who wants to experiment with improving care for people living alone.

The hardware is inexpensive, the algorithm is deliberately explainable, and the limitations are explicitly documented.

If this project gives someone an idea for a better sensor, a better algorithm, a better care process, or an entirely different solution, then the project has achieved its purpose.

## License

See `LICENSE.txt`.

## Credits

Design and detection method by Poppink. Implementation by Claude (Anthropic).
