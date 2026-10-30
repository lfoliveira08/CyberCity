# Adoption checklist — BSH-PowerBox becomes the power-rails node

**Started Oct 28, 2026 on Luis's go. BSH-PowerBox only; FH-PowerBox mirrors it later.** Rule: change the pair, not the half (`CLAUDE.md` §7, `Firmware/CLAUDE.md`, `PLC_Logic/CLAUDE.md`): firmware, system doc, bring-up checklist and PLC move together. Tick a box only when it is done and checked. Who: **Luis** builds hardware, rungs and flashes; **agent** edits docs, drafts and file moves.

Decisions already made (all in `Prototype_PowerRails/README.md`): NO contact; no lights, no fan, no buttons; 5V-pin method; rails off on WiFi loss after 5 s; rails off after power-up until commanded (option A); GPIO 25 Elevator, 26 FireAlarm, 27 Door; prototype ESP32 keeps address 192.168.50.14.

## Step 1 — Record the decision (agent)
- [x] `Logs/2026-10-28.md`: adoption go, what it supersedes
- [x] `STATUS.md`: BSH-PowerBox row says it is being replaced

## Step 2 — Firmware (agent drafts, Luis flashes)
- [x] Final sketch from `Firmware/PowerRails_Node/PowerRails_Node.ino` becomes `Firmware/BSH-PowerBox/BSH_PowerBox_Node/BSH_PowerBox_Node.ino` (path never changes, `Firmware/WORKFLOW.md` §8)
- [x] Old sketch to `Archive/` with a row in `Archive/README.md`
- [x] WiFi values: placeholders in the repo copy (Luis agreed Oct 28)
- [ ] Luis flashes it and the bench result is written in `Firmware/BSH-PowerBox/ESP32_Core_Modbus/Build_Log.md`

## Step 3 — System doc, schematic, checklist, component folders (agent) -- DONE Oct 28, 2026
- [x] `BSH_Systems/04_BSH-PowerBox.md` (role, GPIO map, register map, notes)
- [x] `Firmware/BSH_PowerBox_Bringup_Checklist.md`
- [x] `Firmware/BSH-PowerBox/Schematic/Connections.md`, `README.md`, `Wokwi/` (replaced with the trimmed power-rails sim), `SYSTEM_REPORT.md` (pointer note added, kept as history)
- [x] `Fan_Relay/` and `Lights_Relay/` moved to `Archive/BSH-PowerBox_Fan_Relay/` and `Archive/BSH-PowerBox_Lights_Relay/` with rows in `Archive/README.md`; the as-built wiring is `Firmware/BSH-PowerBox/Schematic/Connections.md` (several fields still unconfirmed, flagged in the file)
- [x] `BSH_Sensors/09_Relay_Module.md` (relay use changed)

## Step 4 — PLC (Luis builds the rungs; agent writes the docs)
- [x] `PLC_Logic/BSH-PowerBox/Register_Map.md` (3 coils, 3 discrete inputs, 2 input registers; published C0-C5, HR0-HR1)
- [x] `Ladder_Diagram.md` (8 rungs, `Prototype_PowerRails/Ladder_DRAFT.md` is the list)
- [x] Luis builds the 8 rungs in the OpenPLC Editor project `BSH-PowerBox-ESP32/` (saved Oct 28, 18:25); `Program.st` regenerated from it
- [~] `Program.st` uploaded and run on container .114 (on-command path bench-confirmed with Kali/modbus_coil_tool.py); compile log text still not captured
- [ ] Container .114 slave device: DI 3, coils 3, IR 2 (already set Oct 28) and its Settings values recorded

## Step 5 — SCADA drafts (agent)
- [ ] `SCADA_HMI/BSH-PowerBox/Tag_Map.md` and `Tags.json` (same sitting)
- [ ] `HMI_View.md`

## Step 6 — Project docs (agent)
- [ ] `Piranga_Workflow_v2.md` Section 14 is frozen: the change is recorded in `Logs/` instead; list what it supersedes (Power Box role, Scenario 7 and 9 meaning, the Section 7 resilience note)
- [ ] `00_START_HERE.md`, `Next_Steps.md` mentions; `References/INDEX.md` only if needed
- [ ] `STATUS.md` final

## Step 7 — Acceptance on the real build (Luis, with numbers)
- [ ] Comms loss: rails off after about 5 s, back when the PLC next writes "on"
- [ ] Power-up: rails stay off until commanded (option A)
- [ ] Each of the three rails with its real node, on and off
- [ ] Current measured for each node and the PowerBox's own port
- [ ] Relay pins do not flick at ESP32 reset

## Rollback
Old sketch in `Archive/`; real `PLC_Logic/BSH-PowerBox/Program.st` until replaced; container device values (DI 2, coils 2, IR 2, `.14`) in `PLC_Logic/BSH-PowerBox/Register_Map.md` section 2 (until Step 4 rewrites it, so copy them into the Build_Log first).

## Not part of this adoption
FH-PowerBox; the log size limit on VM 100; the credentials in the other node sketches.

## Hardware choice (Luis, Oct 28, 2026)
The already-wired spare ESP32 on the breadboard (the prototype build) **is** the new BSH-PowerBox hardware; the old lights-and-fan board is not rebuilt and is set aside. Consequences: the as-built docs describe the breadboard build; the real PowerBox ESP32 must stay off while this one uses 192.168.50.14; a breadboard is fragile for the Dec 11 demo, so a solid, labelled rebuild is a later, separate item (not decided). The spare's exact board model, pin count and USB connector are still not recorded.
