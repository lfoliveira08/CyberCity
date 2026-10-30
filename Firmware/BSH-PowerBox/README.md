# BSH-PowerBox — Build Folder

**Redesigned Oct 28, 2026.** This node no longer drives lights and a fan — it switches power to the other three BSH systems (Elevator, FireAlarm, Door). Built and bench-tested first as a prototype on a spare ESP32 (`Prototype_PowerRails/`), then adopted here the same day. The old design (DALI → plain relay lights, fan, local override buttons) is archived: see `Archive/README.md` for the retired sketch and the `Fan_Relay/`/`Lights_Relay/` folders.

Companion to, not a replacement for:
- **`BSH_PowerBox_Node/BSH_PowerBox_Node.ino`** — the firmware to flash (power rails, WiFi values are placeholders — fill in your own copy, never commit real values).
- **`BSH_Systems/04_BSH-PowerBox.md`** — full system doc (IP, register map, role, decisions).
- **`BSH_Sensors/09_Relay_Module.md`** — general component reference doc.
- **`Schematic/Connections.md`** — as-built wiring (draft quality — several fields unconfirmed).

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the power-rails design (Phase A0) | `diagram.json` + `sketch.ino` + a guide — rebuilt Oct 28, 2026 from the prototype sim |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification; the Oct 28, 2026 entries record the redesign |
| ~~`Lights_Relay/`~~ | retired, archived | lights channel, dropped Oct 28, 2026 |
| ~~`Fan_Relay/`~~ | retired, archived | fan channel, dropped Oct 28, 2026 |

There is no per-rail subfolder yet (Elevator/FireAlarm/Door relay wiring); the only wiring record so far is `Schematic/Connections.md`.

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md` Section 7)
1. Wire and bench-test each of the three rail relays individually, confirming polarity on each board (not yet individually done — see `Firmware/BSH_PowerBox_Bringup_Checklist.md` Section 4)
2. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (done, Oct 28, 2026)
3. OpenPLC + SCADA integration (Phase C) — ladder built, `Program.st` regenerated, on-command path confirmed once (Elevator); SCADA tags not started

See `Firmware/BSH_PowerBox_Bringup_Checklist.md` for the full checklist and what is still open.
