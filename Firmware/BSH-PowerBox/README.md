# BSH-PowerBox — Build Folder

Physical build documentation for the BSH-PowerBox node, one subfolder per
component — same convention as `Firmware/BSH-FireAlarm/`.

Companion to, not a replacement for:
- **`BSH_PowerBox_Node/BSH_PowerBox_Node.ino`** — the firmware to flash. Now at
  `BSH_PowerBox_Node/` (Phase B, 192.168.50.14). Phase B bench-confirmed Oct 6, 2026.
- **`BSH_Systems/04_BSH-PowerBox.md`** — full system doc (IP,
  register map, role).
- **`BSH_Sensors/09_Relay_Module.md`** — general component reference doc.

## Design change (Sep 28, 2026)

This node originally used DALI for dimmable lighting plus a plain relay for
the fan. DALI was dropped — lights now use the second channel of the same
relay module as the fan, plain on/off. See `BSH_Systems/04_BSH-PowerBox.md`
Notes and `Piranga_Workflow_v2.md` Section 14. This node is no longer
flagged higher-risk than Fire Alarm — it's now the same relay-only pattern
already proven on Fire Alarm and Elevator.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `Lights_Relay/` | 2nd channel of the relay module (lights) — formerly `DALI_LED_Driver/` | Wiring notes + build/test log |
| `Fan_Relay/` | 1st channel of the relay module (fan) | Wiring notes + build/test log |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) |

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real hardware
1. `Fan_Relay/` — plain digital relay, confirm the ESP32/relay/fan chain works
2. `Lights_Relay/` — same pattern, second relay channel
3. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (Phase B),
   once both outputs work standalone
4. OpenPLC + SCADA integration (Phase C) — follow the same pattern as
   `Firmware/BSH_FireAlarm_PILOT_Bringup.md` Phase C, substituting this
   node's IP (192.168.50.14) and register map

Once BSH-PowerBox is built and tested, all four BSH systems are done and the same pattern moves on to Founder's Hall (Phase 2), per the project's chosen build order (Fire Alarm -> Elevator -> Door -> Power Box).
