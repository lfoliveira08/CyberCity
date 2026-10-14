# BSH-Elevator -- PLC logic

OpenPLC instance `OpenPLC-BSH-Elevator` (container 192.168.50.111) polls this node's ESP32 at 192.168.50.11 and exposes it to SCADA.

**Status (Oct 11, 2026):** drafted only. The ladder diagram, register map and Structured Text program exist; nothing has been compiled in OpenPLC Editor or run on a container. Phase C has not been declared started.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: Pure pass-through. Fault handling and the comms-loss behavior live in the ESP32 firmware.

Related: system design `BSH_Systems/01_BSH-Elevator.md`; ESP32 firmware `Firmware/BSH-Elevator/`; bring-up checklist `Firmware/BSH_Elevator_Bringup_Checklist.md`.
