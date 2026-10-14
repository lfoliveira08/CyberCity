# Push Button

**Used in:**
- FH-FireAlarm node (1x — manual pull-station button)
- ~~FH-Elevator node (3x — floor call buttons)~~ — retired Sep 26, 2026 when FH
  slot 1 became an HVAC zone unit. The three floor-button rows below are kept for
  reference; BSH-Elevator still uses them (`BSH_Sensors/02_Push_Button.md`).
- FH-PowerBox node (2x — local lights/fan override switches, added Sep 29, 2026)

(Founder's Hall mirror of BSH's push-button usage — identical hardware, wiring, and logic.)

## Function
Momentary digital input for user-triggered events: calling an elevator floor, manually triggering the fire alarm (as opposed to a hub/attacker-triggered alarm), or locally overriding a relay-driven output like a real light switch.

## Key Specs
- Standard momentary push button
- Wired `INPUT_PULLUP`, active LOW
- ~50ms software debounce recommended

## Wiring / Pins
| Node | Button | GPIO |
|---|---|---|
| FH-FireAlarm | Manual pull station | GPIO 4 |
| ~~FH-Elevator~~ (retired) | Floor 1 | GPIO 4 |
| ~~FH-Elevator~~ (retired) | Floor 2 | GPIO 16 |
| ~~FH-Elevator~~ (retired) | Floor 3 | GPIO 17 |
| FH-PowerBox | Lights override | GPIO 4 |
| FH-PowerBox | Fan override | GPIO 16 |

(Pin numbers repeat across boards — these are separate physical ESP32s, so there's no conflict.)

## Modbus Register Tie-In
- ~~Elevator buttons: drive local floor-call logic feeding `ELEVATOR_MOVING` and `CURRENT_FLOOR`~~ (retired on FH; still true on BSH)
- Fire alarm button: sets Discrete Input `MANUAL_PULL_TRIGGERED` on the FH-FireAlarm node
- Power Box buttons: each directly toggles its relay (no dedicated "button pressed" register of its own); the resulting physical state is what gets reported, honestly, via Discrete Input `LIGHTS_STATUS`/`FAN_STATUS` — see `FH_Systems/04_FH-PowerBox.md`

## Notes
The fire alarm pull station is a good source for the "authorized/expected" event in your demo scenarios — a physical button press vs. the hub issuing `ALARM_ACTIVATE` remotely gives you two distinguishable trigger sources, which is exactly what Scenario 4 (false fire alarm injection) needs to contrast against.

**Power Box's use (Sep 29, 2026)** mirrors BSH-PowerBox — see `BSH_Sensors/02_Push_Button.md` Notes for the full local-vs-remote authority rationale.

This doc mirrors `BSH_Sensors/02_Push_Button.md` exactly — same components, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-pushbutton`

**How to use it:** Click to press; Ctrl-click (Cmd-click on Mac) to hold it down. Leave the default `bounce` on so the firmware's debounce logic actually gets tested. Wire one side to the GPIO and the other to GND; add a 10kΩ `wokwi-resistor` pull-up to 3V3 if the firmware uses plain `INPUT` (a bare button floats otherwise).

**What the simulation hides:** Real Keyestudio-style modules carry their own resistor — check which kind you have before copying the simulated pull-up onto the bench.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
