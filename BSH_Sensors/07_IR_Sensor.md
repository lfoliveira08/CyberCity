# IR Sensor (Obstacle/Approach Detection)

**Used in:** BSH-Door node

## Function
Detects approach or presence at the door — triggers the gate to open for a legitimate approach, or serves as an intrusion trigger in attack scenarios.

## Key Specs
- IR obstacle/line-tracking sensor module
- Digital output, onboard potentiometer for sensitivity
- 3.3V powered

## Wiring / Pins (BSH-Door ESP32)
| Signal | GPIO |
|---|---|
| Signal (digital out) | GPIO 14 |
| VCC | 3.3V |
| GND | Common ground |

## Modbus Register Tie-In
Feeds Discrete Input `IR_DETECTED`.

## Notes
Tune the onboard sensitivity potentiometer so it only triggers at the intended approach distance — the same tuning step you used for the IRONCLAD garage/track IR sensors.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — Wokwi has no IR obstacle sensor (`wokwi-ir-receiver` is for TV remotes, not obstacle detection)

**How to use it:** Use a `wokwi-slide-switch`: common pin `2` → GPIO 14, pin `1` → GND, pin `3` → 3V3. Slide left = LOW = "object detected" (active LOW, like the real module).

**What the simulation hides:** The real module's sensitivity potentiometer, sunlight interference, and detection range can't be simulated.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
