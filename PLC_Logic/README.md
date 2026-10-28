# PLC_Logic -- OpenPLC programs for all 8 nodes


**Status and open items for the whole project live in `../STATUS.md`.** Do not repeat status here; update that file instead.

This folder is the one home for everything about the OpenPLC side of each system: the ladder diagram, the register-address translation, the program code, and the build and test notes. It sits beside `Firmware/` (the ESP32 side) and follows the same one-folder-per-system layout.

**Status (Oct 21, 2026):** all 8 programs are built in the OpenPLC Editor by Luis, regenerated into `Program.st` from the saved Editor projects, and run on their containers (table below). Phase C started Oct 13; the OpenPLC layer is done and the SCADA layer (Ignition on VM 101) is not started. Luis confirmed on the night of Oct 21 that the compile logs, Settings pages (S7 and EtherNet/IP off) and Slave Devices listings are fine on all eight containers (reported, not pasted). The direct-write, WiFi-drop and unplug checks are open on every node; see the open questions below and each `Build_Log.md`.

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
| BSH-FireAlarm | 192.168.50.12 | 192.168.50.112 | Fire Alarm | Built in the Editor and running (Oct 19); slave ID 0 proven, three functional tests passed, Kali read test passed (heartbeat 101 -> 131, Oct 20) |
| FH-FireAlarm | 192.168.50.22 | 192.168.50.122 | Fire Alarm | Built in the Editor and running (Oct 21); functional and Kali read tests passed; gas read 0, reported fine by Luis |
| BSH-Elevator | 192.168.50.11 | 192.168.50.111 | Elevator | Built in the Editor and running (Oct 20); functional and Kali read tests passed; comms-loss design A (fail-off stays in the ESP32) |
| BSH-Door | 192.168.50.13 | 192.168.50.113 | Door/Gate | Built in the Editor and running (Oct 20); slave ID 0 proven, Kali read test passed (heartbeat 599 -> 604), functional tests passed; a held open bit keeps the gate open and blocks the reader (accepted risk) |
| FH-Door | 192.168.50.23 | 192.168.50.123 | Door/Gate | Built in the Editor and running (Oct 21); Kali read test passed (heartbeat 156 -> 161), functional tests reported as worked; plain level copy for open/close, same as BSH-Door |
| BSH-PowerBox | 192.168.50.14 | 192.168.50.114 | Power Box | Built in the Editor and running (Oct 21); Kali read test passed (heartbeat 881 -> 887), lights and fan tests passed after the ESP32 firmware change (a coil write counts only when its value changes) |
| FH-PowerBox | 192.168.50.24 | 192.168.50.124 | Power Box | Built in the Editor and running (Oct 21); Kali read test passed (heartbeat 536 -> 541), functional tests reported as worked; plain level copy, mirror of BSH-PowerBox; the last of the eight PLC programs |
| FH-HVAC | 192.168.50.21 | 192.168.50.121 | HVAC zone unit | Built in the Editor and running (Oct 21); Kali read test passed (heartbeat 135 -> 140), functional tests reported as worked; plain level copy for the enable and a MOVE for the setpoint, comms-loss design A |

## Open questions to settle on the first real container

1. ~~Where slave-device data appears (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`).~~ SETTLED Oct 15: confirmed on BSH-FireAlarm's container -- the Slave Devices page states "attached to address 100 onward" and listed this device at exactly those addresses. Per-system ranges still verify on each system's own container.
2. Which Modbus slave ID the ESP32 firmware answers to. Oct 15: **slave ID 0** entered on BSH-FireAlarm (form accepted it); confirm by live polling (HEARTBEAT ticking). If it fails, retry ID 1.
3. ~~The exact Docker image to use for OpenPLC.~~ Settled Oct 13: built locally as `openplc:v3` from the OpenPLC_v3 repo (see `Setup/README.md`).
4. ~~FH-HVAC: the comms-loss direction for `HVAC_ENABLE`.~~ SETTLED Oct 21: fail-off in the ESP32 (design A), no PLC rung; the residual risk is recorded in `FH-HVAC/Build_Log.md`.
5. FH-Door: compile log, Settings values and test values were not shown on Oct 21 (compile log, Settings and listings later confirmed fine by Luis, not pasted; test values still not reported); direct-write, WiFi-drop and unplug checks are left for the attack-scenario session (see `FH-Door/Build_Log.md`).
6. FH-PowerBox: test values were not reported on Oct 21 (compile log, Slave Devices listing and Settings, S7 and EtherNet/IP off, later confirmed fine by Luis, not pasted; Polling Period and Timeout reported the same as BSH-FireAlarm); the Oct 21 firmware rule on FH was reported as working, no serial lines given; direct-write, WiFi-drop and unplug checks are left for the attack-scenario session (see `FH-PowerBox/Build_Log.md`).

## References

Outside sources used here are listed in `References/INDEX.md`:
- OpenPLC Runtime: `References/GitHub/AllNodes_thiagoralves_OpenPLC_v3.md`
- OpenPLC Editor docs, Modbus addressing (basis of `Templates/Address_Mapping.md`): Web articles table in `References/INDEX.md`
- Dunn, *Introduction to Instrumentation, Sensors, and Process Control*, Programmable Logic Controllers chapter (background): `References/Books/`
- Related papers on OpenPLC and Modbus TCP testbeds: `References/Papers/INDEX_Papers.md`, section A
