# Magnetic Reed Switch

**Used in:** BSH-Door node

## Function
Confirms the door/gate's actual physical open-or-closed state, independent of what the servo was told to do — this is what lets the dashboard tell "commanded open" apart from "actually open."

## Key Specs
- Normally-open magnetic reed switch
- Digital input, `INPUT_PULLUP`
- Magnet mounted on the moving door/gate arm; switch body mounted on the fixed frame

## Wiring / Pins (BSH-Door ESP32)
| Signal | GPIO |
|---|---|
| Signal | GPIO 34 (input-only ADC pin — fine for a digital read) |
| GND | Common ground |

## Modbus Register Tie-In
Feeds Discrete Input `DOOR_POSITION`.

## Notes
This is a key cybersecurity-relevant sensor: if the hub sends `DOOR_CLOSE_CMD` but the reed switch keeps reporting open, that mismatch is a clean, demonstrable anomaly for your intrusion/compromise scenarios — the system "knows" something doesn't match its own command.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — Wokwi has no reed switch

**How to use it:** Use a `wokwi-slide-switch` with the same external 10kΩ pull-up to 3V3 on GPIO 34 that the real build needs, so the simulated wiring matches the bench exactly.

**What the simulation hides:** Do not rely on the simulation to prove the pull-up is required: GPIO 34–39's missing internal pull-ups may not be modeled. Build the external resistor on the bench regardless.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
