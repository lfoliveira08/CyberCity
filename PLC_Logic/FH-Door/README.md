# FH-Door -- PLC logic

OpenPLC instance `OpenPLC-FH-Door` (container 192.168.50.123) polls this node's ESP32 at 192.168.50.23 and exposes it to SCADA.

**Status (Oct 21, 2026):** built and running. Luis built the 8 rungs in the OpenPLC Editor (`FH-Door-ESP32/`, task `T#20ms`), `Program.st` was regenerated from it, uploaded and run on the container; the Kali read test passed (heartbeat 156 -> 161) and the functional tests were reported as worked. Open: compile log and Settings values not shown. Details in `Build_Log.md`.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: the PLC forwards the open and close commands as plain level copies (same as BSH-Door; SCADA must send pulses) and publishes the access-vs-command contrast. The ESP32 keeps its own safe-state watchdog.

Related: system design `FH_Systems/03_FH-Door.md`; ESP32 firmware `Firmware/FH-Door/`; bring-up checklist `Firmware/FH_Door_Bringup_Checklist.md`.
