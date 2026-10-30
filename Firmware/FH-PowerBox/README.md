# FH-PowerBox — Build Folder

**Redesigned Oct 28, 2026, mirroring `Firmware/BSH-PowerBox/README.md`.** This node no longer drives lights and a fan — it switches power to the other three FH systems (FH-HVAC, FH-FireAlarm, FH-Door). **Not yet bench-confirmed on FH's own hardware.** The old design is archived: see `Archive/README.md` for the retired sketch and the `Fan_Relay/`/`Lights_Relay/` folders.

Companion to, not a replacement for:
- **`FH_PowerBox_Node/FH_PowerBox_Node.ino`** — the firmware to flash (power rails, WiFi values are placeholders — fill in your own copy, never commit real values).
- **`FH_Systems/04_FH-PowerBox.md`** — full system doc (IP, register map, role, decisions).
- **`FH_Sensors/09_Relay_Module.md`** — general component reference doc.
- **`Schematic/Connections.md`** — rewritten Oct 28, 2026 for the power rails, assumed to mirror `Firmware/BSH-PowerBox/Schematic/Connections.md`'s pattern. **Draft quality — not individually confirmed against this node's own hardware**, open-questions list at the bottom of the file.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the power-rails design (Phase A0) | **not yet created** — copy from `Firmware/BSH-PowerBox/Wokwi/`, same mirroring convention as every other FH system (`Firmware/WORKFLOW.md` Section 7) |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification — old entries describe the lights-and-fan design; nothing logged yet for the power rails |
| ~~`Lights_Relay/`~~ | retired, archived | lights channel, dropped Oct 28, 2026 |
| ~~`Fan_Relay/`~~ | retired, archived | fan channel, dropped Oct 28, 2026 |

## Suggested build order for this node

0. `Wokwi/` — create it (copy from BSH-PowerBox) and run the simulation first (Phase A0, `Firmware/WORKFLOW.md` Section 7)
1. Wire and bench-test each of the three rail relays individually, confirming polarity on each board
2. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification
3. OpenPLC + SCADA integration (Phase C) — ladder to be built by Luis in the Editor, same pattern as BSH-PowerBox

See `Firmware/FH_PowerBox_Bringup_Checklist.md` for the full checklist — nothing on it is checked yet.
