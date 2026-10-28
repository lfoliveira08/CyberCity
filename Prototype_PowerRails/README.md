# Prototype_PowerRails — PowerBox as the building's power switch

**Status: PROTOTYPE. Not approved. Not part of the real design.** Opened Oct 27, 2026 from a design discussion with Luis (`Logs/2026-10-27.md`, evening sections).

## Goal (Luis, Oct 27, 2026)

A USB hub powers the PowerBox. The PowerBox's relays then switch power to the other systems in its building. That is all. Anything beyond that (motor and servo power banks, FireAlarm-first, charging stations) was raised by the agent as a side note and is **not** part of the goal unless Luis says so. My reading of the goal: the motor and servo power banks stay as they are today. Luis to confirm.

## Isolation rules (read first)

- **Nothing in this folder is the design.** The real PowerBox is `BSH_Systems/04_BSH-PowerBox.md`, `Firmware/BSH-PowerBox/`, `PLC_Logic/BSH-PowerBox/` and `SCADA_HMI/BSH-PowerBox/`. They still say lights + fan, and they are correct today.
- **Nothing outside this folder was changed** to create it (apart from a guard line in `CLAUDE.md` §5 and the log entries).
- **Do not build, flash, import or deploy anything from here** unless Luis has approved adoption (see below).
- **To discard the idea:** delete this folder and log it. Nothing else needs undoing.
- **The physical prototype uses its own parts**, never the real nodes: a spare ESP32 for the prototype PowerBox, new relay modules, and stand-in loads instead of the real Elevator, FireAlarm and Door. It does not use the real IPs (.11-.14, .111-.114), does not join the real OpenPLC containers, and runs Serial-only like the Wokwi sketch until Luis approves more. The real PowerBox keeps its fan, relays and sketch untouched.
- Every file here is a **draft**. Nothing was run, compiled, imported or tested on hardware.

## The idea

Drop the fan. The PowerBox ESP32 drives relays that switch the USB +5V of the other three nodes in its building:

| Building | PowerBox | Rails it would switch |
|---|---|---|
| BSH | BSH-PowerBox (.14) | Elevator (.11), FireAlarm (.12), Door (.13) |
| FH | FH-PowerBox (.24) | HVAC (.21), FireAlarm (.22), Door (.23) |

BSH first; FH mirrors it later (`Firmware/WORKFLOW.md` §7). The other six nodes' firmware, PLC programs, registers and tags do not change; only their USB power cabling does.

## What is in here

| File | What it holds |
|---|---|
| `Wokwi/` | Draft simulation: lights + 3 rail relays, downstream nodes shown as LEDs. **Not yet run.** |
| `Wiring_DRAFT.md` | Parts, per-rail wiring, USB breakout approach, what to check before buying or wiring |
| `Register_Map_DRAFT.md` | Candidate ESP32 map, OpenPLC slave settings and published block |
| `Ladder_DRAFT.md` | The rung list for the PowerBox PLC (names and purpose only; Luis builds the rungs) |
| `SCADA_DRAFT.md` | What would change in Ignition, derived node state, cascade and attack ideas |

## Open decisions (Luis)

| # | Decision | Notes |
|---|---|---|
| 1 | Go / no-go, and before or after the SCADA layer | Recommendation given: after SCADA |
| 2 | NC (fail-powered) or NO (fail-dark) contact | **Luis: fail-dark, i.e. NO** (Oct 27): when the PowerBox is off, the other systems turn off too. The Wokwi sim and the three drafts were flipped to NO the same day; the sim has **not** been re-run since. Cost: a PowerBox failure takes down three systems (against the `Piranga_Workflow_v2.md` §7 resilience claim), and the relay coils draw current while rails are on |
| 3 | **DECIDED (Luis, Oct 27): lights stay** | the prototype has 4 relay modules: 3 rails + lights |
| 4 | **DECIDED (Luis, Oct 27): no rail buttons** | rails are commanded only through the sketch (Serial in the prototype); the lights keep their one button |
| 5 | **DECIDED (Luis, Oct 27): USB-A male + female screw-terminal breakout pair per node** | relay between the two boards' +5V terminals; no cable is cut. VIN-rail rejected for now (back-feed unverified) |
| 6 | Real GPIOs for the three rails | 25/26/27 are sim-only placeholders |
| 7 | **DECIDED (Luis, Oct 27): rails turn OFF on comms loss**, same as the current rule. Consequence accepted: a WiFi drop or a stopped PowerBox PLC container darkens that building. Timeout length not read from the firmware. (Original question:) What the rails do when the PowerBox loses comms (WiFi drop, PLC container stopped) | Found Oct 27: `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md` Notes say the ESP32 fails both outputs to OFF on comms loss. With fail-dark rails that would black out the building on any WiFi drop. Options: hold the last state, or keep rails on. The ESP32 owns safe-state (`Firmware/CLAUDE.md`), so this is a firmware rule change. Not decided |
| 8 | How rails come ON after a power-up | Follows from 7: with fail-off, the rails stay off until something commands them. Today the command comes from SCADA through the PLC; the PLC's SCADA command bits very likely start at 0 after a restart (assumed, not checked). Options: the ESP32 turns rails on by itself once WiFi and the PLC are talking, or SCADA must send "on". If Ignition is down, the second option leaves the campus dark. Not decided |

## Facts still missing (do not assume)

- Contact rating: read off a photo of a Songle SRD-05VDC-SL-C module (10A 250VAC / 10A 30VDC) on Oct 27; not confirmed against the actual bench modules.
- What each node plugs into: Luis reported two USB hubs on a power extension (Oct 27). Hub labels (amps per port / total) not read.
- USB connector at the board end: Luis reported BSH-FireAlarm is USB-C and the rest micro-USB.
- Real current draw per node: not measured.
- Whether the DevKit blocks back-feed when USB and another 5V are both present: not verified.
- Whether the relay pins glitch during an ESP32 boot: not tested.
- The Elevator motor and Door servo need an external 5V supply per the docs; Luis reported they are usually on power banks (Oct 27). Cutting the ESP32's USB would not cut them (see `Wiring_DRAFT.md`). Decision needed: does a rail mean the ESP32 only, or ESP32 plus motor/servo supply?

## If Luis adopts it: what it would touch (one system at a time, BSH-PowerBox first)

Per `Firmware/CLAUDE.md` and `PLC_Logic/CLAUDE.md` these change together, in the same sitting:

1. `BSH_Systems/04_BSH-PowerBox.md` (role, GPIO map, register map)
2. `Firmware/BSH-PowerBox/` — the sketch, `Wokwi/`, `Schematic/Connections.md`, `Fan_Relay/` (retire to `Archive/`), the new rail wiring, `Firmware/BSH_PowerBox_Bringup_Checklist.md`
3. `PLC_Logic/BSH-PowerBox/` — `Register_Map.md`, `Ladder_Diagram.md`, `Program.st`, the OpenPLC slave-device sizes on the container
4. `SCADA_HMI/BSH-PowerBox/` — `Tag_Map.md` and `Tags.json`
5. `BSH_Sensors/09_Relay_Module.md` and the FH mirror files, later
6. Retired files go to `Archive/` with a row in `Archive/README.md`; a new `Logs/` entry supersedes the PowerBox role text

This prototype folder is then deleted or moved to `Archive/`.
