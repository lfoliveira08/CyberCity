# FH-FireAlarm -- PLC logic

OpenPLC instance `OpenPLC-FH-FireAlarm` (container 192.168.50.122) polls this node's ESP32 at 192.168.50.22 and exposes it to SCADA.

**Status (Oct 11, 2026):** drafted only. The ladder diagram, register map and Structured Text program exist; nothing has been compiled in OpenPLC Editor or run on a container. Phase C has not been declared started.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: The ESP32 already sounds the alarm locally from its own sensors. The PLC only forwards the remote command and publishes status.

Related: system design `FH_Systems/02_FH-FireAlarm.md`; ESP32 firmware `Firmware/FH-FireAlarm/`; bring-up checklist `Firmware/FH_FireAlarm_Bringup_Checklist.md`.
