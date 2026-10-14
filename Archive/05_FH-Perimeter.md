# FH-Perimeter Node

**IP Address:** 192.168.1.25
**Field layer:** Digital (PIR motion) + analog-timed (HC-SR04 ultrasonic) ↔ ESP32
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-FH-Perimeter` — its own Docker container at 192.168.1.125 (macvlan, running on the hub host 192.168.1.10), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.1.25:502

Mirrors `BSH-Perimeter` exactly — same components, same protocol choices.
Replaces `FH-AirQuality` as the fifth FH system (same IP, ports, and
OpenPLC slot).

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | MOTION_DETECTED | Raw PIR output, reported honestly |
| Discrete Input | DI1 | OBJECT_IN_RANGE | Ultrasonic distance below a fixed threshold |
| Input Register | IR0 | DISTANCE_CM | Raw measured distance (sentinel value when no echo is received) |
| Coil | Coil0 | ZONE_DISABLE | Master→slave, level — not trusted on comm loss (fails toward "enabled"); see `BSH_Systems/05_BSH-Perimeter.md` "Notes" |

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| PIR motion sensor (GPIO 32) | `wokwi-pir-motion-sensor` | Native |
| HC-SR04 (TRIG 14, ECHO 27) | `wokwi-hc-sr04` | Native — **no divider in sim; the bench still needs the 1kΩ/2kΩ divider** |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-Perimeter/Wokwi/`) as-is. When FH-Perimeter becomes the active system, copy that folder to `Firmware/FH-Perimeter/Wokwi/` and log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
