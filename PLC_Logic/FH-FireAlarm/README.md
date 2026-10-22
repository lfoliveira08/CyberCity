# FH-FireAlarm -- PLC logic

OpenPLC instance `OpenPLC-FH-FireAlarm` (container 192.168.50.122) polls this node's ESP32 at 192.168.50.22 and exposes it to SCADA.

**Status (Oct 21, 2026):** built and running. Luis built the 8 rungs in the OpenPLC Editor (`FH-FireAlarm-ESP32/`, task `T#20ms`), `Program.st` was regenerated from it, uploaded and run on the container; the four functional tests were reported as worked and the Kali read test passed (heartbeat 149 -> 154). Gas read 0 on the Kali reads; Luis reported the sensor is fine (not re-measured). Open: Settings values not shown. Details in `Build_Log.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text, generated from the saved Editor project (upload in the OpenPLC web UI) |
| `FH-FireAlarm-ESP32/` | Luis's OpenPLC Editor project (visual source and learning record) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: The ESP32 already sounds the alarm locally from its own sensors. The PLC only forwards the remote command and publishes status.

Related: system design `FH_Systems/02_FH-FireAlarm.md`; ESP32 firmware `Firmware/FH-FireAlarm/`; bring-up checklist `Firmware/FH_FireAlarm_Bringup_Checklist.md`.
