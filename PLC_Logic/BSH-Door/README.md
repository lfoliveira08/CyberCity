# BSH-Door -- PLC logic

OpenPLC instance `OpenPLC-BSH-Door` (container 192.168.50.113) polls this node's ESP32 at 192.168.50.13 and exposes it to SCADA.

**Status (Oct 11, 2026):** drafted only. The ladder diagram, register map and Structured Text program exist; nothing has been compiled in OpenPLC Editor or run on a container. Phase C has not been declared started.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: The ESP32 keeps its own safe-state watchdog. The PLC forwards commands and publishes the access-vs-command contrast.

Related: system design `BSH_Systems/03_BSH-Door.md`; ESP32 firmware `Firmware/BSH-Door/`; bring-up checklist `Firmware/BSH_Door_Bringup_Checklist.md`.
