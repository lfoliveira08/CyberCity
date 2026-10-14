# FH-HVAC -- PLC logic

OpenPLC instance `OpenPLC-FH-HVAC` (container 192.168.50.121) polls this node's ESP32 at 192.168.50.21 and exposes it to SCADA.

**Status (Oct 11, 2026):** drafted only. The ladder diagram, register map and Structured Text program exist; nothing has been compiled in OpenPLC Editor or run on a container. Phase C has not been declared started.

| File | What it is |
|---|---|
| `Ladder_Diagram.md` | The rungs drawn as a ladder diagram, with a plain-language line for each rung |
| `Register_Map.md` | ESP32 registers, OpenPLC slave-device settings, and the address translation |
| `Program.st` | The same logic as OpenPLC Structured Text (upload in the OpenPLC web UI) |
| `Build_Log.md` | Phase C steps and test results, filled in during the build |

What the program does: DECISION PENDING: which way HVAC_ENABLE fails on comms loss (fail-off vs fail-open) is not decided in the system doc yet. This draft is pass-through only; add the comms-loss rung once that is decided.

Related: system design `FH_Systems/01_FH-HVAC.md`; ESP32 firmware `Firmware/FH-HVAC/`; bring-up checklist `Firmware/FH_HVAC_Bringup_Checklist.md`.
