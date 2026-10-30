# BSH-PowerBox -- PLC logic

OpenPLC instance `OpenPLC-BSH-PowerBox` (container 192.168.50.114) polls this node's ESP32 at 192.168.50.14 and exposes it to SCADA.

**Status (Oct 28, 2026):** redesigned: the BSH-PowerBox now switches power to the other three BSH systems (three rails) instead of driving lights and a fan. Luis rebuilt the 8 rungs in the OpenPLC Editor (project `BSH-PowerBox-ESP32/`, edited in place, saved 18:25) and the agent checked the saved project file (all 8 rungs wired as designed); `Program.st` was regenerated from it. **Open:** `Program.st` has not been compiled or uploaded from this file yet (container .114 ran a draft with the same addresses, Luis-reported), the compile log and the container's Settings values were never captured, and the Modbus on-command through the PLC is untested. The old design is archived in `Archive/BSH-PowerBox_PLC_lights-and-fan/`. Overall status: `../../STATUS.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The 8 rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation (power-rails version) |
| `Program.st` | The same logic as OpenPLC Structured Text, regenerated from the Editor build (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results; the Oct 28 entries record the redesign |

What the program does: the PLC copies the three SCADA rail commands to the ESP32 as plain levels (it rewrites them every poll) and republishes the three rail status bits and the two counters for SCADA. The ESP32 turns all rails off after 5 s without WiFi and keeps them off after power-up until commanded on. Since Oct 21, 2026 the ESP32 only treats a coil write as a hub command when its value differs from the last hub value.

Related: system design `BSH_Systems/04_BSH-PowerBox.md` (still the old design until adoption Step 3); ESP32 firmware `Firmware/BSH-PowerBox/`; bring-up checklist `Firmware/BSH_PowerBox_Bringup_Checklist.md` (old design until Step 3); adoption plan `Prototype_PowerRails/ADOPTION_CHECKLIST.md`.
