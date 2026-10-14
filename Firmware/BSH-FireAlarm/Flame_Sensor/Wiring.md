
# Flame Sensor — BSH-FireAlarm

Reference doc: `BSH_Sensors/15_Flame_Sensor.md`

## What it does here
Fast, direct detection of an open flame's IR signature — one of three local
trigger sources (alongside the pull-station button and the MQ-2 gas/smoke
sensor) that get OR'd together into the node's `alarmActive` logic, plus the
remote `ALARM_ACTIVATE` coil.

## Key specs
- 3-pin IR flame sensor module (LM393-comparator breakout), digital output
- **Active-LOW**: reads LOW when flame/strong IR is detected, HIGH otherwise
- Onboard comparator drives the pin push-pull — no external pull-up/down
  resistor needed
- Rated detection range/angle (~60–100cm at ~60°) is optimistic for cheap
  modules — bench-test the real distance against the printed building's
  actual scale before the demo, and test under the actual demo lighting
  (strong sunlight/bright IR sources can false-trigger it)

## Wiring
| Sensor pin | ESP32 pin |
|---|---|
| D0 (digital out) | GPIO 18 |
| VCC | 3.3V or 5V — check your specific module's rating |
| GND | GND |

Plain `INPUT`, no pull-up needed — the module drives both states.

## Firmware tie-in
`BSH_FireAlarm_Node.ino`: `PIN_FLAME = 18`, read with
`digitalRead(PIN_FLAME) == LOW` into `flameDetected`, published to Modbus
Discrete Input `FLAME_DETECTED` (DI1), and OR'd into `alarmActive` alongside
the pull-station button, the MQ-2 gas/smoke alert, and the remote coil.

## Bench-test checklist (Phase A, this component only)
- [ ] Sensor wired per the table above, ESP32 not yet powered
- [ ] Power up the ESP32 with the real firmware (or a throwaway test sketch
      that just prints `digitalRead(18)`) — Serial Monitor / LED should show
      "no flame" (HIGH) at rest
- [ ] Hold a lighter or lit match a foot or so away (briefly, and don't hold
      it near the module itself) — sensor output should go LOW, and with the
      real firmware the buzzer/LED should react exactly like pressing the
      pull-station button
- [ ] Test from the distance/angle the sensor will actually sit at once
      mounted in the printed building, not just on the open bench
- [ ] Confirms: correct pin, correct polarity, sensor itself responds to a
      real flame (not just a lighter's IR — a lighter's flame is a fine
      stand-in, but note in Lessons Learned if it behaves differently than
      expected)

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "always reads flame detected" -> check the
module's onboard sensitivity trim-pot, it may be turned up too high; "never
triggers" -> confirm VCC voltage matches the module's rating)*
