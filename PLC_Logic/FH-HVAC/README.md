# FH-HVAC -- PLC logic

OpenPLC instance `OpenPLC-FH-HVAC` (container 192.168.50.121) polls this node's ESP32 at 192.168.50.21 and exposes it to SCADA.

**Status (Oct 21, 2026):** built and running. Luis built the 8 rungs in the OpenPLC Editor (`FH-HVAC-ESP32/`, task `T#20ms`), `Program.st` was regenerated from it, uploaded and run on the container; the functional tests were reported as worked and the Kali read test passed (heartbeat 135 -> 140). Open: compile log and Settings values not shown. Details in `Build_Log.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: copies the SCADA enable command (plain level copy) and the SCADA setpoint (MOVE) to the ESP32, and publishes the ESP32 status and measurements for SCADA. Comms loss: design A decided Oct 21, fail-off stays in the ESP32 (WiFi down more than 10 s opens the relay); the PLC adds nothing on top. See `Build_Log.md`.

Related: system design `FH_Systems/01_FH-HVAC.md`; ESP32 firmware `Firmware/FH-HVAC/`; bring-up checklist `Firmware/FH_HVAC_Bringup_Checklist.md`.
