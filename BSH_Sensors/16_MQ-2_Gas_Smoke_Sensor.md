# MQ-2 Gas / Smoke Sensor

**Used in:** BSH-FireAlarm node (new addition — part of the flame/smoke/gas
Fire Alarm upgrade; replaces the earlier placeholder MQ-135 assignment —
see "Why MQ-2, not MQ-135" below and the Sep 2026 Lessons Learned entry in
`Piranga_Workflow_v2.md`)

## Function
Detects combustible gas and smoke concentration via analog output — the
slower, indirect complement to the flame sensor's fast, direct IR detection
(see `15_Flame_Sensor.md`). Together they let the dashboard distinguish
"smoke/gas building up" from "an actual flame is present," a richer signal
than either sensor alone.

## Why MQ-2, not MQ-135
This slot was originally flagged as a maybe-reuse for the MQ-135 already
documented in this folder's history (see the retirement note in the old
`10_MQ135_Air_Quality_Sensor.md`, now removed from the Project). The MQ-135
is tuned for general air-quality gases (NH3, NOx, alcohol, benzene, CO2)
and only picks up smoke as a side effect. The `bugsNburgers/IOT-Fire-Alarm-
System` reference the team chose to adapt from uses an MQ-2 instead, which
is purpose-built for combustible gas/LPG/smoke — the better-matched sensor
for a fire alarm specifically. The MQ-135 doc was retired rather than
repurposed, since the two chips aren't interchangeable.

## Key Specs
- MQ-2 analog gas/smoke sensor module
- Onboard heater coil requires **5V** to operate correctly — do not run it
  on 3.3V
- Needs a warm-up period after power-on (commonly 20–60 seconds, some
  datasheets recommend longer) before readings stabilize — do not
  calibrate immediately at boot
- Analog output (AOUT) only is used here; most breakout boards also expose
  a digital threshold output (DOUT) with an onboard trim-pot, but tuning
  the threshold in firmware (see Calibration below) is more flexible than
  a screwdriver adjustment

## ⚠️ Hardware Caveat — verify AOUT voltage before wiring to the ESP32
**Not called out in the `bugsNburgers` reference, and easy to miss:** the
ESP32's ADC pins are only 3.3V-tolerant, but many MQ-2 breakout boards are
powered at 5V and their AOUT can swing close to that 5V rail depending on
the specific board's onboard op-amp/comparator circuit. Wiring a
5V-swinging AOUT directly into an ESP32 GPIO risks damaging the pin.

**Before wiring, measure AOUT with a multimeter** (sensor powered, sitting
in clean air) and confirm it stays under ~3.3V for your specific board. If
it doesn't, add the same style of voltage divider already used for the
HC-SR04's ECHO pin (see `14_HC-SR04_Ultrasonic_Sensor.md`): a 1kΩ resistor
from AOUT to the GPIO, a 2kΩ resistor from that same GPIO to GND — giving
AOUT × 2000/(1000+2000) ≈ AOUT × 0.667.

## Wiring / Pins (BSH-FireAlarm ESP32)
| Signal | GPIO | Notes |
|---|---|---|
| AOUT | GPIO 34 | ADC1, input-only pin — safe alongside active WiFi (unlike ADC2 pins). **Add a voltage divider first if your board's AOUT exceeds 3.3V — see caveat above.** |
| VCC | 5V | Heater coil needs 5V |
| GND | Common ground | Shared with the divider's ground leg, if used |

## Calibration & Threshold Logic
Adapted from the `bugsNburgers` reference's approach, with one deliberate
improvement. Their firmware samples the sensor 20 times in clean air at
boot to compute a baseline average, then compares live readings against a
**fixed** magic-number threshold (`GAS_THRESHOLD = 1500` in their code,
though their own comments reference a different number — a sign of exactly
the fragility a fixed threshold produces on hardware it wasn't tuned on).
This project instead computes the alert threshold **relative to the
boot-time baseline** (`gasBaseline + GAS_ALERT_MARGIN`), so the same
firmware tolerates some baseline drift between individual sensor units,
rooms, or humidity conditions without needing a new magic number each
time. `GAS_ALERT_MARGIN` still needs real bench calibration once the
sensor is on hardware — treat the value in the firmware as a starting
point, not a final answer.

Known limitation carried over from the reference: baseline is only
computed once, at boot, in whatever air quality happens to be present at
that moment. If the room is already slightly smoky at boot, the baseline
(and therefore the threshold) will be too high. Worth an explicit
"clean air, then power on" step in the demo setup checklist.

## Modbus Register Tie-In
- Input Register `GAS_RAW_VALUE` (IR0) — raw ADC reading (0–4095), so the
  dashboard can show a live gas/smoke level rather than just a flag
- Discrete Input `GAS_SMOKE_ALERT` (DI2) — set locally once `GAS_RAW_VALUE`
  crosses `gasBaseline + GAS_ALERT_MARGIN`; see `BSH_FireAlarm_Node.ino`
  for how this feeds into the combined `alarmActive` logic

## Notes
This sensor doubles as good material for the project's "false sensor data
injection" cyber-layer demo (see the Cyber Layer section of the project
description): because the raw value is exposed as its own Modbus point, a
compromised or attacker-controlled node could report a fabricated
`GAS_RAW_VALUE` with no matching physical gas source — a variant of the
same false-alarm-injection idea already used for the pull-station/coil
contrast in Scenario 4, but distinguishable from a flame-triggered alarm.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-gas-sensor` (an MQ-2 model)

**How to use it:** VCC, GND, **`AOUT`**→GPIO 34 (`DOUT` left unused, as on the bench). Note the Wokwi pin is named `AOUT`, not `AO`: a connection to a wrong pin name is silently dropped, leaving GPIO 34 floating, and a floating ADC pin reads random values that cause false alarms (found while building the FireAlarm simulation). Click the sensor while running to open its gas-level slider (log scale, 0.1–100,000 ppm) and raise the level and watch `gasRaw` climb past `baseline + GAS_ALERT_MARGIN`.

**What the simulation hides:** No heater warm-up (the real one needs 20–30 s minimum, longer for a stable baseline), no humidity or temperature drift, and AO voltage isn't a safety issue in simulation. Real AO can swing toward 5V, so measure it before wiring to GPIO 34. The BSH bench currently uses an **MQ-9**, so real thresholds will differ from the simulation.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
