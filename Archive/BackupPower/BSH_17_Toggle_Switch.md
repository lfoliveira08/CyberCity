# SPST Toggle Switch (Utility-Power-Loss Stand-In)

**Used in:** BSH-BackupPower node (models the utility-power-loss signal)

## Function
Stands in for the dry-contact signal a real power-monitoring relay or ATS
auxiliary contact would provide when utility power is lost. Flipped
manually during bench testing/demo to simulate a real outage — same
"physical switch models a real-world condition" pattern already used
elsewhere in the project (pull stations, reed switches).

## Key Specs
- Standard SPST panel-mount toggle switch
- Uses the ESP32's internal pull-up (`INPUT_PULLUP`) — reads LOW when
  toggled to "power lost," same active-LOW convention as the project's
  push buttons

## Wiring / Pins (BSH-BackupPower ESP32)
| Signal | GPIO |
|---|---|
| Switch terminal 1 | GPIO 26 |
| Switch terminal 2 | GND |

No external pull-up needed — GPIO 26 is not one of the pull-resistor-less
input-only pins (34–39).

## Modbus Register Tie-In
Feeds Discrete Input `UTILITY_POWER_LOST` directly — reported raw and
honestly, no local judgment.

## Notes
A real deployment would wire this input to an actual power-monitoring
relay's dry contact instead of a manual switch. The switch is a bench/demo
stand-in only.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-slide-switch`

**How to use it:** One terminal to GPIO 26, the other to GND. Toggle it during simulation to flip `UTILITY_POWER_LOST`.

**What the simulation hides:** Nothing significant — a slide switch behaves the same in Wokwi as a real toggle switch does on the bench.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
