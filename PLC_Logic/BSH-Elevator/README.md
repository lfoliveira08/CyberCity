# BSH-Elevator -- PLC logic

OpenPLC instance `OpenPLC-BSH-Elevator` (container 192.168.50.111) polls this node's ESP32 at 192.168.50.11 and exposes it to SCADA.

**Status (Oct 20, 2026):** built and running. Luis built the 9 rungs in the OpenPLC Editor (`BSH-Elevator-ESP32/`), `Program.st` was regenerated from that build, uploaded to the container and run against the real ESP32; the three functional tests passed (`Build_Log.md`).

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: the three call-floor commands go through a 300 ms `TP` pulse (one press = one pulse, even if SCADA holds the bit); the lockout (`ELEVATOR_DISABLE`) is a plain level copy; the ESP32 status is published for SCADA. Fault handling and the comms-loss behavior live in the ESP32 firmware.

Related: system design `BSH_Systems/01_BSH-Elevator.md`; ESP32 firmware `Firmware/BSH-Elevator/`; bring-up checklist `Firmware/BSH_Elevator_Bringup_Checklist.md`.
