# Gas/Smoke Sensor ("MQ-2" slot) — BSH-FireAlarm

Reference doc: `BSH_Sensors/16_MQ-2_Gas_Smoke_Sensor.md`

## ⚠️ Hardware note — actual part used differs from the documented MQ-2
The system doc and reference doc assume an **MQ-2** sensor (tuned for
combustible gas/LPG/smoke). The actual part on the bench, confirmed by the
model number printed on the metal sensor can, is an **MQ-9** — a
generic "MH Sensor series" breakout board (same visual family as the flame
sensor module), tuned primarily for carbon monoxide (CO) plus some
combustible gases (methane, LPG). This folder keeps the name
`MQ-2_Gas_Smoke_Sensor/` for consistency with the rest of the project's
folder structure, but everywhere below assume MQ-9 hardware.

**Decision (2026-09-17):** proceed with the MQ-9 as-is rather than sourcing
a real MQ-2. Reasoning: the demo isn't going to involve actually burning
something to produce real smoke — what gets tested on the bench and shown
in the demo is "does the raw reading cross a threshold when some gas
source is introduced" (a lighter's unburned gas, alcohol vapor, canned
air, etc.), and MQ-series sensors are all broadly cross-sensitive to
combustible gases regardless of which one they're individually tuned for.
The firmware and Modbus register names (`GAS_RAW_VALUE`, `GAS_SMOKE_ALERT`)
are already written generically — they don't encode an assumption about
which specific gas triggered them. Framing shifts from "smoke/LPG
detection" to the more accurate **"general combustible gas detection."**

**Reference used for MQ-9 specifics (2026-09-17):**
[electropeak MQ-9 calibration guide](https://www.hackster.io/electropeak/how-to-calibrate-use-mq9-gas-sensor-w-arduino-e93cb1).
Two notable deltas from what was originally assumed (carried over from the
MQ-2 plan):
- **Preheat time is much longer than assumed.** That guide states the MQ-9
  wants **24–48 hours of preheating** before readings are trustworthy, not
  the "20–60 seconds" warm-up written into the firmware comments and
  earlier version of this doc. Plan bench-calibration sessions around
  this — power the sensor on a day or two ahead, don't expect clean
  numbers from a cold start. A short warm-up (a minute or two) is still
  enough for an initial "is it alive and responding at all" sanity check,
  just not for real calibration.
- **A more rigorous calibration method exists** (Rs/R0 ratio against a
  clean-air reference: `R0 = RS_air / 9.9` per the MQ-9 datasheet) that
  would yield real ppm-ish readings instead of just a relative
  baseline-drift threshold. Not yet adopted — see Calibration section
  below for the decision on this.

## What it does here
Slower, indirect complement to the flame sensor — detects rising
combustible-gas concentration and reports both a raw ADC value (for a
dashboard gauge) and a threshold alert, one of three local trigger sources
OR'd into the node's `alarmActive` logic.

## ⚠️ Before you wire anything (hardware-safety step, applies regardless of MQ variant)
Measure this specific board's AO pin voltage with a multimeter (sensor
powered from 5V, sitting in clean air) and confirm it stays under ~3.3V
before connecting it to GPIO 34. Many MQ breakouts can swing AOUT close to
their 5V supply rail — wiring a 5V-swinging output straight into an
ESP32 ADC pin risks damaging it. If it exceeds 3.3V, add a voltage
divider: 1kΩ from AO to the GPIO, 2kΩ from that same GPIO to GND (same
pattern as the HC-SR04's ECHO divider) — this gives
AO × 2000/(1000+2000) ≈ AO × 0.667.

**Status: physically wired (2026-09-17), voltage check not yet confirmed.**
Wiring was completed before the AO voltage was checked with a multimeter —
recommend verifying it now, before relying on any readings from GPIO 34,
to make sure this board's AO isn't sitting above 3.3V. If it is, add the
divider described above even though the wire is already run.

## Key specs
- "MH Sensor series" MQ-9 module, 4-pin header labeled **AO / DO / GND /
  VCC** (confirmed from board silkscreen)
- Onboard POWER LED (lights when VCC/GND connected) and DOUT LED (reflects
  the digital threshold output) — useful bring-up aids, independent of the
  ESP32
- Onboard sensitivity trim potentiometer (same style as the flame sensor
  module) — sets the DO threshold; not used here since the project
  calibrates a threshold in firmware off the analog value instead
- Analog output (AO) only is read by firmware; DO is wired but unused in
  code (see Wiring table below)
- Onboard heater needs **5V** — do not run it on 3.3V
- Needs a warm-up period before readings stabilize — see the preheat-time
  note above (24–48 hours for a proper calibration; a minute or two is
  enough only for a basic "is it responding" check)
- MQ-9 specifically is designed around an alternating heater-voltage cycle
  (~60s at 5V, ~90s at ~1.5V) to distinguish CO from general combustible
  gas. This project powers it continuously at 5V, same as MQ-2 would be —
  still gives a usable baseline-relative "something's off" signal, just
  without cleanly separating "this is CO" from "this is combustible gas."
  A possible future refinement, not a blocker now.

## Wiring (finalized 2026-09-17)
| Sensor pin | ESP32 pin | Notes |
|---|---|---|
| AO | GPIO 34 | ADC1, input-only — safe alongside active WiFi. GPIO 21 was considered and rejected: it has no ADC hardware at all, `analogRead()` doesn't work there. **Voltage check (see safety note above) still needs to be confirmed even though this is already wired.** |
| DO | GPIO 19 | Digital threshold output. Wired for future use, but `BSH_FireAlarm_Node.ino` does not currently read this pin — no functional effect until firmware is added to use it. No conflict with the rest of the BSH-FireAlarm pin map (23, 25, 26, 27, 4, 18, 34 are the only other pins in use). |
| GND | GND | Shared with the divider's ground leg, if one gets added later |
| VCC | VIN | Heater coil needs 5V — VIN ties to the board's 5V rail, equivalent to wiring straight to a 5V pin |

## Calibration & Threshold Logic
Current firmware approach (unchanged for now): samples the sensor 20 times
at boot to compute `gasBaseline`, then alerts once the live reading
crosses `gasBaseline + GAS_ALERT_MARGIN`. `GAS_ALERT_MARGIN` (currently
400, a placeholder) still needs real bench calibration — against this
MQ-9's actual behavior, not MQ-2's, since sensitivity and baseline levels
differ between the two. This is a deliberate "good enough for the demo"
simplification, not a real ppm measurement — see the Rs/R0-ratio method
in the electropeak reference (linked above) for a more rigorous
alternative if the project wants actual gas-concentration numbers later
instead of just a relative drift-from-baseline flag. Not yet adopted;
revisit if the demo needs real ppm values rather than a threshold alert.

Known limitation carried over: baseline is only computed once, at boot, in
whatever air quality happens to be present at that moment — power on in
clean air. Also note the preheat-time caveat above: a boot-time baseline
taken before the sensor has properly warmed up will drift as the sensor
continues heating, which is a bigger deal now that the real preheat window
(24–48 hrs) is known to be much longer than originally assumed.

## Modbus Register Tie-In
- Input Register `GAS_RAW_VALUE` (IR0) — raw ADC reading (0–4095)
- Discrete Input `GAS_SMOKE_ALERT` (DI2) — set once `GAS_RAW_VALUE` crosses
  `gasBaseline + GAS_ALERT_MARGIN`; see `BSH_FireAlarm_Node.ino`

Register names are kept as originally documented (`GAS_RAW_VALUE`,
`GAS_SMOKE_ALERT`) — they're generic enough to still be accurate for an
MQ-9's "combustible gas" reading, not just MQ-2's "smoke" framing.

## Bench-test checklist (Phase A, this component only)
- [ ] AO voltage verified under 3.3V with a multimeter (add divider if
      needed) — **wiring is already done; verify this now before trusting
      GPIO 34 readings, and retrofit the divider if it fails the check**
- [x] GND + VIN wired
- [ ] POWER LED confirmed lit
- [x] DO wired to GPIO 19 (unused by firmware, wired for future use)
- [x] AO wired to GPIO 34
- [ ] Power-cycle with a throwaway test sketch printing `analogRead(34)`,
      confirm a sane, stable-ish clean-air baseline
- [ ] Introduce a gas source (lighter's unburned gas, alcohol vapor, etc.)
      a foot or so away — raw value should rise noticeably
- [ ] Record the clean-air baseline and the triggered value to sanity-check
      `GAS_ALERT_MARGIN`

## Lessons Learned
- Board turned out to be an MQ-9, not the documented MQ-2 — confirmed by
  reading the model number printed on the sensor can, since the PCB alone
  (same generic "MH Sensor"/"Flying-Fish"-style breakout) is used for many
  MQ variants and doesn't indicate which one by itself. Worth checking the
  actual canister text on any MQ-series module before assuming.
- Decided to proceed with MQ-9 as-is rather than source a real MQ-2, since
  the project's demo and firmware logic are both already gas-agnostic
  (threshold-relative-to-baseline, not tied to a specific chemistry).
- Initial wiring plan (DO → GPIO 21) would not have worked — GPIO 21 has
  no ADC capability on the ESP32. Caught before any pin damage occurred;
  worth double-checking ADC1-vs-digital-only capability against the
  specific ESP32 board's pinout diagram before wiring any analog sensor,
  not just assuming any GPIO number works.
- The 24–48 hour MQ-9 preheat time (per the electropeak reference) is much
  longer than the "20–60 second" figure originally assumed from the MQ-2
  plan — worth building this into bench-test scheduling going forward.

## Troubleshooting Tips
*(fill in as you go — e.g. "baseline way too high/unstable" -> room wasn't
clean air at boot, or sensor hadn't warmed up yet (remember: real preheat
is 24-48 hrs, not a minute); "never alerts" -> check `GAS_ALERT_MARGIN`
isn't set too high for what your test source can actually move the raw
value)*
