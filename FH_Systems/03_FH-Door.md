# FH-Door Node (Gate)

**IP Address:** 192.168.50.23
**Field layer:** SPI (RFID reader) + plain GPIO (IR sensor, reed switch)
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-FH-Door` — its own Docker container at 192.168.50.123 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.23:502
**PLC program:** `PLC_Logic/FH-Door/` (ladder diagram, register map, Structured Text program, build log)

Mirrors `BSH-Door` exactly — same components, same GPIO map, same protocol choices.

> **Field layer updated Sep 2026:** the card reader was swapped from
> Wiegand to an MFRC522/RC522 RFID reader (SPI) — same role, same
> `ACCESS_GRANTED` register semantics. See Section 14, Lessons Learned &
> Decisions Log, of `Piranga_Workflow_v2.md` for the swap rationale.

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | IR_DETECTED | Approach/presence detected |
| Discrete Input | DI1 | DOOR_POSITION | Actual open(1)/closed(0) state |
| Discrete Input | DI2 | ACCESS_GRANTED | Valid card read accepted locally |
| Coil | Coil0 | DOOR_OPEN_CMD | Command to open (remote override) |
| Coil | Coil1 | DOOR_CLOSE_CMD | Command to close |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 2.**

Same authorized-vs-injected-access contrast applies here as documented for BSH-Door — comparing the two buildings' Door nodes side by side (same function, same protocol, independent hardware) is a good resilience-scenario source: does an attack on one building's door affect the other's?

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| SG90 servo (GPIO 13) | `wokwi-servo` | Native |
| RGB LED (GPIO 25/26/27) | `wokwi-rgb-led` (`common: cathode`) + 3× 220Ω | Native |
| IR obstacle sensor (GPIO 14) | `wokwi-slide-switch` (left = object) | **Stand-in** |
| Reed switch (GPIO 34) + 10kΩ pull-up | `wokwi-slide-switch` + 10kΩ pull-up | **Stand-in** |
| RC522 RFID (SPI 18/19/23, SS 32, RST 33) | `board-mfrc522` | Native — 6 preset cards |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-Door/Wokwi/`) as-is. That folder was copied to `Firmware/FH-Door/Wokwi/` on Oct 9, 2026 (same simulation as BSH's, which ran fine); log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
