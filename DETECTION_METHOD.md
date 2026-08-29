> **This system is never a replacement for human or professional care —
> at most a supplement.** It only observes *whether* there is movement,
> makes no decisions, and does not claim to be a safety guarantee.

# Detection method — esp-motion-absence-detection

This document is the **exact** specification of the detection method.
For the high-level picture, see [README.md](README.md); for the
motivation and background, see [PHILOSOPHY.md](PHILOSOPHY.md).

It deliberately keeps three concepts separate, because they are easy to
conflate: **ticks**, **severity**, and **alarm**.

---

## 1. PIR → ticks

**Ticks are counted, never judged.** A tick is a raw, unweighted event —
it carries no meaning about whether a block is "quiet" or "busy" by
itself. Only its rank relative to the historical average of that same
cell (§4) determines severity, and only severity — never a raw tick
count — feeds the alarm calculation (§5). Read that as the single most
important sentence in this document: if you take away only one thing,
take away that raw counts are never compared to a fixed number anywhere
in this method, only ever to their own cell's history.

Every PIR-detected motion event, after debounce, is one **tick**.

- **Debounce**: a minimum of 1 second between two counted events, applied
  in the interrupt handler itself. Without it, a few seconds of hand-waving
  produced roughly a thousand raw counts from electrical noise / rapid
  retriggering on the PIR output. This debounce is a deliberate design
  choice and must not be removed — it also keeps the live count on the
  Status/Logging tab readable, and prevents noise from unfairly inflating
  one block's tick count relative to others. The value is set at compile
  time (`PIR_DEBOUNCE_MS` in `motion_tracking.cpp`), separately for each
  board (`#if defined(ESP8266)` / `#elif defined(ESP32)`), currently 1
  second on both — this split exists so each device can be tuned and
  reflashed independently if needed, not because the two boards actually
  require different values.

- The ESP counts ticks per 4-hour block:

  ```
  00:00–04:00
  04:00–08:00
  08:00–12:00
  12:00–16:00
  16:00–20:00
  20:00–24:00
  ```

- **The absolute tick count has no meaning for the alarm by itself.** It
  only matters relative to the other blocks (see §4) and, separately, as
  the raw trigger for the bootstrap fallback (§7) and the flat safety net
  (§8).

---

## 2. History

For each (weekday, block) cell, the ESP keeps the tick count of the last
**6 weeks**:

```
6 weeks × 7 weekdays × 6 blocks = 252 values
```

- **Round-robin / rolling history**: the oldest week's value for a cell is
  replaced when a new week's value for that same cell arrives.
- This history is internal calculation data only. It is never exposed
  day-by-day in the web UI (see §10, privacy boundary). No data leaves
  the device except the Telegram notifications themselves — there is no
  external integration or cloud dashboard of any kind.
- A cell distinguishes two states for each of its 6 stored week-values:
  - `0 ticks` — a genuine, fully-measured block with no motion.
  - *invalid / not filled* — no reliable measurement for that week yet
    (either because history hasn't reached that week, or because the
    block was excluded per §6 or §5d).
  - These are never conflated: a missing measurement is not the same as
    a zero measurement.
  - A round-robin slot that is *skipped* this cycle (§6/§5d) simply keeps
    whatever value — filled or not-filled — it already held from 6 weeks
    earlier. There is no third "skipped" state; skipping a write is
    indistinguishable, in storage, from "this cycle hasn't come around
    yet."

---

## 3. Bootstrap period (first 3 weeks per weekday)

For a given weekday, until **3 valid historical measurements** exist for
that weekday, there is not yet enough data to build a personal severity
profile for it.

During this bootstrap period for that weekday, the stricter fallback rule
in §7 applies *in addition to* the permanent flat safety net (§8).

Once a weekday reaches 3 valid measurements, a first severity profile is
computed for it (§4). The profile is recomputed again after 4, 5, and 6
weeks. After 6 weeks the 6-week history for that weekday is fully
populated and keeps rolling.

---

## 4. Severity (0–3)

For each block, activity is judged **relative to the other blocks**, not
by its absolute tick count.

```
0 = lowest activity
1 = low activity
2 = high activity
3 = highest activity
```

Severity is the *meaning* of the tick count, not the tick count itself.

- **Ranking method**: severity is derived from each block's rank among
  the relevant set of historical averages (rank = number of blocks with a
  lower average). This is deterministic: blocks with equal average tick
  counts get equal rank and therefore equal severity. No artificial
  differentiation is introduced when the data itself shows no difference
  — a resulting distribution does not have to be a neat 0/1/2/3 spread
  across the six blocks of a day.

Two severity views are calculated and **both shown on the status page**,
so the method's behaviour can be inspected and compared:

- **(A) Per-weekday**: Monday is ranked against historical Mondays,
  Tuesday against historical Tuesdays, etc.
- **(B) All-days-combined**: all available days are pooled into one
  general activity profile, independent of weekday.

---

## 5. Alarm calculation

After each **fully closed** timeblock (see §6 for what "fully closed"
excludes), the severities of the preceding consecutive closed blocks are
summed. **Tick counts themselves are never summed — only severities.**

```
alarm_sum = severity(block t) + severity(block t-1)
if alarm_sum < threshold:
    alarm_sum += severity(block t-2)

ALARM if alarm_sum >= threshold   (threshold is configurable, default 6)
```

The threshold is set via a **sensitivity preset** on the Settings tab
rather than a free numeric field, so it can't accidentally be set to a
nonsensical value. The preset is applied by default and can be changed
by the user:

| Preset           | Threshold |
|------------------|-----------|
| Less sensitive   | 7         |
| Normal (default) | 6         |
| More sensitive   | 5         |

A block only contributes to `alarm_sum` if it counts as "quiet" for
alarm purposes — see §5a for that definition. Blocks that don't count as
quiet do not contribute their severity to the sum (their severity may
still be 0 or low, but the *quiet* qualification below is the actual
gate).

### 5a. When does a block count toward the alarm sum?

A block counts as "quiet" for the alarm calculation when its tick count
is **below 30% of the historical average** for that specific cell
(weekday + block) — **not** a strict "0 ticks" requirement. A strict
zero-ticks rule was considered and rejected: a single incidental tick
from noise, even while the sensor is otherwise silent, would otherwise
wrongly disqualify an actually-quiet block from ever contributing to the
alarm sum. The 30%-of-average threshold is more robust.

### 5b. Alarm cooldown and a hard cap of 3 notifications per episode

Once an alarm has fired, the **next alarm notification is suppressed for
3 further timeblocks** (12 hours), even if the alarm condition is
re-evaluated as true again during that window. This prevents repeated
notifications for what is effectively the same ongoing situation.

- The cooldown counts elapsed *closed timeblocks*, not wall-clock time
  directly (so it aligns with the block boundaries used everywhere else
  in the method).
- The cooldown does **not** suppress the alarm *state* itself (the
  underlying condition keeps being evaluated) — it only suppresses
  sending another notification within that window.

**Hard cap.** Within one continuous alarm episode (i.e. no motion has
been detected since the episode began), **at most 3 notifications are
ever sent in total**: the first, then two more, each after a 3-block
cooldown (roughly a 24-hour span). This cap applies across *all*
notification types together within the episode — the severity-based
alarm (§5), the flat safety net (§8), and the bootstrap fallback (§7)
all share one counter, one episode. After the 3rd notification, the
system enters **rest mode** (§5d) instead of sending a 4th.

Rationale: if the family does not respond to 3 notifications over roughly
24 hours, continuing to notify does not help — either they are already
aware (e.g. the person is on holiday) or, in the worst case, the family's
lack of response is the family's problem, not something more
notifications from the device can fix. See also the note added to
`PHILOSOPHY.md`: this system is a supplementary aid, not a replacement
for family care, and does not claim to be one.

**Notification wording.** Only notification 1 is a plain alarm. To keep
the family informed about what happens next, notifications 2 and 3 each
carry an added line about the approaching/starting rest mode:
- **Notification 2** adds: if the next notification also finds no
  motion, the system will enter rest mode after it.
- **Notification 3** adds: the system is entering rest mode now — no
  further notifications and no further baseline learning until motion
  resumes, with a weekly reassurance message in the meantime (§5d).

This wording lives in `main.cpp` (which owns exact message text; see
`motion_tracking`'s `motion.notificationCount`, which already reflects
the current notification number — 1, 2, or 3 — at send time), not in
`motion_tracking` itself.

**Failed sends still count.** The Telegram call is fire-and-forget
(`WiFiClientSecure::setInsecure()`, no delivery confirmation is tracked).
If a send attempt fails (e.g. WiFi or the Bot API is unreachable) at the
moment the 3rd notification would go out, the *attempt* still counts
toward the cap of 3, and the system proceeds into rest mode regardless.
Adding retry/delivery-confirmation logic was deliberately rejected as
unnecessary complexity (KIS) — consistent with how sending is already
handled elsewhere in this project.

### 5c. Alarm state machine

```
NORMAL
  ↓  (alarm_sum >= threshold, and not in cooldown)
ALARM  →  notification 1 sent
  ↓  (motion detected)              ↓  (3-block cooldown elapses, still no motion)
RESET → NORMAL                   ALARM → notification 2 sent
                                     ↓  (motion detected)              ↓  (3-block cooldown elapses, still no motion)
                                  RESET → NORMAL                   ALARM → notification 3 sent
                                                                       ↓  (motion detected)              ↓  (no motion — cap reached)
                                                                    RESET → NORMAL                   REST MODE (§5d)
                                                                                                           ↓  (first motion detected)
                                                                                                       RESET → NORMAL
```

In words: **any** tick, at any point before the 3rd notification, resets
the whole episode immediately back to `NORMAL` and the notification
counter back to 0 — this already follows from the existing "motion
always resets to NORMAL" rule, it is just spelled out explicitly here so
it isn't mistaken for a new gap once rest mode is introduced.

On the first tick after an alarm (whether that reset happens before or
after entering rest mode):
- alarm state resets to `NORMAL`,
- the notification counter and cooldown are both cleared,
- `last_motion` timestamp is updated,
- evaluation restarts from a clean slate.

No additional hysteresis is used beyond this.

### 5d. Rest mode

If no motion occurs before the 3rd notification's cooldown elapses, the
system enters **rest mode** instead of sending a 4th notification.

- **Counting continues as normal.** The live tick count keeps updating
  and stays visible on the Status/Logging tab exactly as in normal
  operation — rest mode is not a separate UI mode. This was a deliberate
  choice: it's the simplest option (no extra UI state), and it doesn't
  weaken the existing privacy boundary (§10), since a live "current
  block" count never exposed day-level history to begin with, rest mode
  or not.
- **Baseline exclusion, decided at write time — an active episode is
  never learned.** A block is excluded from the 6-week baseline (§2) if,
  at the moment it closes, an episode was **already** running — i.e.
  `notification count > 0` or rest mode was already active, evaluated
  using the state as it stood *before* this block's own alarm evaluation
  runs. Concretely: the block (or two blocks) whose own evaluation
  triggers notification 1 for the very first time is still written to
  the baseline — at the moment it closed there was no confirmed episode
  yet to distrust it. Every block from notification 1 onward — through
  notification 2, 3, and the whole of rest mode — is excluded. This is
  the *same* exclusion mechanism as the reboot rule in §6 (a skipped slot
  simply keeps its value from 6 weeks earlier, see §2), applied to a
  second, independent trigger. **No block is ever retroactively removed**
  from an already-stored week — the exclusion is a one-time decision made
  at write time, using only state already available, so no extra
  bookkeeping is needed to track which blocks belong to which episode
  (KIS). Any short quiet stretch that turns out *not* to escalate into a
  full episode is therefore absorbed into the baseline like any other
  seemingly-quiet stretch, exactly as intended — only a confirmed episode
  is excluded.
- **Resuming from rest mode**: the block in which the first new motion
  falls is treated the same as a partial/reboot block (§6) — it is
  **not** counted as a valid baseline measurement, only the next fully
  closed block after that counts again. This is a deliberate simplicity
  choice: tracking the exact moment within a block when someone first
  returns would add complexity for a marginal, and privacy-sensitive,
  gain — it reuses logic the system already has.
- **Passive status indicator.** A `rest_mode` state is shown as a status
  line on the Status tab, so it's visible at any time that the system is
  not currently learning and is waiting for the first motion. This is a
  passive status display, not a notification, and does not count against
  the cap in §5b.
- **Weekly reassurance message.** While in rest mode, one low-frequency
  message is sent **once per week** (independent of the notification cap
  in §5b, which only applies within the original alarm episode) to
  confirm the system itself is still running. The message states that
  the system has been in rest mode since the last recorded motion
  (`last_motion` date/time) and that it is still operating normally. This
  is a reassurance heartbeat, not an escalation — it does not repeat more
  often, and it stops the moment motion resumes and the system leaves
  rest mode.

---

## 6. Reboot handling

**A block is only used as learning data once it has been measured in
full.**

If the ESP reboots partway through a block (e.g. at 10:00, inside the
08:00–12:00 block), that block is **not** stored as a valid measurement
when it closes — it is marked invalid/not-filled for that cell's history
(see §2), never as `0 ticks`. This prevents a partial measurement from
being wrongly interpreted as an extremely quiet period, which would
otherwise pull that cell's baseline down unfairly.

Concretely: the **first block boundary encountered after any boot** is
always excluded from baseline learning, regardless of how much of that
block was actually measured.

This is one of two independent exclusion rules that share the same
underlying mechanism (a skipped round-robin write, see §2) — the other
being the episode-based exclusion described in §5d. Either rule, on its
own, is enough to exclude a given block; they are never combined into a
single check with different behaviour.

---

## 7. Bootstrap fallback (temporary, per weekday, first 3 weeks)

While a given weekday is still in its bootstrap period (§3, fewer than 3
valid measurements for that weekday), the following stricter,
block-aligned rule applies **in addition to** the permanent flat safety
net (§8):

> **4 consecutive fully-elapsed 4-hour blocks with 0 ticks →
> notification**, evaluated only for that weekday.

This rule is block-aligned (16 hours, on block boundaries), not a rolling
window. It stops applying to a given weekday once that weekday has 3
valid measurements and a severity profile takes over — but the permanent
flat safety net below keeps running regardless, for every weekday, always.
This threshold is **configurable on the status page** (not hardcoded).

---

## 8. Flat safety net (permanent)

Independent of the learned severity model, and independent of the
bootstrap fallback above:

> **16 hours of continuous, uninterrupted absence of motion →
> notification**, regardless of weekday/bootstrap status.

This threshold is **configurable on the status page** (not hardcoded).
It runs permanently — both during the first 6 weeks (before any baseline
exists) and afterwards, as a standing extra layer of safety that costs
nothing to keep.

---

## 9. Pattern instability ("onrust") — a separate signal

This is tracked **independently of the acute alarm** above, and reported
differently (never as its own notification — see below).

### 9a. What it detects

Not a single quiet moment, but whether a person's **whole movement
pattern is shifting** over the weeks — an early signal, separate from any
one acute event. It is symmetric by design: it flags both **less**
movement than usual (e.g. spending much more time in bed/asleep) and
**more** movement than usual during normally-quiet periods (e.g.
nighttime wandering, a recognized pattern in memory-loss cases).

### 9b. Level and calculation

- **Level**: whole-weekday totals (sum of a day's 6 blocks), not
  per-cell — simpler to compute and explain than 42 separate checks.
- For each weekday, compute the mean and standard deviation of the daily
  total across the filled weeks in history (at least 2 filled weeks
  needed for this to be meaningful).
- If the current week's total for that weekday deviates from that mean by
  more than **1.5×stddev**, in either direction, that day is flagged as
  "deviant."
- If **3 or more of the last 7 processed days** are flagged deviant, the
  overall "onrust" (pattern-instability) flag is set.

### 9c. Delivery

- **Never** sent as its own Telegram message.
- Only appended as an extra line of text to an absence-alarm message,
  when one is already being sent.
- If no absence-alarm ever fires, no onrust notification is sent either —
  normal family contact is assumed to cover that case.

### 9d. Repeated-vs-spread deviation

- The **same** cell/timeslot deviating repeatedly (e.g. Monday 00–04
  deviant three weeks running) is **not** immediately treated as
  instability — it may simply be a new habit. The 6-week rolling history
  will eventually absorb it into the new normal pattern.
- Deviations **spread across different days/times** (e.g. Mon 00–04, Wed
  12–16, Fri 16–20, Sun 04–08) are treated as a genuinely suspicious
  irregular pattern, and factored into the onrust flag as described
  above.

---

## 10. Privacy boundary (web UI)

Never expose day-level history of past weeks — that would be a
re-identifiable presence log ("Monday 14 July: 5 movements"). Permitted:
the *learned average pattern* per block, aggregated across the 6-week
history, which cannot be traced back to a specific date. This boundary
applies equally to every new addition to the webinterface, even when a
day-level view would be "convenient for debugging." There is no external
integration of any kind (no MQTT, no cloud dashboard) — the only data
that ever leaves the device is the Telegram notification text itself.

---

## 11. Status LED

An optional status LED mirrors the debounced tick stream 1:1: it fires on
every counted tick, the same events that drive the live count on the
Status/Logging tab — no separate debounce or timing logic. This is
deliberately the simplest possible behaviour (KIS): the LED is a physical
echo of "a tick was just counted," nothing more (not an alarm indicator,
not a rest-mode indicator).

The PIR input pin and the LED output pin are both configurable, but as
**compile-time** settings in `pinout.h`, not runtime/web settings —
consistent with treating hardware pin assignment as a build-time concern,
not something that should be editable (and potentially misconfigured)
through the web UI. A sensible default pin is preset for each. The LED
output circuit is always present in the firmware; whether the physical
LED is actually installed in a given enclosure is up to whoever builds
it.

---

## 12. Person identifier and startup notification

### 12a. Person identifier

`secrets.h` holds a free-text **person identifier** field (e.g. a family
relation like "Father", a name, or an organizational client/registration
number). This value is included in every Telegram notification — the
alarm messages (§5b), the weekly rest-mode reassurance (§5d), and the
startup notification (§12b) — so that in a
multi-client/organizational deployment it is always clear which sensor
and which person a given message concerns.

This is a free field with no format validation. Because it is transmitted
in every message, the setup instructions must warn against putting
directly identifying information in it that isn't necessary — e.g. a full
home address — a short name, relation, or client number is sufficient and
safer.

### 12b. Startup notification

On every boot, once WiFi and Telegram are confirmed working, the system
sends exactly one **startup notification** — independent of, and not
counted against, the 3-notification cap in §5b (it's not an alarm, it's a
one-time "I'm alive and connected" message). It states that the sensor
for `<person identifier>` is now connected, and that settings can be
changed by anyone physically in the room (on the home network) via the
board's IP address.

---

## 13. Notification behaviour summary

- One startup notification per boot (§12b) — not counted against the cap
  below.
- One Telegram message per alarm edge (transition into `ALARM`), subject
  to the 3-block cooldown in §5b, up to a hard cap of **3 notifications
  per continuous episode** across all notification types combined.
- Onrust note appended to any of those 3 messages when the flag is active
  (§9c).
- After the 3rd notification with no motion in between, the system enters
  **rest mode** (§5d): live counting continues, baseline learning pauses,
  and a single reassurance message is sent once per week until motion
  resumes.
- Every notification (startup, alarm, weekly reassurance) includes the
  person identifier from `secrets.h` (§12a).
- Any motion at any point resets the episode to `NORMAL` immediately,
  clearing the notification counter, cooldown, and (if applicable) rest
  mode.

---

## 14. Web interface block numbering (display only)

Internally, a block within a day is a 0-indexed array position (0–5),
matching `config.cells[day][block]` and every calculation in this
document. The web interface shows this to the user as blocks **1–6**
instead — a display-only `+1` applied where the block number is rendered
in `webinterface.cpp`, nowhere else. No internal logic, storage index, or
calculation anywhere in this document or the code uses 1-indexed block
numbers; this section exists solely so a reader comparing the UI to this
document or to the code isn't confused by the difference.
