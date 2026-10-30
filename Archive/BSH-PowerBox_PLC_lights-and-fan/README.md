# BSH-PowerBox -- PLC logic

OpenPLC instance `OpenPLC-BSH-PowerBox` (container 192.168.50.114) polls this node's ESP32 at 192.168.50.14 and exposes it to SCADA.

**Status (Oct 21, 2026):** built and running on real hardware. Luis built the 8 rungs in the OpenPLC Editor (project `BSH-PowerBox-ESP32/`, task `T#20ms`), `Program.st` was regenerated from it, uploaded and run on the container, and the functional tests and the Kali read test passed (details in `Build_Log.md`). Open: compile log not kept, Polling Period/Timeout not seen on this container, WiFi-drop and direct-write checks not done.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: the PLC copies the SCADA lights and fan commands to the ESP32 as plain levels (it rewrites them every poll), computes two mismatch flags (command XOR actual status), and republishes the status bits and the two counters for SCADA. The ESP32 fails both outputs to OFF on WiFi loss and keeps local buttons working. Since Oct 21, 2026 the ESP32 only treats a coil write as a hub command when its value differs from the last hub value, so a local button press sticks even though the PLC keeps writing (see `Build_Log.md`).

Related: system design `BSH_Systems/04_BSH-PowerBox.md`; ESP32 firmware `Firmware/BSH-PowerBox/`; bring-up checklist `Firmware/BSH_PowerBox_Bringup_Checklist.md`.
