# Push Button

**Used in:**
- BSH-Elevator node (3x — floor call buttons)
- BSH-FireAlarm node (1x — manual pull-station button)
- BSH-PowerBox node (2x — local lights/fan override switches, added Sep 29, 2026)

## Function
Momentary digital input for user-triggered events: calling an elevator floor, manually triggering the fire alarm (as opposed to a hub/attacker-triggered alarm), or locally overriding a relay-driven output like a real light switch.

## Key Specs
- Standard momentary push button
- Wired `INPUT_PULLUP`, active LOW
- ~50ms software debounce recommended

## Wiring / Pins
| Node | Button | GPIO |
|---|---|---|
| BSH-Elevator | Floor 1 | GPIO 4 |
| BSH-Elevator | Floor 2 | GPIO 16 |
| BSH-Elevator | Floor 3 | GPIO 17 |
| BSH-FireAlarm | Manual pull station | GPIO 4 |
| BSH-PowerBox | Lights override | GPIO 4 |
| BSH-PowerBox | Fan override | GPIO 16 |

(Pin numbers repeat across boards — these are separate physical ESP32s, so there's no conflict.)

## Modbus Register Tie-In
- Elevator buttons: drive local floor-call logic feeding `ELEVATOR_MOVING` and `CURRENT_FLOOR`
- Fire alarm button: sets Discrete Input `MANUAL_PULL_TRIGGERED` on the BSH-FireAlarm node
- Power Box buttons: each directly toggles its relay (no dedicated "button pressed" register of its own); the resulting physical state is what gets reported, honestly, via Discrete Input `LIGHTS_STATUS`/`FAN_STATUS` — see `BSH_Systems/04_BSH-PowerBox.md`

## Notes
The fire alarm pull station is a good source for the "authorized/expected" event in your demo scenarios — a physical button press vs. the hub issuing `ALARM_ACTIVATE` remotely gives you two distinguishable trigger sources, which is exactly what Scenario 3 (false data injection) needs to contrast against.

**Power Box's use (Sep 29, 2026)** is the same local-vs-remote idea applied to an actuator instead of an alarm: a button press changes the physical state immediately and directly, and the hub only regains authority the next time it issues a *fresh* command (not merely by holding its last one) — see `BSH_PowerBox_Node.ino`'s header comment and the Sep 29, 2026 entry in `Piranga_Workflow_v2.md` Section 14 for the full authority rule. This gives Power Box its own local-vs-remote mismatch scenario: SCADA's last known command can honestly disagree with `LIGHTS_STATUS`/`FAN_STATUS` if someone has since flipped the physical switch.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-pushbutton`

**How to use it:** Click to press; Ctrl-click (Cmd-click on Mac) to hold it down. Leave the default `bounce` on so the firmware's debounce logic actually gets tested. Wire one side to the GPIO and the other to GND; add a 10kΩ `wokwi-resistor` pull-up to 3V3 if the firmware uses plain `INPUT` (a bare button floats otherwise).

**What the simulation hides:** Real Keyestudio-style modules carry their own resistor — check which kind you have before copying the simulated pull-up onto the bench.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
