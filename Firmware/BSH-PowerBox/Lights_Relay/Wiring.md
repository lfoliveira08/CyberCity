# Lights Relay — BSH-PowerBox

Reference doc: `claude/BSH_Sensors/09_Relay_Module.md`

## What it does here
Switches the building's illumination lighting fully on/off — its own
dedicated 1-channel relay module as of Sep 30, 2026 (previously the
second channel of a shared 2-channel module with the fan -- see
`../Fan_Relay/Wiring.md` and the Sep 30, 2026 entry in
`Piranga_Workflow_v2.md` Section 14). This board has its own `VCC`/`GND`
pair wired to the ESP32, separate from the fan relay's.

**3 LEDs, one per floor (Sep 30, 2026):** "the light" on this channel is
physically 3 LEDs, one representing each of BSH's three floors, wired in
parallel on the relay's single switched output — see "Wiring" below. They
are controlled together as one group: this stays a single relay channel,
a single GPIO, and a single Modbus coil (`LIGHTS_ON_CMD`), same as before.
There's no independent per-floor control — all 3 floors' lights turn on
and off together whenever this channel is commanded.

**Design change (Sep 28, 2026):** this channel originally drove a DALI
transceiver/ballast for individually dimmable lighting. DALI was dropped in
favor of this plain relay channel — see
`claude/BSH_Systems/04_BSH-PowerBox.md` Notes and `Piranga_Workflow_v2.md`
Section 14. The old DALI wiring notes that lived in this folder (formerly
`DALI_LED_Driver/`) have been superseded; see the retired
`claude/BSH_Sensors/12_DALI_LED_Driver.md` stub and
`References/GitHub/PowerBox_silahcen001_DALI-ESP32.md` if you need the
history.

## Wiring
| Signal | ESP32 pin | Notes |
|---|---|---|
| Lights relay IN | GPIO 13 | Its own 1-channel relay module -- separate board from the fan relay as of Sep 30, 2026 |
| Lights relay VCC | 5V | This board's own VCC, not shared with the fan relay module |
| GND | Common ground | |

The relay's switched output (NO/COM) feeds all 3 LEDs **in parallel**, each
with its **own** series resistor (don't share one resistor across all
three — their forward voltages and currents aren't guaranteed identical,
and a single shared resistor won't current-limit each LED correctly).
Three LEDs at typical ~20mA each is about 60mA total through the relay's
switched contact — trivial for any relay module rated for real AC/DC
loads, so no special consideration needed there.

**Relay polarity — confirmed Oct 5, 2026: `RELAY_LIGHTS_ACTIVE_LOW = false`
(active-HIGH)** on this specific board, verified via the Phase A combined
standalone integration sketch (see `Build_Log.md`). This does not match
the common "most cheap modules are active-LOW" assumption, and it is not
assumed to match the fan relay's result just because both turned out
active-HIGH — each was tested independently. (Note: the Wokwi Phase A0
sim also needed active-HIGH, a `wokwi-relay-module` simulator quirk — it
happened to land on the same value as this real board, but that was
coincidence, not evidence, confirmed by testing fresh here. See
`../Wokwi/README.md`'s "Relay polarity" section.)

## Firmware tie-in
`BSH_PowerBox_Node.ino`: `PIN_LIGHTS_RELAY = 13`, driven by `setRelay(pin,
on, activeLow)`, called with `RELAY_LIGHTS_ACTIVE_LOW` (its own
independent flag as of Sep 30, 2026, set to `false` as of Oct 5, 2026).
Commanded by Modbus Coil `LIGHTS_ON_CMD` (Coil0), and reported back via
Discrete Input `LIGHTS_STATUS` (DI0) — see the local override button
addition in `claude/BSH_Sensors/02_Push_Button.md` and the Sep 29, 2026
entry in `Piranga_Workflow_v2.md` Section 14. Modbus-level verification
(Phase B) is still pending.

**GPIO correction (Sep 28, 2026):** originally assigned GPIO 12 (carried over
from the old DALI TX pin). GPIO 12 is an ESP32 strapping pin (MTDI) that
samples flash voltage at reset -- a relay module's onboard IN pull-up drags
it HIGH during that window, which was suspected of causing inverted/
unreliable behavior. Moved to GPIO 13. **Update (Sep 29, 2026):** that move
was worth keeping but turned out not to explain the actual inversion seen
in Wokwi — see `../Wokwi/README.md`'s "Relay polarity" section. See
`00_START_HERE.md` Section 7 for the still-valid general strapping-pin
caveat.

## Bench-test checklist (Phase A, this component only)
- [x] Relay module's lights channel wired per the table above — its own
      board, own VCC/GND (not the fan relay's)
- [x] All 3 floor LEDs connected in parallel to the relay's NO/COM output,
      each with its own series resistor
- [x] Flashed and confirmed via the Phase A Step 4 combined integration
      sketch — all 3 LEDs follow together
- [x] Confirmed active-LOW vs. active-HIGH against **this specific
      module** — active-HIGH (`false`), Oct 5, 2026
- [x] Confirmed all 3 LEDs are roughly equally bright

## Lessons Learned
- Don't carry a polarity result from one relay board to another, even
  when they used to live on the same 2-channel module -- as of Sep 30,
  2026 they're physically separate boards and always were independent
  circuits regardless. (This channel and the fan channel both ended up
  active-HIGH on Oct 5, 2026 -- a coincidence worth noting, not a reason
  to skip testing FH-PowerBox's boards independently.)

## Troubleshooting Tips
- One LED noticeably dimmer or dark while the others light up: check that
  LED's own resistor and connection — a parallel branch with a bad
  connection just goes dark, it doesn't affect the other two.
- Lights behave opposite of the commanded state: flip
  `RELAY_LIGHTS_ACTIVE_LOW` and re-flash — confirmed correct value for
  this board, Oct 5, 2026: `false`.
- Zero Serial output at all when pressing the lights button: confirm the
  flashed sketch is actually current before suspecting wiring.
