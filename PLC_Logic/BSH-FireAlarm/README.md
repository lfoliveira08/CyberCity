# BSH-FireAlarm -- PLC logic

OpenPLC instance `OpenPLC-BSH-FireAlarm` (container 192.168.50.112) polls this node's ESP32 at 192.168.50.12 and exposes it to SCADA.

**Status (Oct 15, 2026):** slave device added on the container (192.168.50.112), address convention confirmed live, and all 8 rungs built in the OpenPLC Editor (project at `BSH-FireAlarm-ESP32/`, verified identical to `Program.st`). **Not yet** compiled/Started on the container, and Monitoring not yet checked against the live ESP32. Deploy artifact for the v3 container is the hand-written `Program.st` (the v4 Editor does not export a loose `.st`).

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |
| `BSH-FireAlarm-ESP32/` | OpenPLC Editor (v4) project -- the visual ladder source; `build/` is git-ignored |

What the program does: The ESP32 already sounds the alarm locally from its own sensors. The PLC only forwards the remote command and publishes status.

Related: system design `BSH_Systems/02_BSH-FireAlarm.md`; ESP32 firmware `Firmware/BSH-FireAlarm/`; bring-up checklist `Firmware/BSH_FireAlarm_PILOT_Bringup.md`.
