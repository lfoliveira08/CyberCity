# FH-BackupPower Node

**IP Address:** 192.168.1.25
**Field layer:** Digital (utility-loss switch, ATS relay) + analog (battery voltage divider, ACS712 current sensor) ↔ ESP32
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-FH-BackupPower` — its own Docker container at 192.168.1.125 (macvlan, running on the hub host 192.168.1.10), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.1.25:502

Mirrors `BSH-BackupPower` exactly — same components, same GPIO map, same
protocol choices. Replaces `FH-Perimeter` as the fifth FH system (same IP,
ports, and OpenPLC slot — see Section 14 of `Piranga_Workflow_v2.md`,
Sep 28, 2026).

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | UTILITY_POWER_LOST | Raw switch input, reported honestly |
| Discrete Input | DI1 | LOW_BATTERY | `UPS_BATTERY_PCT` below a fixed threshold |
| Input Register | IR0 | UPS_BATTERY_PCT | Scaled ADC reading — always reports the real value, regardless of `LOW_BATTERY` |
| Input Register | IR1 | LOAD_CURRENT_MA | Scaled ACS712 reading |
| Coil | Coil0 | ATS_TRANSFER_CMD | Master→slave; open-loop, no feedback wired yet — see `BSH_Systems/05_BSH-BackupPower.md` "Notes" |

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Toggle switch (GPIO 26) | `wokwi-slide-switch` | Native |
| Relay module (GPIO 27) | `wokwi-relay-module` + `wokwi-led` | Native |
| Voltage divider / battery sense (GPIO 34) | `wokwi-potentiometer` | **Stand-in** (no battery/divider part) |
| ACS712 current sensor (GPIO 35) | `wokwi-potentiometer` | **Stand-in** (no ACS712 part in Wokwi) |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-BackupPower/Wokwi/`) as-is. When FH-BackupPower becomes the active system, copy that folder to `Firmware/FH-BackupPower/Wokwi/` and log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
