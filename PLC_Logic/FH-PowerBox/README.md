# FH-PowerBox -- PLC logic

OpenPLC instance `OpenPLC-FH-PowerBox` (container 192.168.50.124) polls this node's ESP32 at 192.168.50.24 and exposes it to SCADA.

**Status (Oct 28, 2026):** redesigned, mirroring BSH-PowerBox: the FH-PowerBox now switches power to the other three FH systems (three rails) instead of driving lights and a fan. Luis built the 8 rungs in the OpenPLC Editor (project `FH-PowerBox-ESP32/`, edited in place, saved 19:33) and the agent checked the saved project file (all 8 rungs wired as designed, structurally identical to BSH's build); `Program.st` was regenerated from it. **Open:** `Program.st` has not been compiled or uploaded from this file yet, the compile log and the container's Settings values were never captured, and the Modbus on-command through the PLC is untested. The old design is archived in `Archive/FH-PowerBox_PLC_lights-and-fan/`. Overall status: `../../STATUS.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The 8 rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation (power-rails version, VERIFIED on this container) |
| `Program.st` | The same logic as OpenPLC Structured Text, regenerated from the Editor build (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results; the Oct 28 entries record the redesign |

What the new program will do (mirrors BSH-PowerBox exactly): the PLC copies three SCADA rail commands to the ESP32 as plain levels, republishes three rail status bits and the two counters. The ESP32 turns all rails off after 5 s without WiFi and keeps them off after power-up until commanded on.

Related: system design `FH_Systems/04_FH-PowerBox.md`; ESP32 firmware `Firmware/FH-PowerBox/`; bring-up checklist `Firmware/FH_PowerBox_Bringup_Checklist.md`; adoption plan `Prototype_PowerRails/ADOPTION_CHECKLIST.md` (written for BSH; FH mirrors the same steps).
