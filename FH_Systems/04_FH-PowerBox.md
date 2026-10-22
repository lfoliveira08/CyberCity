# FH-PowerBox Node

**IP Address:** 192.168.50.24
**Field layer:** Plain relay (lights + fan) + local override push buttons, 2-channel relay module
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-FH-PowerBox` — its own Docker container at 192.168.50.124 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.24:502
**PLC program:** `PLC_Logic/FH-PowerBox/` (ladder diagram, register map, Structured Text program, build log)

Mirrors `BSH-PowerBox` exactly — same components, same GPIO map, same protocol choices.

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | LIGHTS_ON_CMD | Lights on/off (master-written command) |
| Coil | Coil1 | FAN_CMD | Fan on/off (master-written command) |
| Discrete Input | DI0 | LIGHTS_STATUS | Lights' actual physical state (honest feedback — may disagree with the last command if a local button moved it since) |
| Discrete Input | DI1 | FAN_STATUS | Fan's actual physical state (same idea) |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

## Notes
**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 2.**

**Local override buttons (Sep 29, 2026):** mirrors the BSH-PowerBox change — added a real physical push-button switch per channel alongside the existing remote Modbus command. See `BSH_Systems/04_BSH-PowerBox.md` Notes and the Sep 29, 2026 entry in `Piranga_Workflow_v2.md` Section 14 for the full authority-rule rationale. Also closes the previous open-loop gap — `LIGHTS_STATUS`/`FAN_STATUS` (DI0/DI1) now report actual physical state honestly.

**Authority rule changed (Oct 21, 2026, Phase C):** the same change as BSH-PowerBox (a coil write counts as a hub command only when its value differs from the last hub value; forgotten after a WiFi rejoin), made in `FH_PowerBox_Node.ino` ahead of FH Phase C because OpenPLC rewrites the coils every poll. Luis reported flashing it; it is NOT bench-tested on FH. The old sketch is `Archive/FH_PowerBox_Node_PhaseB_every-write-wins.ino`. See `BSH_Systems/04_BSH-PowerBox.md` for the full reason.

**Design change (Sep 28, 2026):** mirrors the BSH-PowerBox change — DALI dropped in favor of a plain on/off relay for lights, and the lights relay's GPIO corrected from 12 (a strapping pin) to 13. **Update (Sep 29, 2026):** that GPIO move turned out not to be what actually caused the inverted-lights symptom seen in BSH-PowerBox's Phase A0 test — the real cause was Wokwi's simulated relay module energizing on HIGH rather than LOW, on both channels. The GPIO 12→13 move itself is still worth keeping (a real hazard for any pulled-up peripheral), just not the explanation for that particular bug. See `BSH_Systems/04_BSH-PowerBox.md` Notes and the Sep 28/29, 2026 entries in `Piranga_Workflow_v2.md` Section 14.

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 13 | Lights relay |
| GPIO 15 | Fan relay |
| GPIO 4 | Lights local override button |
| GPIO 16 | Fan local override button |

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Lights relay (GPIO 13) | `wokwi-relay-module` + `wokwi-led` as the "light" | Native |
| Fan relay (GPIO 15) | `wokwi-relay-module` + `wokwi-led` as the "fan" | Native |
| Lights override button (GPIO 4) | `wokwi-pushbutton` | Native |
| Fan override button (GPIO 16) | `wokwi-pushbutton` | Native |

FH mirrors BSH pin-for-pin, so FH uses the BSH simulation (`Firmware/BSH-PowerBox/Wokwi/`) as-is. When FH-PowerBox becomes the active system, copy that folder to `Firmware/FH-PowerBox/Wokwi/` and log the run in FH's own Build_Logs.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `FH_Sensors/`.
