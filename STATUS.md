# STATUS — as of Oct 29, 2026

**This is the only place project status lives.** Any agent answering "where are we" reads this file and nothing else. When status changes, change it here the same day, then write `Logs/YYYY-MM-DD.md`. Per-system detail stays in that system's `Build_Log.md`; do not copy status back into READMEs.

Deadline: **demo Dec 11, 2026**.

## One line

Phases A and B are done on all eight nodes. Phase C: the **OpenPLC layer is done** (all 8 programs built and running on their containers, through Oct 21). The **SCADA layer: all four BSH systems have a device and tags (Oct 29), the FH systems and the UDT, screens and cascade do not**; it started on the pilot node — Ignition 8.3.10 is installed and licensed, and on Oct 29 the `BSH-FireAlarm` device was rebuilt and its 8 tags were built by hand in the Designer (`BSH/FireAlarm/`), read and write tested; **no UDT, screens or cascade rules exist yet, and the other seven nodes have no device**.

## Per system

| System | ESP32 | Phase B firmware | OpenPLC program | Ignition |
|---|---|---|---|---|
| BSH-FireAlarm | .12 | bench-confirmed | built, running (Oct 19 verified on hardware) | **Oct 29: device rebuilt (Zero-based Addressing ON), 8 tags built by hand in `BSH/FireAlarm/` (3 registers, 5 coils); Heartbeat climbing, GasRaw live, ManualPull followed a real pull-station press, AlarmActivate write sounded the buzzer (Luis-reported).** Flame and GasSmokeAlert also turned true when triggered (Luis-reported, Oct 29). Tags exported Oct 29 (`SCADA_HMI/Exports/`), identical to `Tags.json`, which was still never imported. See `Logs/2026-10-29.md` |
| BSH-Elevator | .11 | bench-confirmed | built, running (Oct 20); PLC found stopped Oct 29 and restarted by hand | **Oct 29: device connected, 9 tags built in `BSH/Elevator/` and exported (matches `Tags.json`). Heartbeat climbing and CurrentFloor matching the car (Luis-reported, so Zero-based is ON); a CallFloor write moved the car (Luis-reported, no details); ElevatorMoving, ElevatorFault (only seen false), ReconnectCount and ElevatorDisable not proven.** |
| BSH-Door | .13 | bench-confirmed | built, running (Oct 20); PLC found stopped Oct 29 and restarted by hand | **Oct 29: device connected, 8 tags built in `BSH/Door/` and exported (matches `Tags.json`), Heartbeat increasing (Luis-reported). IrDetected and AccessGranted followed real triggers (Luis-reported); **DoorPosition reads true open and closed (known issue, undiagnosed, accepted by Luis Oct 29)**; writes not done yet.** |
| BSH-PowerBox | .14 | **Redesigned Oct 28, 2026: power rails, not lights+fan.** Firmware replaced and flashed (heartbeat counting, status lines pasted); PLC rungs built in the Editor, `Program.st` regenerated, uploaded, clean compile reported, heartbeat confirmed live on the container; on-command path confirmed once via Kali (Elevator rail, coil write -> PLC -> ESP32 -> status readback, relay LED lit); system doc, bring-up checklist, schematic, Wokwi sim, `BSH_Sensors/09_Relay_Module.md` all rewritten for the new design (adoption Step 3 done); old lights+fan design archived. **Still open:** per-board relay polarity not individually confirmed for FireAlarm/Door, a rail reset to OFF with the cause not pinned down (PLC restart vs. WiFi drop), no current or voltage measured, SCADA tags not started (Step 5), project-doc supersede entries not written (Step 6). Checklist: `Prototype_PowerRails/ADOPTION_CHECKLIST.md` | container .114 runs the regenerated `Program.st`; compile log text not captured | **Oct 29: device connected, 8 tags built in `BSH/PowerBox/` and exported, `Tag_Map.md`/`Tags.json` rewritten for power rails. Heartbeat increasing; writing true to each rail command clicked its relay and set its status (Luis-reported, nothing measured). Rails not connected to the nodes; false direction untested.** |
| FH-HVAC | .21 | bench-confirmed | built, running (Oct 21) | not started |
| FH-FireAlarm | .22 | bench-confirmed | built, running (Oct 21) | not started |
| FH-Door | .23 | bench-confirmed | built, running (Oct 21) | not started |
| FH-PowerBox | .24 | **Redesigned Oct 28, 2026, mirroring BSH-PowerBox: power rails (FH-HVAC/FireAlarm/Door), not lights+fan.** Firmware flashed (WiFi joined, heartbeat ticking, reported); system doc, bring-up checklist, Wokwi sim and relay doc rewritten; PLC rungs built in the Editor (saved Oct 28, 19:33, checked from the saved file), `Program.st` regenerated, container .124's slave device edited and verified (3/3/2, addresses match), uploaded and reported working ("tested, working nice" -- no specific checks or screenshot given). **Confirmed (Luis):** heartbeat ticking on Monitoring, a rail toggled via Serial, a real FH node powered on through its relay, and the Modbus on-command path through the PLC (Kali write -> PLC -> ESP32 -> relay -> status readback, same as BSH-PowerBox's Elevator test). **Still open:** per-rail relay polarity, measured current/voltage, `Firmware/FH-PowerBox/Schematic/Connections.md` still describes the old lights+fan wiring, SCADA tags not started, compile log and Settings page not captured. Old lights+fan design archived | container .124 runs the regenerated `Program.st`, reported working | not started |

"Running" = `Program.st` uploaded to the container and the PLC started, with a Kali read test proving slave ID 0 and a rising heartbeat. Functional tests on the FH nodes and BSH-PowerBox were **reported by Luis, not pasted** — treat as reported, not verified.

## Infrastructure

| Host | IP | State |
|---|---|---|
| `pve` (Proxmox) | .143 | running; `local-lvm` thin pool over-provisioned — **filled to 100% on Oct 26 and paused VM 100** |
| VM 100 `openplc-docker` | .10 | running; 8 containers up (`.111`–`.114`, `.121`–`.124`); **root filesystem was 100% full on Oct 28 (Luis screenshot); freed to 43% used the same evening by emptying one container log; no log size limit yet, so it will refill** |
| VM 101 `scada-hmi` | .30 | running; Ignition 8.3.10, Capstone license active (expires 2027-01-31, lease renews ~72 h, **needs internet**) |
| VM 102 Kali | .40 | running; used for read tests |

## Next up (the SCADA layer)

1. Settle the tag path rule and the Device UDT in `SCADA_HMI/Templates/` **before** building the other seven nodes. The `BSH/FireAlarm/` folder layout is in use on the pilot but not formally confirmed.
2. Finish BSH-FireAlarm: tag export done Oct 29 and matches `Tags.json`; still to do: a project export or Gateway backup (dated). Then **repeat per system with Zero-based Addressing ON on each new device**.
3. **Monday: the FH systems, in order HVAC, FireAlarm, Door, PowerBox** (`Firmware/WORKFLOW.md` §1), each with **Zero-based Addressing ON**, the PLC started by hand if stopped, and the tags exported. Then the Gateway backup (all eight devices), the Device UDT and `Templates/`, the Campus_Map screen and the cascade rules. Open on the four BSH systems (details in `Logs/2026-10-29.md`): proofs for `ReconnectCount`, `GasRaw`, `ElevatorMoving`, `ElevatorDisable`, the Door writes and `DoorPosition`, and the PowerBox `false` direction.

Guided step by step — Luis builds it in the Designer himself. Every Ignition step is looked up in `References/Ignition/` and cited (see `CLAUDE.md` §4).

## Open items

- [x] Zero-based vs one-based addressing (opened Oct 27, **closed Oct 29**): the Gateway default is OFF and then `HR1` is register 0 (gas), not the heartbeat; the Oct 26 "HR1 = heartbeat" note was wrong. Zero-based is now ON for `BSH-FireAlarm` and tags use the register map numbers (`HR0` gas, `HR1` heartbeat, `HR2` reconnects; `C0`-`C4` coils). Luis-reported plus screenshots. **Applies to all 8 devices: turn it ON when creating each.**
- [ ] Oct 29 tag state: `ReconnectCount` consistent, not proven; `GasRaw` not shown to follow the sensor (`Flame`/`GasSmokeAlert` closed, see below). Seven Tag_Maps and all eight `Tags.json` exist as drafts generated Oct 27, unverified and never imported; see `Logs/2026-10-27.md`
- [ ] Tag path convention `<Bldg>/<System>/<Name>` and the shared Device UDT: proposed Oct 26; the folder layout `BSH/FireAlarm` is in use on the pilot (Oct 29), the UDT is **not built**, `Templates/README.md` not updated
- [x] Unit ID and Zero-Based fields (opened Oct 26, closed Oct 29): Zero-based seen in screenshots; the device form has **no Unit ID field** (the unit ID is an optional prefix in the item path, `modbus-addressing.md`); default prefix used
- [x] Delete temp tag `TestHeartbeat` (closed Oct 29: deleted with the old device; Luis-reported)
- [ ] Confirm Ignition shows healthy Modbus connections after the Oct 26 VM 100 pause: **only `BSH-FireAlarm` (.112) seen Connected on Oct 29, after Luis started its PLC by hand**; the PLCs were found not running (cause unknown); the other seven devices do not exist yet
- [ ] Move VM 100's disk off `local-lvm` to `Big_Storage`; tell the team to clone to `Big_Storage`; keep pool under ~80%
- [ ] **Oct 28: the BSH-PowerBox container (.114) answered "Error: database or disk is full" when Luis tried to delete its slave device** (screenshot seen; the delete did not go through, device state not rechecked). Cause found from Luis's screenshot: VM 100's root filesystem `/dev/sda2` is 40G used of 40G (100%, 0 available). **Where the space is (screenshots):** `/var/lib/docker/containers` is 26G, of which the `OpenPLC-FH-FireAlarm` container's folder is about 24G and `OpenPLC-BSH-FireAlarm`'s 1.2G; the others are under 430M. Confirmed from screenshots: it is the Docker log of `OpenPLC-FH-FireAlarm` (one 24G `*-json.log`, still growing when listed). The end of the file is OpenPLC's Modbus DEBUG output for every transaction (successful `ReadCoilsResponse`), not an error. `run_containers.sh` sets no log size limit, and 24G over about 11 days is roughly 2 GB a day (agent arithmetic) per container that polls a live node, so all 8 would fill 40G in days once every node is on. The `local-lvm` pool usage was not read. All 8 containers share VM 100, so none can write to its `openplc.db` safely until space is freed. Details in `Logs/2026-10-28.md`. **Update, same evening:** Luis emptied the 24G log with `truncate`; `df -h /` then showed 40G size, 16G used, 22G free (43%, screenshot). Still open: a log size limit for all containers (Luis deferred it on Oct 28; it refills), recheck that `openplc.db` is intact. Resolved: the FH-FireAlarm and BSH-Door nodes were powered on (Luis reported), so their containers were logging normal polling
- [ ] Decide whether to keep VM 101 snapshots `before_ignition` and `ignition_licensed`
- [ ] BSH-Door `DoorPosition` (reed switch, GPIO 34, no internal pull-up) reads true with the gate open and closed; not diagnosed, accepted by Luis Oct 29 as minor (opened Oct 29)
- [ ] Direct-write, WiFi-drop and unplug checks are open on **every** PLC node
- [ ] Compile logs / Settings values not captured on several containers
- [ ] FH-HVAC reports temperature with decimals (193) though the firmware says whole degrees — unexplained
- [ ] FH-FireAlarm gas read 0 against a ~2700 baseline; Luis reported the sensor is fine, **not re-measured**
- [ ] Whether the 72-hour license lease needs internet at renewal — question to Inductive, open since Oct 22

## Known-wrong things, so nobody "fixes" them wrong

- The claude.ai project **description** field says 10 ESP32 nodes and names an old SCADA choice. It is wrong and can only be edited by hand in claude.ai. The project has **eight** nodes and uses Ignition.
- `References/GitHub/` notes and `Archive/` still say ten nodes. Historical; do not build from them.
- `Piranga_Workflow_v2.md` Section 14 is frozen history through Oct 14, 2026 and mentions ten nodes in a Sep 10 entry. Correct for its date, wrong today.
