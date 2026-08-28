> **This system is never a replacement for human or professional care —
> at most a supplement.** It only observes *whether* there is movement,
> makes no decisions, and does not claim to be a safety guarantee.

# Philosophy

## The problem

Most systems for people who live alone — often the elderly — are built
on two extremes: camera surveillance or a personal emergency button.

A camera can observe a lot, but it's a major privacy intrusion. An
emergency button is privacy-friendly, but only works when the person is
themselves able and willing to call for help — precisely at the moment
that often isn't possible anymore.

This project deliberately chooses a simple middle ground.

## The starting point

A single PIR motion sensor in the living room registers only **whether**
movement is detected. No image is captured, no audio is recorded, and no
attempt is made to determine *what* someone is doing.

The most important piece of information is actually the **absence** of
movement. The system therefore doesn't try to establish whether someone
is safe — it tries to establish when the absence of movement
**becomes unusual**, and that gives reason to alert the family.

The basic rule is simple: as soon as movement is detected, evaluation
starts over. The number of movements is not important in itself — one
movement and a thousand movements are both equally sufficient for the
current alarm system to establish that movement has occurred. The
counts are only used in the background to determine what's normal at a
given moment.

## Self-learning, but simple

The system keeps a rolling average per weekday and time block (6 blocks
of 4 hours) over the last 6 weeks — a small, local dataset that fits on
the ESP itself, without an external server or cloud. The numbers have no
absolute meaning: it doesn't matter whether a time block normally sees
30 or 30,000 movements. What matters is the ratio to that specific time
block's normal pattern.

The intelligence isn't in recognizing activities, but in **weighing the
absence of movement**. No movement during a period when barely any is
normally registered (e.g. at night) may mean little. No movement during
a period when a lot of movement normally happens is far more notable.
That's why the absence of movement is given a different weight depending
on the time, the day, and what's normally expected there.

Alongside this acute signal, the system also looks at the pattern over a
longer stretch of time: does the whole daily rhythm become more
irregular than usual — in both directions, so also unusually much
movement during normally quiet hours — then that's a separate,
lower-urgency early signal ("pattern instability"), which is included
alongside any acute alarm.

Exactly how all of this is worked out — the calculation rules, the
thresholds, what happens when nobody responds — is in `DETECTION_METHOD.md`;
the technical setup is in `README.md`. This document deliberately stays
at the level of the *why*.

## Privacy by design

Privacy isn't an extra feature, it's a starting point. The sensor
registers no image, no audio, no identity, no location within the home,
no activity type, and no personal content — only: a movement event was
registered. All processing happens locally on the ESP. Nowhere in the
system — not in the web interface — is a day-by-day presence log shown
or sent; only the aggregated, learned pattern is visible.

## Not aiming for perfection

The system cannot establish what is actually going on. Someone might, for
example, sit still or sleep for a long time, and a sensor can miss a
movement. The system therefore does not claim to be flawless safety
monitoring. The goal is simpler: build a system that is significantly
better than nothing, without introducing constant surveillance of the
person to achieve that. It does not take the place of family or
professional care — it only helps an unusual situation come to attention
sooner.

This is also why a simple, inexpensive, imperfect PIR sensor turned out
to be a genuinely good technical fit rather than a compromise to live
with. Because the system never judges an absolute tick count — only the
ratio to that same cell's own learned history (see `DETECTION_METHOD.md`
§4) — a sensor's fixed quirks (too sensitive, too insensitive, noisy)
simply get absorbed into what that specific sensor, in that specific
room, is learned to consider normal. A more expensive or more accurate
sensor would not change the underlying logic; it would only shift the
absolute numbers the system already treats as meaningless on their own.

## Guarding against alarm fatigue

A system that sends too many warnings, or warnings that turn out to be
false too often, eventually gets ignored — the family stops taking it
seriously. This effect, often called *alarm fatigue* or the "cry wolf"
problem, is a real and well-documented risk in hospitals and care homes,
and it applies just as much to a home monitoring system: it is only
useful for as long as the people receiving its messages still trust and
act on them.

This project deliberately limits how often, and how insistently, it
speaks up: at most 3 notifications per unusual period, spaced apart by a
cooldown, after which the system falls silent into a rest mode rather
than keep repeating itself (see `DETECTION_METHOD.md` §5b). For the same
reason, there is deliberately no routine "everything is fine" message —
a system that talks constantly trains people to stop listening, which is
exactly the opposite of what's needed the one time it truly matters.

## KIS

The project deliberately follows the **Keep It Simple** principle. The
hardware is, in essence, an ESP and one motion sensor. The software
consists of: register movement → track data per time block → determine
the normal pattern → weigh the absence of movement → notify family if
needed. No camera, no complex sensor networks, no heavy AI, and no large
database.

The strength of the system lies in the simple question it tries to
answer:

> **When is "no movement" unusual enough to have someone check that
> everything is okay? (family or professional care)**

## Related projects

- **Automatic Alarm System for the Elderly Living Alone (Japan)** —
  [jstage.jst.go.jp](https://www.jstage.jst.go.jp/article/jami/26/1/26_1/_article/-char/en) —
  PIR sensors for elderly people living alone, based on the absence of
  movement.
- **Seoul National University — Nonresponse Interval** —
  [snu.elsevierpure.com](https://snu.elsevierpure.com/en/publications/detection-of-abnormal-living-patterns-for-elderly-living-alone-us) —
  introduces the *Nonresponse Interval*, of which this project is
  effectively a heavily simplified variant.
- **Low-cost Binary Sensors (2019)** —
  [mdpi.com](https://www.mdpi.com/1424-8220/19/10/2264) /
  [PubMed](https://pubmed.ncbi.nlm.nih.gov/31100824/) —
  shows that cheap binary sensors, including PIR, can signal both acute
  deviations and slowly changing patterns.
- **IBED — Inactivity-Based Emergency Detection** —
  [github.com/WilhelmSebastian/IBED](https://github.com/WilhelmSebastian/IBED) —
  an open-source project with a similar goal, also PIR-based.

What sets this project apart is mainly its emphasis on **fully local
processing on cheap consumer hardware** (one ESP8266 + one PIR sensor,
no server, no cloud) and its explicit, hard privacy boundary: nowhere is
a traceable day-by-day log visible — not even to the user themselves.