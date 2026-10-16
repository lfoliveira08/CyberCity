# PLC_Logic -- OpenPLC programs for all 8 nodes

This folder is the one home for everything about the OpenPLC side of each system: the ladder diagram, the register-address translation, the program code, and the build and test notes. It sits beside `Firmware/` (the ESP32 side) and follows the same one-folder-per-system layout.

**Status (Oct 14, 2026):** all 8 systems are drafted. Phase C started Oct 13: the `openplc:v3` image is built and all 8 containers are running with a blank program (BSH-FireAlarm since Oct 13, the other 7 since Oct 14). Nothing here has been compiled in OpenPLC Editor or run on a container yet. Treat every program as a reference draft: you may rewrite any of them yourself and replace the draft.

## Layout

```
PLC_Logic/
  README.md                 this file
  Templates/                the pass-through pattern and the address-mapping cheat sheet
  Setup/                    Docker/macvlan setup notes, scripts and the container cheat sheet (verified Oct 13-14, 2026)
  <System>/                 one folder per system, e.g. BSH-FireAlarm/
    README.md               what the program does
    Register_Map.md         ESP32 registers -> OpenPLC addresses -> SCADA addresses
    Ladder_Diagram.md       the rungs drawn out, with plain-language notes
    Program.st              the code (OpenPLC Structured Text)
    Build_Log.md            Phase C steps and test results
```

## Conventions

- `SCADA_*` names are bits/words SCADA writes into the PLC's own Modbus server. `PUB_*` names are values the program publishes for SCADA to read. Names without a prefix are the ESP32's own register names.
- The ESP32 firmware owns safe-state behavior (watchdogs, fail-to-OFF, local overrides). The PLC forwards commands and publishes status.
- Cross-system cascade rules live in the SCADA layer (`Piranga_Workflow_v2.md` Section 7), not in these programs.
- Once a program is built and tested in OpenPLC, export it as text and save it as `Program.st` in its system folder. The saved file is the current one; do not add `_v2` or `_final` to names.
- If the ESP32 register map changes, update `Register_Map.md`, `Ladder_Diagram.md` and `Program.st` together or none.
- The ESP32 register map in `BSH_Systems/` or `FH_Systems/` is the source of truth. This folder follows it.

## The 8 systems

| System | ESP32 | OpenPLC container | Kind | Status |
|---|---|---|---|---|
| BSH-FireAlarm | 192.168.50.12 | 192.168.50.112 | Fire Alarm | Drafted; container running (Oct 13), program not loaded yet |
| FH-FireAlarm | 192.168.50.22 | 192.168.50.122 | Fire Alarm | Drafted; container running (Oct 14), program not loaded yet |
| BSH-Elevator | 192.168.50.11 | 192.168.50.111 | Elevator | Drafted; container running (Oct 14), program not loaded yet |
| BSH-Door | 192.168.50.13 | 192.168.50.113 | Door/Gate | Drafted; container running (Oct 14), program not loaded yet |
| FH-Door | 192.168.50.23 | 192.168.50.123 | Door/Gate | Drafted; container running (Oct 14), program not loaded yet |
| BSH-PowerBox | 192.168.50.14 | 192.168.50.114 | Power Box | Drafted; container running (Oct 14), program not loaded yet |
| FH-PowerBox | 192.168.50.24 | 192.168.50.124 | Power Box | Drafted; container running (Oct 14), program not loaded yet |
| FH-HVAC | 192.168.50.21 | 192.168.50.121 | HVAC zone unit | Drafted; container running (Oct 14), program not loaded yet |

## Open questions to settle on the first real container

1. ~~Where slave-device data appears (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`).~~ SETTLED Oct 15: confirmed on BSH-FireAlarm's container -- the Slave Devices page states "attached to address 100 onward" and listed this device at exactly those addresses. Per-system ranges still verify on each system's own container.
2. Which Modbus slave ID the ESP32 firmware answers to. Oct 15: **slave ID 0** entered on BSH-FireAlarm (form accepted it); confirm by live polling (HEARTBEAT ticking). If it fails, retry ID 1.
3. ~~The exact Docker image to use for OpenPLC.~~ Settled Oct 13: built locally as `openplc:v3` from the OpenPLC_v3 repo (see `Setup/README.md`).
4. FH-HVAC: the comms-loss direction for `HVAC_ENABLE`.

## References

Outside sources used here are listed in `References/INDEX.md`:
- OpenPLC Runtime: `References/GitHub/AllNodes_thiagoralves_OpenPLC_v3.md`
- OpenPLC Editor docs, Modbus addressing (basis of `Templates/Address_Mapping.md`): Web articles table in `References/INDEX.md`
- Dunn, *Introduction to Instrumentation, Sensors, and Process Control*, Programmable Logic Controllers chapter (background): `References/Books/`
- Related papers on OpenPLC and Modbus TCP testbeds: `References/Papers/INDEX_Papers.md`, section A
