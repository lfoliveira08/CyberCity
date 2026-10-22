# FH-PowerBox -- PLC logic

OpenPLC instance `OpenPLC-FH-PowerBox` (container 192.168.50.124) polls this node's ESP32 at 192.168.50.24 and exposes it to SCADA.

**Status (Oct 21, 2026):** built in the OpenPLC Editor by Luis (8 rungs, task `T#20ms`), `Program.st` regenerated from the saved project and run on the container; slave ID 0 proven (Kali read, heartbeat 536 -> 541); functional tests reported as worked (no values). Compile log, Slave Devices listing and Settings values were not shown. Plain level copy, same as BSH-PowerBox. Details in `Build_Log.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: The ESP32 fails both outputs to OFF on comms loss and keeps local buttons working. The PLC forwards commands and publishes command-vs-actual mismatch.

Related: system design `FH_Systems/04_FH-PowerBox.md`; ESP32 firmware `Firmware/FH-PowerBox/`; bring-up checklist `Firmware/FH_PowerBox_Bringup_Checklist.md`.
