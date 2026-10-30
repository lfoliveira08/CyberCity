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
| `Wokwi/` | Draft simulation: 3 rail relays, downstream nodes shown as LEDs (lights trimmed out Oct 28; the trimmed version was not re-run). The earlier version ran in Wokwi, reported. |
| `Wiring_DRAFT.md` | Parts, per-rail wiring, USB breakout approach, what to check before buying or wiring |
| `Register_Map_DRAFT.md` | Candidate ESP32 map, OpenPLC slave settings and published block |
| `Ladder_DRAFT.md` | The rung list for the PowerBox PLC (names and purpose only; Luis builds the rungs) |
| `SCADA_DRAFT.md` | What would change in Ignition, derived node state, cascade and attack ideas |

## Open decisions (Luis)

| # | Decision | Notes |
|---|---|---|
| 1 | Go / no-go, and before or after the SCADA layer | Recommendation given: after SCADA |
| 2 | NC (fail-powered) or NO (fail-dark) contact | **Luis: fail-dark, i.e. NO** (Oct 27): when the PowerBox is off, the other systems turn off too. The Wokwi sim and the three drafts were flipped to NO the same day; the sim has **not** been re-run since. Cost: a PowerBox failure takes down three systems (against the `Piranga_Workflow_v2.md` §7 resilience claim), and the relay coils draw current while rails are on |
| 3 | **DECIDED (Luis, Oct 28): the lights are OUT; the PowerBox only powers the other systems** | **Supersedes** the Oct 27 "lights stay". 3 relay modules, no lights LEDs or button. The Wokwi sim and the bench sketch were trimmed the same day (not re-run) |
| 4 | **DECIDED (Luis, Oct 27): no rail buttons** | rails are commanded only through the sketch (Serial in the prototype) |
| 5 | **DECIDED (Luis, Oct 28): the 5V-pin method** | each relay feeds a node's 5V pin by jumper wire, node USB unplugged. **Supersedes** the Oct 27 choice of USB breakout pairs. Bench-tested once (reported). Open: the 5V source for three nodes; USB plus 5V pin together is unverified |
| 6 | Rail-to-GPIO map | **Luis (Oct 28): GPIO 25 = Elevator, GPIO 26 = FireAlarm, GPIO 27 = Door** (same order as the sim). Applies to the prototype; not a decision about the real PowerBox. All three are ordinary pins per the ESP-IDF GPIO table; boot-time pin behaviour not tested |
| 7 | **DECIDED (Luis, Oct 27): rails turn OFF on comms loss**, same as the current rule. Consequence accepted: a WiFi drop or a stopped PowerBox PLC container darkens that building. Timeout length not read from the firmware. (Original question:) What the rails do when the PowerBox loses comms (WiFi drop, PLC container stopped) | Found Oct 27: `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md` Notes say the ESP32 fails both outputs to OFF on comms loss. With fail-dark rails that would black out the building on any WiFi drop. Options: hold the last state, or keep rails on. The ESP32 owns safe-state (`Firmware/CLAUDE.md`), so this is a firmware rule change. Not decided |
| 8 | **DECIDED (Luis, Oct 28): option A, rails stay OFF after power-up until commanded on** (by SCADA, or until Ignition exists a Modbus write to the PLC's `SCADA_*_RAIL_CMD` bits; Serial command on the bench). Matches the draft sketch (`RAILS_ON_AT_BOOT` false). Cost: if nothing sends the command, the buildings stay dark. (Original question:) How rails come ON after a power-up | Follows from 7: with fail-off, the rails stay off until something commands them. Today the command comes from SCADA through the PLC; the PLC's SCADA command bits very likely start at 0 after a restart (assumed, not checked). Options: the ESP32 turns rails on by itself once WiFi and the PLC are talking, or SCADA must send "on". If Ignition is down, the second option leaves the campus dark. Not decided |

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

## Added Oct 28, 2026

- Luis reported all nodes sit on an ESP32 expansion board (barrel jack, two USB ports, a 3.3V/5V jumper). Not yet known: which port the power cable uses on each node (the DevKit's own or the board's), what the jumper selects, and whether the barrel jack is used. See `Logs/2026-10-28.md`.
- Luis reported (Oct 28) one ESP32 DevKitC screw-terminal breakout board and seven "nano" boards (my reading: the expansion boards in the earlier photo). Unknown: whether the screw-terminal board is free for the prototype's spare ESP32. See `Logs/2026-10-28.md`.
- Luis reported (Oct 28): the screw-terminal breakout board is free; the prototype's spare ESP32 sits on it. Terminal labels, 5V and GND terminals not yet checked on the real board.
- Luis reported (Oct 28): every node has a USB cable plugged in, and none of the cables is connected to any power. All real nodes are off.
- Luis said (Oct 28): forget the breakout board; the PowerBox prototype now uses an expansion board and a breadboard. Reading: the blue screw-terminal board is dropped; the USB breakout pairs stay. Unconfirmed.
- Luis reported (Oct 28): first bench run on the spare ESP32 with one relay on GPIO 25 worked like the Wokwi sim (details not given, not seen). Polarity of that one board assumed active-HIGH, unconfirmed.
- Luis reported (Oct 28): multimeter continuity on one relay board: connected when the relay is on, not connected when cut. Which terminals were probed not stated. NO contact behaviour unconfirmed until the terminals are identified.
- Luis reported (Oct 28): COM and NO probed on one relay board, connected when on and open when cut: NO behaviour confirmed for that board (reported, not seen).
- Luis reported (Oct 28): one relay powered a second spare ESP32 through its 5V pin (relay COM on the PowerBox VIN, NO on the second spare's 5V, GND joined, second spare's USB unplugged) and it worked. Decision 5 may change from USB breakout pairs to the 5V-pin method; not decided. See `Logs/2026-10-28.md`.
- Luis reported (Oct 28): typing `1` cut the second spare's power and it came back. One relay and one stand-in only. Decision 5 (USB breakout pairs vs 5V-pin method) proposed to Luis, not decided.
- Exception (Oct 28, Luis's choice): he powered the real BSH-FireAlarm through the prototype relay's NO output at its VIN. Reported, not seen. Firmware and PLC untouched; real node must go back to its normal power when done.

## Added Oct 28, 2026 (evening)

- `Firmware/PowerRails_Node/PowerRails_Node.ino`: draft WiFi + Modbus firmware for the prototype, written from the real BSH-PowerBox sketch. Never compiled or flashed. IP and WiFi lines are FILL IN placeholders; no password or IP is stored here.
- `PLC/Program.st`: draft Structured Text for a **prototype** OpenPLC container only. Never compiled. Uploading it to a real container replaces the program that container runs. **Luis decided (Oct 28) to test it on the BSH-PowerBox container (.114)**; any other real container is off limits. Restore the real program from `PLC_Logic/BSH-PowerBox/Program.st` and the slave device from `PLC_Logic/BSH-PowerBox/Register_Map.md` section 2 afterwards. This is an exception to the prototype-only rule, by Luis's choice.
- Correction to decision 7: the real node's fail-safe detects WiFi LINK loss only, not a stopped PLC container or a silent master (`Firmware/BSH-PowerBox/BSH_PowerBox_Node/BSH_PowerBox_Node.ino`, lines 48-51). So a stopped PowerBox PLC container would NOT darken the building; a WiFi drop of more than 5 seconds would. This supersedes the wording in decision 7 and in the Oct 27 log.
- Luis created `PLC/BSH-POWERBOX-PROTOTYPE/` (Oct 28), an empty OpenPLC Editor project (no rungs, no variables). The prototype logic is in `PLC/Program.st`; do not upload a build of the empty project.
- **ON HOLD (Luis, Oct 28):** nothing from this folder goes into the real design, including the real BSH-PowerBox container (.114), until Luis confirms. This supersedes the earlier line saying Luis decided to test the ST on .114.
- Oct 28 evening: Luis said he will upload the prototype ST to test it; read as lifting the hold for the BSH-PowerBox container only (unconfirmed). Original slave device values captured from his screenshot; restore from `PLC_Logic/BSH-PowerBox/Register_Map.md` section 2.
- Oct 28 evening: Luis reported the prototype worked with the actual systems powered through it (not seen). The prototype ESP32 uses the real BSH-PowerBox address 192.168.50.14, so the real PowerBox ESP32 must stay off while it runs. The flashed sketch now holds real WiFi values on disk; replace them with placeholders before sharing or committing this folder.
- **Decision 1 DECIDED (Luis, Oct 28): go.** Adopt the power-rails design on BSH-PowerBox, staged. The plan and tick boxes are in `ADOPTION_CHECKLIST.md`. This folder stays as the reference until the real files are updated, then moves to `Archive/`.
- **Decided (Luis, Oct 28): keep this folder** as the reference/template for FH-PowerBox's adoption; archive it once FH is also adopted, not before.
