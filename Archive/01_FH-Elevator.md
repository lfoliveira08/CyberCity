# FH-Elevator Node

**IP Address:** 192.168.1.21
**Field layer:** GPIO/I2C (motor driver, floor buttons, LCD) — no dedicated field-bus protocol
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-FH-Elevator` — its own Docker container at 192.168.1.121 (macvlan, running on the hub host 192.168.1.10), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.1.21:502

Mirrors `BSH-Elevator` exactly — same components, same GPIO map, same protocol choices.

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | ELEVATOR_MOVING | Motor currently running |
| Discrete Input | DI1 | ELEVATOR_FAULT | Fault/stall condition |
| Coil | Coil0 | CALL_FLOOR_1 | Remote call to floor 1 |
| Coil | Coil1 | CALL_FLOOR_2 | Remote call to floor 2 |
| Coil | Coil2 | CALL_FLOOR_3 | Remote call to floor 3 |
| Coil | Coil3 | ELEVATOR_DISABLE | Lock out the elevator |
| Input Register | IR0 | CURRENT_FLOOR | Reported current floor (1–3) |

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Push buttons ×3 (GPIO 4/16/17) | `wokwi-pushbutton` ×3 | Native |
| I2C 16×2 LCD (GPIO 21/22) | `wokwi-lcd1602`, `"pins": "i2c"` | Native |
| L293D + DC motor (GPIO 18/19/5) | 3× `wokwi-led` — ENABLE brightness = speed, DIRA/DIRB = direction | **Stand-in** (no L293D/DC motor part) |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-Elevator/Wokwi/`) as-is. When FH-Elevator becomes the active system, copy that folder to `Firmware/FH-Elevator/Wokwi/` and log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
