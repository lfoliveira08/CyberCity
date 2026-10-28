# STATUS — as of Oct 27, 2026

**This is the only place project status lives.** Any agent answering "where are we" reads this file and nothing else. When status changes, change it here the same day, then write `Logs/YYYY-MM-DD.md`. Per-system detail stays in that system's `Build_Log.md`; do not copy status back into READMEs.

Deadline: **demo Dec 11, 2026**.

## One line

Phases A and B are done on all eight nodes. Phase C: the **OpenPLC layer is done** (all 8 programs built and running on their containers, through Oct 21). The **SCADA layer is started, barely** — Ignition 8.3.10 is installed and licensed, the first Modbus device is connected and its heartbeat verified, and **no real tags, UDT, screens or cascade rules exist yet**.

## Per system

| System | ESP32 | Phase B firmware | OpenPLC program | Ignition |
|---|---|---|---|---|
| BSH-FireAlarm | .12 | bench-confirmed | built, running (Oct 19 verified on hardware) | **device added + `HR1` heartbeat verified live (Oct 26)**; temp tag only |
| BSH-Elevator | .11 | bench-confirmed | built, running (Oct 20) | not started |
| BSH-Door | .13 | bench-confirmed | built, running (Oct 20) | not started |
| BSH-PowerBox | .14 | bench-confirmed | built, running (Oct 21) | not started |
| FH-HVAC | .21 | bench-confirmed | built, running (Oct 21) | not started |
| FH-FireAlarm | .22 | bench-confirmed | built, running (Oct 21) | not started |
| FH-Door | .23 | bench-confirmed | built, running (Oct 21) | not started |
| FH-PowerBox | .24 | bench-confirmed | built, running (Oct 21) | not started |

"Running" = `Program.st` uploaded to the container and the PLC started, with a Kali read test proving slave ID 0 and a rising heartbeat. Functional tests on the FH nodes and BSH-PowerBox were **reported by Luis, not pasted** — treat as reported, not verified.

## Infrastructure

| Host | IP | State |
|---|---|---|
| `pve` (Proxmox) | .143 | running; `local-lvm` thin pool over-provisioned — **filled to 100% on Oct 26 and paused VM 100** |
| VM 100 `openplc-docker` | .10 | running; 8 containers up (`.111`–`.114`, `.121`–`.124`) |
| VM 101 `scada-hmi` | .30 | running; Ignition 8.3.10, Capstone license active (expires 2027-01-31, lease renews ~72 h, **needs internet**) |
| VM 102 Kali | .40 | running; used for read tests |

## Next up (the SCADA layer)

1. Settle the tag path rule and the Device UDT in `SCADA_HMI/Templates/` **before** importing all eight nodes.
2. Build the real BSH-FireAlarm tag tree from `SCADA_HMI/BSH-FireAlarm/Tag_Map.md` (drafted Oct 26, **unverified against the live PLC**), then delete the temp tag `TestHeartbeat`.
3. Repeat per system, then the Campus_Map screen, then the cascade rules.

Guided step by step — Luis builds it in the Designer himself. Every Ignition step is looked up in `References/Ignition/` and cited (see `CLAUDE.md` §4).

## Open items

- [ ] **Zero-based vs one-based addressing is unproven.** Manual says `HR1` = 40001 (one-based) unless the device's Zero-based Addressing is on; the Oct 26 `HR1` heartbeat test (Luis-reported) may have read register 0. All draft tag files assume zero-based (Oct 27 review)
- [ ] Seven Tag_Maps and all eight `Tags.json` exist as drafts generated Oct 27, unverified and never imported; see `Logs/2026-10-27.md`
- [ ] Tag path convention `<Bldg>/<System>/<Name>` and the shared Device UDT: proposed Oct 26, **not settled**
- [ ] Unit ID and Zero-Based Addressing fields on the Ignition device were never inspected (defaults work for `HR1`)
- [ ] Delete temp tag `TestHeartbeat` once the real tree exists
- [ ] Confirm Ignition shows healthy Modbus connections after the Oct 26 VM 100 pause
- [ ] Move VM 100's disk off `local-lvm` to `Big_Storage`; tell the team to clone to `Big_Storage`; keep pool under ~80%
- [ ] Decide whether to keep VM 101 snapshots `before_ignition` and `ignition_licensed`
- [ ] Direct-write, WiFi-drop and unplug checks are open on **every** PLC node
- [ ] Compile logs / Settings values not captured on several containers
- [ ] FH-HVAC reports temperature with decimals (193) though the firmware says whole degrees — unexplained
- [ ] FH-FireAlarm gas read 0 against a ~2700 baseline; Luis reported the sensor is fine, **not re-measured**
- [ ] Whether the 72-hour license lease needs internet at renewal — question to Inductive, open since Oct 22

## Known-wrong things, so nobody "fixes" them wrong

- The claude.ai project **description** field says 10 ESP32 nodes and names an old SCADA choice. It is wrong and can only be edited by hand in claude.ai. The project has **eight** nodes and uses Ignition.
- `References/GitHub/` notes and `Archive/` still say ten nodes. Historical; do not build from them.
- `Piranga_Workflow_v2.md` Section 14 is frozen history through Oct 14, 2026 and mentions ten nodes in a Sep 10 entry. Correct for its date, wrong today.
