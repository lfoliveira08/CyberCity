# FH-FireAlarm Node

**IP Address:** 192.168.50.22
**Field layer:** GPIO (buzzer, RGB LED, manual pull-station button, flame sensor, MQ-2 gas/smoke sensor)
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-FH-FireAlarm` — its own Docker container at 192.168.50.122 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.22:502
**PLC program:** `PLC_Logic/FH-FireAlarm/` (ladder diagram, register map, Structured Text program, build log)

Mirrors `BSH-FireAlarm` exactly — same components, same GPIO map, same protocol choices, same flame/smoke/gas upgrade (see `BSH_Systems/02_BSH-FireAlarm.md` for the full upgrade rationale).

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | MANUAL_PULL_TRIGGERED | Local pull station activated |
| Discrete Input | DI1 | FLAME_DETECTED | Flame sensor tripped |
| Discrete Input | DI2 | GAS_SMOKE_ALERT | MQ-2 reading exceeded baseline + margin |
| Input Register | IR0 | GAS_RAW_VALUE | Raw MQ-2 ADC reading (0–4095), for dashboard display |
| Coil | Coil0 | ALARM_ACTIVATE | Remote/hub-triggered alarm |
| Input Register | IR1 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR2 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 3.**

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Active buzzer (GPIO 23) | `wokwi-buzzer` | Native — sim uses `tone()`, real firmware uses `digitalWrite` |
| RGB LED (GPIO 25/26/27) | `wokwi-rgb-led` (`common: cathode`) + 3× 220Ω | Native |
| Pull-station button (GPIO 4) | `wokwi-pushbutton` + 10kΩ pull-up | Native |
| Flame sensor (GPIO 18) | `wokwi-slide-switch` (left = flame) | **Stand-in** (no flame sensor part) |
| MQ-2 gas/smoke (GPIO 34) | `wokwi-gas-sensor` | Native |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-FireAlarm/Wokwi/`) as-is. When FH-FireAlarm becomes the active system, copy that folder to `Firmware/FH-FireAlarm/Wokwi/` and log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
