# SCADA_HMI -- Ignition tags, screens and cascade rules for all 8 nodes


**Status and open items for the whole project live in `../STATUS.md`.** Do not repeat status here; update that file instead.

This folder is the one home for the SCADA and HMI side of each system: the tag map, the tag import file, the screen design, and the build and test notes. It sits beside `PLC_Logic/` (the OpenPLC side) and `Firmware/` (the ESP32 side) and follows the same one-folder-per-system layout.

The live Ignition project is stored on the Gateway (VM 101 `scada-hmi`, 192.168.50.30). This folder holds what is written down about it: documentation, tag import files, and exports. Never keep the only copy of anything in the Gateway.

**Status (Oct 29, 2026):** Ignition 8.3.10 is installed and licensed (Oct 22). The BSH-FireAlarm device and its 8 tags exist and were tested (see `../STATUS.md`); no UDT, screens or cascade rules yet. Luis builds the SCADA himself in the Designer, guided step by step, so he learns it.

## Ground truth for Ignition: use the local manual

The complete Ignition 8.3 User Manual is saved at `References/Ignition/` (1,571 Markdown pages). **Any time you set up, change or explain something in Ignition, look it up there first, cite the page you used, and say "not in the manual" if it is not covered.** Do not answer Ignition how-to questions from memory. Start with `References/Ignition/README.md`: it has the full rule and a map from topic (Modbus device, tags, UDTs, tag import, Perspective, alarms, scripting, backups) to the right folder. The manual wins on how Ignition works; this folder wins on what the project chose (addresses, tag names, IPs). `SCADA_HMI/CLAUDE.md` repeats this rule so it loads automatically for agents working here.

## Layout

```
SCADA_HMI/
  README.md          this file
  Setup/             Gateway facts, device settings, backup steps (no keys, tokens or passwords)
  Templates/         tag naming rule, UDT definitions, tag import template, view template
  Cascade/           cross-system rules and any scripts (Piranga_Workflow_v2.md Section 7)
  Campus_Map/        the top-level HMI: campus map, status colors, event log panel
  Exports/           Gateway backups and project exports (dated file names)
  <System>/          one folder per system, e.g. BSH-FireAlarm/
    README.md        what this system shows and controls in SCADA
    Tag_Map.md       OpenPLC address -> Ignition address -> tag path
    Tags.json        the tag import file (added when it exists and is tested)
    HMI_View.md      what this system's screen looks like and which tags drive it
    Build_Log.md     steps and test results
```

## Conventions

- Ignition talks only to the OpenPLC containers (192.168.50.111-.124, port 502), not to the ESP32s. Addresses come from the PLC's published block: `SCADA_*` coils and holding registers SCADA writes, `PUB_*` values SCADA reads (see each `PLC_Logic/<System>/Register_Map.md`).
- Tag paths follow `<Bldg>/<System>/<Name>`, for example `BSH/FireAlarm/Heartbeat`. In use on the pilot since Oct 29 but not formally confirmed; write it into `Templates/` before building the other seven.
- One Device UDT shared by all eight nodes (heartbeat, reconnect count, status). Proposed, not built.
- The ESP32 firmware owns safe-state behavior; the PLC forwards commands; cross-system cascade rules live here in SCADA.
- If a register map changes, update `Tag_Map.md` and `Tags.json` in the same sitting.
- Do not add `_v2` or `_final` to file names. Exports carry a date instead, for example `CyberCity_project_2026-10-26.zip`.
- Every change is logged the same day in `Logs/`.

## The 8 systems

| System | ESP32 | OpenPLC container (Ignition device host) | Kind | Status |
|---|---|---|---|---|
| BSH-FireAlarm | 192.168.50.12 | 192.168.50.112 | Fire Alarm | Device rebuilt and 8 tags built and tested Oct 29 (pilot node); see `../STATUS.md` |
| BSH-Elevator | 192.168.50.11 | 192.168.50.111 | Elevator | Not started |
| BSH-Door | 192.168.50.13 | 192.168.50.113 | Door/Gate | Not started |
| BSH-PowerBox | 192.168.50.14 | 192.168.50.114 | Power Box | Not started |
| FH-HVAC | 192.168.50.21 | 192.168.50.121 | HVAC zone unit | Not started |
| FH-FireAlarm | 192.168.50.22 | 192.168.50.122 | Fire Alarm | Not started |
| FH-Door | 192.168.50.23 | 192.168.50.123 | Door/Gate | Not started |
| FH-PowerBox | 192.168.50.24 | 192.168.50.124 | Power Box | Not started |

## Open questions to settle on the first device

1. ~~Modbus addressing in Ignition: 0-based or 1-based.~~ SETTLED Oct 29, 2026 (supersedes the Oct 26 note): the Gateway default for Zero-based Addressing is OFF and then `HR1` is register 0. **Turn Zero-based Addressing ON on every device**, so Ignition addresses equal the PLC register-map numbers (`HR0` gas, `HR1` heartbeat). Proof: BSH-FireAlarm, `Logs/2026-10-29.md`.
2. Unit ID the OpenPLC server answers to (0 is used on the slave side; confirm on the Ignition side).
3. Tag naming rule and Device UDT (see Conventions).
4. Whether the 72-hour license lease needs internet (question to Inductive open since Oct 22).
