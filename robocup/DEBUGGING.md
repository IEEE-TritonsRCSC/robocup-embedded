# Debugging Log — Scaling from 1 motor to 5 (4 wheels + dribbler)

Context: last hardware test (`7d3705e`) validated commands end-to-end on a
single motor (Arduino UNO R4 WiFi + Longan Labs CAN-FD shield + Moteus C1 +
PDB). This log covers the review done before testing all 5 motors, and the
changes made in this pass.

## 2026-07-02

### 1. Kicker never actually pulses — fixed
**File:** `commands.cpp` / `commands.h`

`kick()` set the pin `HIGH` then immediately `LOW` with a `// TODO: delay
100ms without blocking` — the solenoid was never actually energized long
enough to fire (if at all; instruction-level timing is sub-microsecond).

**Change:** `kick()` now sets the pin `HIGH` and records `millis()`. A new
`serviceKicker(KICKER_PIN)` is called every `loop()` iteration
(`robocup.ino`) and turns the pin back `LOW` once `KICK_PULSE_MS` (default
`8`, in `commands.h`) has elapsed. This keeps `loop()` non-blocking, which
matters because CAN sends and UDP polling still need to run during the
pulse.

**Unknown / needs your input:** the header comment block above `KICKER_PIN`
lists `KICK-DIS D2`, `DONE D1`, `CHARGE D0` — this reads like the kicker
board expects a charge/fire/done handshake across 3 pins, not a single HIGH
pulse on D2. Only `KICKER_PIN` (D2) is wired up in code today. If the kicker
board needs to see a "charge" signal before firing or a "done" signal
read back, the current fix is incomplete — I don't have the kicker board's
datasheet/schematic, so I left it as a single-pin pulse. **Please confirm
KICK_PULSE_MS=8ms is correct for your solenoid** (too short = no kick, too
long = risk of overheating/damaging the coil) and let me know if D0/D1 need
to be wired into logic too.

**Test plan:** trigger a `k` (kick) UDP command, verify on a scope/LED that
the pin pulses for ~8ms, then tune `KICK_PULSE_MS` up/down based on actual
kick strength observed on hardware.

### 2. CAN-FD TX FIFO size likely too small for 5 motors
**File:** `commands.cpp` (`configCANFDSettings`)

`mDriverTransmitFIFOSize` was `1`. That was fine for the single-motor test
(one `SetPosition()` per loop), but `sendPositionCommands()` sends up to 4
wheel frames back-to-back every `loop()` iteration (the dribbler is
one-shot, sent separately on catch/drop). With a FIFO depth of 1, the driver
has no buffer between consecutive `SetPosition()` calls in the same loop —
if the bus is busy or a frame hasn't finished transmitting, subsequent sends
may silently drop or spin waiting.

**Change:** bumped `mDriverTransmitFIFOSize` from `1` to `4`, matching
`NUM_WHEELS`. Kept `mDriverReceiveFIFOSize` at `2` since Moteus replies are
less latency-critical here.

**Test plan:** with all 4 wheels connected, watch Serial output for CAN
errors (`CAN error 0x..`) during sustained `dash`/`turn` commands. If you
still see TX overflow/errors, we may need to go higher, or switch to sending
frames spread across multiple loop iterations instead of all 4 in one shot.

### 3. Not changed, but flagged for your awareness
- `sendPositionCommands()` streams wheel commands **every** `loop()`
  iteration unconditionally (even if nothing changed since the last UDP
  command). This is standard for Moteus (it also acts as a heartbeat so the
  controller doesn't fault on staleness), so I left it as-is — just noting
  it's intentional, not an oversight.
- The UDP protocol is a bare, unauthenticated ASCII string
  (`<robotId> <cmd> [arg1] [arg2]`) with no checksum/sequence number. Fine
  for a controlled RoboCup field network; flagging only in case you ever see
  "phantom" commands — that would point to a stray packet from another
  robot's controller rather than a code bug, since all robots likely share
  a broadcast/multicast UDP port.
- `ENABLE_UDP_DEBUG` is `1` by default (verbose Serial logging per packet).
  Fine for now while validating 5 motors, but worth flipping to `0` before
  competition to cut Serial overhead/latency.

## 2026-07-02 (cont.) — Dribbler stops immediately after catch

**Symptom reported:** wheels (via `t`/`d`) worked correctly. `1 c` (dribbler
catch) made the dribbler spin briefly, then stop on its own — well before
the 4s watchdog timeout.

**Root cause found in Serial log:** `Dribbler: -5.00` stayed in the printed
command snapshot the whole time, but the motor still stopped. That means
the *stored* command was right, but the CAN frame wasn't being resent.
`sendPositionCommands()` in `commands.cpp` had:
```cpp
if (i == DRIBBLER_INDEX) {
  continue;
}
```
so wheels get a `SetPosition()` frame every loop (~1kHz), but the dribbler
only ever got one frame, sent once from `dribblerCatch()`. Moteus
controllers have their own internal command-timeout fault and stop the
motor if they don't see frames frequently enough — so the one-shot dribbler
frame caused it to run briefly then self-stop, independent of our 4s
watchdog. The design comment ("one-shot commands so it's not streamed
continuously") was based on an incorrect assumption about Moteus behavior.

**Fix:** removed the `DRIBBLER_INDEX` skip so the dribbler is streamed every
loop just like the wheels. `dribblerCatch()`/`dribblerDrop()` still also
send one immediate frame via `sendSinglePositionCommand()` for a fast first
response; that's now redundant but harmless (the next loop iteration would
pick it up anyway) — left in place since it costs nothing.

**Also worth knowing (not a bug):** every test in your log hit
`WATCHDOG timeout: stopping robot` about 4 seconds after each command. This
is `WATCHDOG_TIMEOUT` (`commands.h`) working as intended — it stops the
robot if no *new* UDP packet arrives within 4s. Since `send_gui.py` appears
to send one-off commands rather than a repeated stream, every command will
self-stop after 4s regardless of the dribbler fix. That's fine for manual
one-shot testing, but for continuous driving your client needs to keep
sending commands (e.g. at 10-20Hz) to hold state, the same way the wheels
already require continuous Moteus frames.

**Test plan:** re-run `1 c`, confirm the dribbler now spins continuously
until either `1 o` (drop) or the 4s watchdog stops it. If it still cuts out
early, check Serial for CAN errors on the dribbler's ID (5) specifically —
that would point to a wiring/bus issue rather than this fix.

## Next steps for you to test
1. Flash with all 5 Moteus controllers wired (CAN IDs 1–5 matching
   `FL/FR/BR/BL/Dribbler` order in `robocup.ino`).
2. Send `dash`/`turn` commands and confirm no CAN errors print on Serial.
3. Send `k` (kick) and confirm actual solenoid fire + tune `KICK_PULSE_MS`.
4. Report back: any CAN error codes, wrong wheel directions/speeds, or
   kicker behavior, and I'll iterate.
