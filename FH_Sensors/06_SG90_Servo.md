# SG90 Micro Servo

**Used in:** FH-Door node (Founder's Hall mirror of BSH-Door)

## Function
Physically opens/closes the door or gate arm.

## Key Specs
- SG90 micro servo
- ~500mA stall current
- **Requires an external 5V supply** — do not power from the ESP32's 3.3V pin

## Wiring / Pins (FH-Door ESP32)
| Signal | Connection |
|---|---|
| Signal | GPIO 13 |
| VCC | External 5V PSU |
| GND | Shared ground with ESP32 |

## Modbus Register Tie-In
Driven by Coils `DOOR_OPEN_CMD` / `DOOR_CLOSE_CMD`.

## Notes
Same brownout warning that applied on IRONCLAD: powering the servo from the ESP32's own 3.3V regulator can brown out the whole board when the servo stalls or starts moving. Always use a separate 5V supply with a shared ground.

This doc mirrors `BSH_Sensors/06_SG90_Servo.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-servo`

**How to use it:** PWM→GPIO 13, V+ and GND. Uses the same `ESP32Servo` library as the real firmware (add it to `libraries.txt`). You can watch the horn move to each commanded angle.

**What the simulation hides:** The real SG90 needs an external 5V supply with a shared ground; powering it from the ESP32 can cause brownout resets. The simulation draws no current.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
