# Firmware Folder — Workflow & Process

This file is the source of truth for how work moves through `Firmware/`
from here on. Read it before starting or switching any system.

## 1. Build order

**Phase 1 — BSH systems, in this order:**
1. BSH-FireAlarm
2. BSH-Elevator
3. BSH-Door
4. BSH-PowerBox

**Phase 2 — FH systems**, same order once all four BSH systems are done.
Three of the four mirror BSH pin-for-pin per the system docs (same
components, same GPIO maps). **FH-HVAC is the exception** — it replaced
FH-Elevator on Sep 26, 2026 and has no BSH counterpart, so it gets its own
pin map, its own Wokwi simulation and its own bring-up from scratch:
1. FH-FireAlarm
2. FH-HVAC
3. FH-Door
4. FH-PowerBox

> Note (Oct 5, 2026): the fifth system in each building, Backup Power / ATS
> Monitor, was dropped by Luis together with the course professor. Each
> building now has four systems (eight nodes in total). Its files are in
> `Archive/BackupPower/`; see `Piranga_Workflow_v2.md` Section 14.

> Note (updated Sep 23, 2026): the order above replaces both earlier
> orders — the original "Fire Alarm -> Power Box -> Perimeter -> Elevator
> -> Door" and the previous version of this file ("FireAlarm -> Perimeter
> -> Door -> PowerBox -> Elevator"). Every README, bring-up checklist, and
> `Piranga_Workflow_v2.md` Section 9 has been updated to match. If any
> other file disagrees, this file is authoritative.

## 2. Single-focus rule

Only one system is "active" at a time. Work stays scoped to that system's
Phase A bring-up — wiring and bench-testing its own components, then a
standalone integration test proving they work together (no WiFi, no
Modbus) — until it's confirmed working end-to-end.

- Focus only changes when the user explicitly says to switch, or when
  Claude asks first and gets a yes before changing focus.
- No wiring or testing a different system "while we're at it."
- No touching WiFi, Modbus, or OpenPLC for any system until the user says
  to start that phase (see Section 5).
- The active system's scope starts with its Wokwi simulation (Phase A0,
  Section 7), then moves to the real bench (Phase A).

## 3. Sensor / wiring alignment

Keep pin conventions and documentation format consistent across systems
so nothing has to be re-learned system to system:

- Every component subfolder always has the same two files: `Wiring.md`
  (general reference — specs, pinout, hazards) and `Build_Log.md` (the
  actual build — status checklist, date log, Lessons Learned,
  Troubleshooting Tips). One **optional** third file is allowed:
  `BenchTest.ino`, the throwaway sketch that exercised that one component in
  isolation during Phase A. Added Sep 26, 2026 — these sketches were being
  written and then left outside the project structure entirely, which is worse
  than keeping them next to the log that references them. Keep them
  single-component and no-network; the system's combined sketch lives
  elsewhere (Section 8).
- Two folders are **not** component folders and do not follow that rule:
  `Wokwi/` (the system-level simulation, Section 7) and `ESP32_Core_Modbus/`
  (the board/WiFi/Modbus bring-up milestone — it holds only a `Build_Log.md`,
  because there is nothing component-specific to wire beyond USB power, so a
  `Wiring.md` there would be an empty file). Documented Sep 26, 2026 — this
  was already the practice in every system, just never written down.
- Shared hardware conventions already established on Fire Alarm and
  carried forward: GPIO 34-39 have no internal pull-up/down resistors in
  silicon (`INPUT_PULLUP` compiles but does nothing there — external
  pull-up required, e.g. Door's Reed Switch); any 5V digital output
  feeding a 3.3V ESP32 GPIO needs a 1kΩ/2kΩ voltage divider (e.g. the
  HC-SR04's ECHO pin); ADC1 pins (32, 33, 34, 35, 36, 39) are preferred
  for analog reads since ADC2 pins get unreliable once WiFi is active.
- A hardware substitution or correction (wrong part in hand, a pin that
  doesn't do what a reference doc assumed, etc.) gets logged where it's
  found — in that component's `Wiring.md`/`Build_Log.md` — and then
  carried into that system's completion report (Section 4), not just
  left in chat history.

## 4. Per-system completion report

Once a system's Phase A is fully done (every component bench-tested
standalone, then confirmed together in one combined integration test),
before moving on to the next system, produce
`Firmware/<System>/SYSTEM_REPORT.md` covering:

- **How the system works** — its role, its trigger/output sources, the
  local state logic
- **The code** — the current firmware/test sketch, in full
- **The wiring** — one consolidated pin table across all of that
  system's components
- **Schema** — the target Modbus register map (what it will expose once
  Phase B adds networking, even though that hasn't started yet)
- **Troubleshooting** — Lessons Learned + Troubleshooting Tips pulled
  together from each component's `Build_Log.md`/`Wiring.md`
- **Simulation** — link to the system's `Wokwi/` folder, the stand-ins
  used, and anything the simulation got wrong compared with the bench

## 5. WiFi / OpenPLC

**Phase B started Oct 5, 2026** at Luis's instruction, for BSH-FireAlarm,
BSH-Elevator and BSH-PowerBox (BSH-Door waits for its Phase A). Still one
system at a time, in this order: FireAlarm, then Elevator, then PowerBox. A
system's Phase B is done when its `ESP32_Core_Modbus/Build_Log.md` checklist
passes from a laptop Modbus client. Phase C (OpenPLC + SCADA) **started Oct 13, 2026** with the
infrastructure: the OpenPLC Docker VM, the `openplc:v3` image, the macvlan
network and the first container (BSH-FireAlarm); see `Next_Steps.md`. The
original deferral text below is historical.

**NETWORK (decided Oct 6, 2026):** the lab router SCT5 is a **192.168.50.x** network (gateway 192.168.50.1, SSID `SCT5`) and Luis cannot change it, so the whole project uses 192.168.50.x with the same last numbers as before: hub 192.168.50.10, nodes .11-.24, OpenPLC containers .111-.124. Every sketch, doc and checklist in this folder was updated that day (Archive and the `Alternate_Design_*` files were left alone). Before using any address, `ping` it from the laptop -- the router hands out DHCP leases in the same range and cannot be told to avoid ours, so a reply means another device already has it.

**HOSTING (decided Oct 9, 2026):** Phase C runs on a Proxmox VE server (`pve`, 192.168.50.143, Web UI :8006). The OpenPLC Docker VM (192.168.50.10) holds the 8 OpenPLC containers; the SCADA/HMI layer (Ignition; license confirmed Oct 19, 2026) runs in a separate VM (IP to be assigned). Create the VMs when Luis starts Phase C. See `Piranga_Workflow_v2.md` Section 6 (Hosting layer) and Section 10.

**PLC LOGIC (Oct 11, 2026):** the OpenPLC side of each system lives in `PLC_Logic/<Bldg>-<System>/` (ladder diagram, register map, `Program.st`, build log), not in the bring-up checklists. All 8 were built in the OpenPLC Editor and run on their containers by Oct 21, 2026 (per-node status in `PLC_Logic/README.md`); a node's `Build_Log.md` there holds its Phase C steps and results.

**OPENPLC EDITOR PROJECTS (Oct 15, 2026):** when a system's ladder logic is built in the OpenPLC Editor (the v4 desktop app), the Editor project is saved as a subfolder of that system's PLC_Logic folder, named `<Bldg>-<System>-ESP32` (e.g. `PLC_Logic/BSH-FireAlarm/BSH-FireAlarm-ESP32/`). One per system. The Editor stores the program in `pous/programs/main.ld` plus `project.json` and `devices/`; its `build/` subfolder is regenerable compiled output and is git-ignored (see `.gitignore` at the folder root). The authoritative, human-readable `Program.st` still lives at `PLC_Logic/<Bldg>-<System>/Program.st` and is what gets uploaded to the v3 container; the Editor project is the visual source and learning record. Note: the v4 Editor compiles ST to C++ for its target and does not export a loose `.st` for the v3 web UI, so the hand-written `Program.st` is the v3 deploy artifact.

**Standard registers on every node (Oct 6, 2026):** every Modbus node exposes `HEARTBEAT` (once-a-second counter) and `RECONNECT_COUNT` as its last two Input Registers, so SCADA can detect a dropped ESP32. A node's firmware, system doc, bring-up checklist and OpenPLC Input Register count must all agree on this -- update all four together or none. Source of truth for the numbers: the node's `BSH_Systems/` or `FH_Systems/` doc.

### Original deferral (Phase B for Door/FH, and Phase C for all)

Phase B (WiFi + Modbus TCP slave, verified standalone from a laptop tool)
and Phase C (OpenPLC + SCADA integration) are **not started for any
system** — including deciding which WiFi/communication protocol to
standardize on — until the user explicitly says to begin that phase.
Until then, every system's firmware is its Phase A standalone integration
test sketch: no WiFi, no Modbus, same pattern as `BSH_FireAlarm_Node.ino`
today.

## 6. Status tracker

| # | System | Wokwi (A0) | Status (A/B/C) |
|---|---|---|---|
| 1 | BSH-FireAlarm | **Built** (retroactively, as the template) — `BSH-FireAlarm/Wokwi/` | **Phase A complete** — see `BSH-FireAlarm/SYSTEM_REPORT.md`. **Phase B complete** (bench-confirmed 2026-10-06 at 192.168.50.12 on the SCT5 network -- see the network note below) |
| 2 | BSH-Elevator | **Built & verified** — `BSH-Elevator/Wokwi/` (run 2026-09-23) | **Phase A complete** — see `BSH-Elevator/SYSTEM_REPORT.md`. **Phase B complete** (bench-confirmed 2026-10-07 at 192.168.50.11): remote floor calls, momentary coils, DISABLE lockout, local buttons, heartbeat/reconnect all passed. DI1 fault cannot latch in the current sketch. Second in line |
| 3 | BSH-Door | **Built & run** (2026-10-09, Luis: looks fine) -- `BSH-Door/Wokwi/`; RC522, servo, RGB LED native, IR and reed as slide-switch stand-ins | **Phase A: Luis reported it fine on the bench (2026-10-09)**; no per-step results recorded -- `IR_ACTIVE_LOW` / `REED_LOW_MEANS_CLOSED` still at their defaults and the reed-module pull-up unrecorded. **Phase B bench-confirmed 2026-10-09** at 192.168.50.13 (`BSH-Door/BSH_Door_Node/BSH_Door_Node.ino`): remote open/close coils work with no card, card badge raises ACCESS_GRANTED, heartbeat/reconnect, WiFi watchdog. IR_DETECTED and DOOR_POSITION confirmed by Luis on the bench; the Phase A sketch is kept in `Archive/Door_PhaseA_sketches/`. `BSH-Door/SYSTEM_REPORT.md` finalized 2026-10-09 with the Phase B results |
| 4 | BSH-PowerBox | **Built & run** — `BSH-PowerBox/Wokwi/` (Sep 28–29, 2026) | **Phase A complete** (2026-10-05) — both relays and both local override buttons bench-confirmed in the standalone integration test, both relay boards active-HIGH. `SYSTEM_REPORT.md` written Oct 5 (Section 4). **Phase B complete** (bench-confirmed 2026-10-06 at 192.168.50.14): laptop writes, status feedback, local buttons, hub-regains-authority, 5s Wi-Fi watchdog and heartbeat/reconnect counters all passed **Oct 21, 2026:** Phase C ran on real hardware (program running, functional and Kali read tests passed); the sketch's hub-write rule was changed (a coil write counts only when its value differs from the last hub value) because OpenPLC rewrites the coils every poll -- see `PLC_Logic/BSH-PowerBox/Build_Log.md`. Old sketch in `Archive/` |
| 5 | FH-FireAlarm | Reuses BSH's simulation | **Phase B complete** (bench-confirmed 2026-10-06 at 192.168.50.22). Built at Luis's request ahead of the build order (single-focus rule waived for this one node). Mirrors BSH-FireAlarm's sketch. FH's own Phase A was never logged separately -- the first Serial run and the Phase B tests covered it. Clean-air gas baseline on FH is about 2700 (BSH's is about 40), so FH's gas sensor is a different part or wiring; threshold margin of 400 held **Oct 21, 2026:** Phase C ran on real hardware (program running, four functional tests reported as worked, Kali read test passed, heartbeat 149 -> 154); the PLC read the gas value as 0 on both reads, against the 2700 baseline above -- Luis reported the sensor is fine (not re-measured), see `PLC_Logic/FH-FireAlarm/Build_Log.md`  |
| 6 | FH-HVAC | **Needs its own** — no BSH counterpart to copy | **Phase A bench-confirmed 2026-10-07** (idle, cooling, enable; Phase B WiFi+Modbus also bench-confirmed; started early at Luis's request while BSH/FH-Door hardware is pending). `FH-HVAC/SYSTEM_REPORT.md` written. **Phase B complete** (bench-confirmed 2026-10-07 at 192.168.50.21): TEMP_C/HUMIDITY live, setpoint HR0 drives the relay, HVAC_ENABLE clear forces it off, sensor-unplug latches HVAC_FAULT, WiFi cut fails off (10 s) and recovers, heartbeat/reconnect counters pass. **Oct 21, 2026:** Phase C ran on real hardware (program running, functional tests reported as worked, Kali read test passed, heartbeat 135 -> 140); comms-loss design A (fail-off stays in the ESP32, no PLC rung), see `PLC_Logic/FH-HVAC/Build_Log.md`. Design doc: `FH_Systems/01_FH-HVAC.md` |
| 7 | FH-Door | Reuses BSH's simulation -- copied to `FH-Door/Wokwi/` 2026-10-09 (same as BSH's, which ran fine) | **Phase A: Luis reported it fine on the bench (2026-10-09)**, per-step results not recorded (same open flags as BSH-Door). **Phase B bench-confirmed 2026-10-09** at 192.168.50.23 (`FH-Door/FH_Door_Node/FH_Door_Node.ino`), all tests confirmed by Luis: remote open/close coils, card badge (FH card 3D D2 C6 62 confirmed), ACCESS_GRANTED pulse, IR/reed, heartbeat/reconnect, WiFi watchdog. Phase A sketch in `Archive/Door_PhaseA_sketches/`. `FH-Door/SYSTEM_REPORT.md` finalized 2026-10-09 with the Phase B results **Oct 21, 2026:** Phase C ran on real hardware (Luis built the 8 rungs in the Editor, task `T#20ms`; program running on 192.168.50.123; functional tests reported as worked; Kali read test passed, heartbeat 156 -> 161); plain level copy for open/close, same as BSH-Door (a held open bit keeps the gate open and blocks the reader, accepted risk); compile log and Settings values not shown, see `PLC_Logic/FH-Door/Build_Log.md` |
| 8 | FH-PowerBox | Reuses BSH's simulation | **Phase B complete** (bench-confirmed 2026-10-07 at 192.168.50.24): laptop writes, status feedback, local buttons, hub-regains-authority, 5s Wi-Fi watchdog and heartbeat/reconnect counters all passed. Built at Luis's request ahead of the build order; mirror of BSH-PowerBox, folder `FH-PowerBox/`. FH's relay boards are active-HIGH (verified: outputs OFF at boot with both flags `false`). Tested together with BSH-PowerBox (.14 and .24 online at once, confirmed by Luis 2026-10-09) **Oct 21, 2026:** the same hub-write rule change was made in `FH_PowerBox_Node.ino` ahead of FH Phase C (flashed per Luis); old sketch in `Archive/` **Oct 21, 2026 (later):** Phase C ran on real hardware (Luis built the 8 rungs in the Editor, task `T#20ms`; program running on 192.168.50.124; Kali read test passed, heartbeat 536 -> 541; functional tests incl. the changed-value rule on FH reported as worked, no values); plain level copy, same as BSH-PowerBox; compile log, Slave Devices listing and Settings values not shown, see `PLC_Logic/FH-PowerBox/Build_Log.md`. All eight PLC programs now exist |

**Phase C (OpenPLC), as of Oct 21, 2026:** all 8 OpenPLC containers are running on the OpenPLC Docker VM (192.168.50.10), each with its ESP32 slave device and its program loaded and running (the last, FH-PowerBox, on Oct 21): the OpenPLC layer is done. The SCADA/HMI VM (VM 101 `scada-hmi`, Ubuntu 24.04) was built on Oct 14 but has no SCADA software yet (Ignition is licensed, not installed), so Phase C as a whole is not done. Per-system Phase C status lives in `PLC_Logic/<System>/Build_Log.md`; addresses and commands in `PLC_Logic/Setup/Container_Cheatsheet.md`.

Update this table as each system's Phase A0 and Phase A complete.

*(Oct 5, 2026: the former rows for BSH-BackupPower and FH-BackupPower were
removed when that system was dropped. Its Wokwi simulation had been built and
verified on Oct 5 and is kept in `Archive/BackupPower/`.)*

## 7. Wokwi simulation first (Phase A0)

*Added Sep 23, 2026.* Before any system's Phase A bench build starts, it
gets a Wokwi (wokwi.com) simulation using the same pin map and the same
logic. The goal is understanding: what each sensor outputs, how it's
wired, and how the system reacts, before real parts are involved.

```
A0  Wokwi simulation  ->  A  Bench hardware  ->  B  WiFi + Modbus  ->  C  OpenPLC + SCADA
    (learn it)            (prove it's real)
```

Rules:
- Every BSH system gets `Firmware/<System>/Wokwi/` containing:
    - `diagram.json` — parts and wiring. GPIO numbers must match the
      system's pin table in `BSH_Systems/` exactly.
    - `sketch.ino` — the standalone test logic (no WiFi, no Modbus),
      kept as close to the real Phase A sketch as possible. Any
      simulation-only change sits behind a `WOKWI_SIM` flag.
    - `README.md` — how to open it, what to click, what you should see,
      and a "Real bench differences" list.
    - `libraries.txt` — only if the sketch needs libraries.
- FH systems mirror BSH pin-for-pin, so they reuse the BSH simulation.
  Copy it into `Firmware/FH-<System>/Wokwi/` when that FH system becomes
  active. **FH-HVAC is the exception** — it has no BSH counterpart, so it
  needs its own `diagram.json`/`sketch.ino` built from scratch (all three of
  its parts — DHT22, relay module, RGB LED — are native Wokwi parts, so no
  stand-ins are needed).
- A0 is done when every scenario in the Wokwi README behaves as described.
  Log it in each component's `Build_Log.md`, then start Phase A.
- **Simulation is not proof.** Wokwi does not model voltage levels,
  missing pull-ups, noise, current draw, or sensor warm-up. Every hardware
  caveat in Section 3 (voltage dividers, GPIO 34-39 pull-ups, ADC2 with
  WiFi) still has to be built and checked on the real bench.
- Parts Wokwi doesn't have get a labeled stand-in (listed in each
  `BSH_Systems/` doc and in each `BSH_Sensors/` sheet under "In Wokwi").
  A stand-in teaches the logic, not the real part's wiring.
- The `Wokwi/` folder is one of the two non-component folders called out in
  Section 3, so its file rule is its own.
- A0 covers Phase A only. Phases B/C (Modbus to OpenPLC on the LAN) stay
  on real hardware, since a simulated ESP32 can't be reached by the
  hub's OpenPLC containers.

## 8. Where files live

*Added Sep 26, 2026, after an audit found working sketches sitting outside the
project structure in a `Claude outputs/` download folder — including one stale
draft using an abandoned pin mapping, which is exactly the kind of file that
gets flashed by mistake. There is one home for each kind of file:*

| What | Where | Notes |
|---|---|---|
| A node's current firmware | `Firmware/<Bldg>-<System>/<Bldg>_<System>_Node/<Bldg>_<System>_Node.ino` | One per node. Holds the Phase A standalone sketch now, replaced in place by the WiFi+Modbus version when Phase B starts — the path never changes |
| Simulation | `Firmware/<Bldg>-<System>/Wokwi/` | `diagram.json`, `sketch.ino`, `README.md`, optional `libraries.txt` |
| A node's schematic | `Firmware/<Bldg>-<System>/Schematic/` | `Connections.md` (the connection list the drawing comes from) and `<Bldg>-<System>_Schematic_Draft.pdf` (generated drawing, not a Fritzing file). Added Oct 12, 2026 |
| Per-component wiring + log | `Firmware/<Bldg>-<System>/<Component>/` | `Wiring.md`, `Build_Log.md`, optional `BenchTest.ino` (Section 3) |
| Board/WiFi/Modbus bring-up | `Firmware/<Bldg>-<System>/ESP32_Core_Modbus/` | `Build_Log.md` only |
| Completion report | `Firmware/<Bldg>-<System>/SYSTEM_REPORT.md` | Written when Phase A completes (Section 4) |
| A node's PLC program (OpenPLC) | `PLC_Logic/<Bldg>-<System>/` | `README.md`, `Register_Map.md`, `Ladder_Diagram.md`, `Program.st`, `Build_Log.md`. Shared pieces in `PLC_Logic/Templates/` and `PLC_Logic/Setup/`. Added Oct 11, 2026 |
| Bring-up checklist | `Firmware/<Bldg>_<System>_Bringup_Checklist.md` | One level up, beside this file |
| System design doc | `BSH_Systems/` or `FH_Systems/` | IP, pin map, register map, role |
| Component reference sheet | `BSH_Sensors/` or `FH_Sensors/` | One per building, mirrored |
| Retired files | `Archive/` | Never build from these; see `Archive/README.md` |
| Wireshark/pcap evidence for the Section 11 attack scenarios | `Security_Capture/` | One home for this activity (added Oct 14, 2026): `README.md`, `Capture_Setup.md`, `Scenario_Capture_Plan.md`, `Capture_Log_Template.md`, `Captures/` |
| Attack scripts and attack reports for the Section 11 scenarios (run from the Kali VM) | `Kali/` | Created Oct 20, 2026; **not in use until the attack-scenario session**. `README.md` only for now. Pcap evidence stays in `Security_Capture/` |
| Daily decisions and lessons learned | `Logs/` | One file per working day, `YYYY-MM-DD.md`, plus `INDEX.md` and `README.md` (the rules). Added Oct 15, 2026; continues `Piranga_Workflow_v2.md` Section 14, which is frozen as history up to Oct 14, 2026 |

Rules that follow from the table:

- **A file is only "real" once it is in the place above.** Anything in a
  downloads folder, a chat, or a `Claude outputs/` directory is a copy in
  transit. Move it to its home in the same sitting, or delete it.
- **Never keep two copies of a sketch.** If a variant has to be preserved
  (an alternate pin mapping, an older approach), it goes in `Archive/` with a
  row in `Archive/README.md` saying why — not beside the live file under a
  name like `..._Node-1.ino`.
- **Every change gets logged the same day.** Any new file, retired file, decision, bench result or lesson is recorded in that day's file in `Logs/` (rules in `Logs/README.md`), in addition to the component `Build_Log.md` where it applies. Added Oct 15, 2026.
- **The firmware path is stable across phases.** Do not add `_PhaseA`,
  `_v2`, or `_final` to a sketch name; the file is the current one by
  definition, and `SYSTEM_REPORT.md` records what it was at completion.

Wokwi support for this project's parts (checked Sep 23, 2026 against
docs.wokwi.com):

| Native in Wokwi | Stand-in needed |
|---|---|
| Push button, I2C 16x2 LCD, buzzer*, RGB LED, SG90 servo, relay module, MFRC522 RFID reader, MQ-2 gas sensor, slide switch, potentiometer | L293D + DC motor (LEDs), IR obstacle sensor (slide switch), reed switch (slide switch), flame sensor (slide switch), ACS712 current sensor (potentiometer), UPS battery voltage divider (potentiometer) |

\*Wokwi's buzzer acts like a passive piezo: the simulation uses `tone()`,
while the real active buzzer uses `digitalWrite`.
