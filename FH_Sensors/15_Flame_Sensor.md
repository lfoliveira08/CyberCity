# Flame Sensor (IR Flame Detector)

**Used in:** FH-FireAlarm node (new addition — part of the flame/smoke/gas
Fire Alarm upgrade; see `FH_Systems/02_FH-FireAlarm.md` and Section 14 of
`Piranga_Workflow_v2.md` for the decision log)

## Function
Detects the IR light signature of an open flame directly — a fast, direct
signal that complements the MQ-2 gas/smoke sensor's slower, indirect one
(see `16_MQ-2_Gas_Smoke_Sensor.md`). Together they let the dashboard show
"gas/smoke building up" and "an actual flame is present" as distinguishable
states instead of one generic "fire" flag.

## Key Specs
- 3-pin IR flame sensor module (LM393-comparator breakout), digital output
- Digital OUT is **active-LOW**: reads LOW when flame/strong IR is detected,
  HIGH otherwise
- Onboard comparator drives the digital pin push-pull — no external
  pull-up/pull-down resistor is needed
- Typical cheap-module detection range/angle is rated around 60–100cm at a
  ~60° cone, but that's optimistic — bench-test the real detection distance
  against the printed building's actual scale before the demo

## Wiring / Pins (FH-FireAlarm ESP32)
| Signal | GPIO | Notes |
|---|---|---|
| D0 (digital out) | GPIO 18 | plain `INPUT`, no pull-up needed — module drives both states |
| VCC | 3.3V or 5V (check your specific module's rating) | |
| GND | Common ground | |

## Modbus Register Tie-In
- Discrete Input `FLAME_DETECTED` (DI1) — true when the sensor reads LOW,
  reported honestly; see `FH_FireAlarm_Node.ino` for how it feeds into the
  node's combined `alarmActive` logic alongside the pull-station button, the
  MQ-2 gas/smoke alert, and the remote `ALARM_ACTIVATE` coil

## Notes
Pin choice and active-LOW polarity are carried over unchanged from the
`bugsNburgers/IOT-Fire-Alarm-System` reference the team adapted this from —
wiring doesn't care what's polling the ESP32. What did change: that
reference reported flame state to a Blynk cloud app over WiFi; here it's a
plain Modbus Discrete Input polled directly by `OpenPLC-FH-FireAlarm`,
consistent with every other node in this project. Cheap flame sensor
modules can false-trigger on strong sunlight or other bright IR sources —
worth testing under the actual demo lighting, not just at a desk indoors.

This doc mirrors `BSH_Sensors/15_Flame_Sensor.md` exactly — same component,
same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — Wokwi has no flame sensor

**How to use it:** Use a `wokwi-slide-switch`: common pin `2` → GPIO 18, pin `1` → GND, pin `3` → 3V3. Slide left = LOW = "flame detected" (same active-LOW logic as the real module's D0).

**What the simulation hides:** The real module's sensitivity potentiometer and its response to lighters, sunlight, and IR remotes can only be tested with a real flame source (carefully).

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
