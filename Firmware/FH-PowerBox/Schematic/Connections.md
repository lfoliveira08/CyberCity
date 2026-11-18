# FH-PowerBox -- connection list (power rails)

**Rewritten Oct 28, 2026** for the power-rails redesign, mirroring `Firmware/BSH-PowerBox/Schematic/Connections.md`. Replaces the lights-and-fan connection list, archived in `Archive/FH-PowerBox_PLC_lights-and-fan/` and the `Archive/README.md` rows for the old `FH-PowerBox_Fan_Relay/`/`FH-PowerBox_Lights_Relay/` folders. **Draft quality, assumed to mirror BSH-PowerBox: Luis reported wiring and bench-testing this node (heartbeat, a Serial-toggled rail, a real node powered, and the Modbus on-command path all confirmed working), but did not give the per-pin wiring detail that was collected for BSH-PowerBox.** Treat every field below as an assumption until Luis confirms or corrects it against the real board. Where this file and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.24
- **Board:** not recorded (assumed similar to BSH-PowerBox's breadboard build -- not confirmed whether this used the original FH lights-and-fan board or a different one)
- **Sources:** assumed to mirror BSH-PowerBox's wiring pattern (`Firmware/BSH-PowerBox/Schematic/Connections.md`); confirmed only that the firmware runs and the relays switch real power (`PLC_Logic/FH-PowerBox/Build_Log.md`, Oct 28, 2026)

## Parts (assumed, mirroring BSH-PowerBox)

- ESP32 DevKit
- 3 × 1-channel 5V relay module (one per rail: FH-HVAC, FH-FireAlarm, FH-Door)
- No LEDs, no fan motor, no push buttons

## Connections (assumed pattern -- not individually confirmed per pin on this node)

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | FH-HVAC rail relay | IN | GPIO 25 | Polarity not individually confirmed on this board |
| 2 | FH-HVAC rail relay | VCC | this board's 5V/VIN rail | assumed |
| 3 | FH-HVAC rail relay | GND | this board's GND rail | assumed |
| 4 | FH-HVAC rail relay | COM | this board's 5V source | assumed to mirror BSH (fed from the PowerBox ESP32's own VIN pin); not confirmed for FH |
| 5 | FH-HVAC rail relay | NO | FH-HVAC node's 5V pin | assumed, pin label not recorded |
| 6 | FH-HVAC node | GND | this board's GND, joined directly (not through the relay) | assumed required, same as BSH |
| 7 | FH-FireAlarm rail relay | IN | GPIO 26 | same pattern as rows 2-6 |
| 8 | FH-FireAlarm rail relay | VCC/GND/COM/NO | same as FH-HVAC (rows 2-6), for FH-FireAlarm | assumed |
| 9 | FH-FireAlarm node | GND | this board's GND | assumed |
| 10 | FH-Door rail relay | IN | GPIO 27 | same pattern |
| 11 | FH-Door rail relay | VCC/GND/COM/NO | same as FH-HVAC, for FH-Door | assumed |
| 12 | FH-Door node | GND | this board's GND | assumed |

## Power and ground

- GND assumed common across the PowerBox board and all three downstream nodes (required for the design to work at all).
- COM assumed fed from this board's own VIN pin, mirroring BSH-PowerBox -- **not confirmed for FH**. If a different 5V source was used, this file needs correcting.
- Each downstream node's own USB cable assumed unplugged while its 5V pin is fed this way (the design requires it; not individually confirmed here).

## Contact type

**NO (normally open)**, mirrors BSH-PowerBox's Oct 27, 2026 decision. The Oct 28, 2026 Kali test confirmed the behaviour works end-to-end (coil write -> relay -> status readback), which is consistent with NO but does not by itself prove the physical contact wiring matches this file's assumption -- it proves the *result* is correct, not every intermediate pin.

## Check before trusting this file (open questions -- same list as BSH, none answered for FH yet)

- [ ] Board model and pin count
- [ ] Is relay VCC/GND actually on the same rail as COM (the VIN pin), or separate?
- [ ] The exact pin label on each node's expansion board that the NO wire lands on
- [ ] Relay contact rating and polarity confirmed per board (none tested on FH with a multimeter)
- [ ] Current draw: each node, and the PowerBox board itself
- [ ] Voltage measured at a node's power pins with its rail commanded on

## Fritzing notes

Re-drawn Nov 15, 2026 in Fritzing (by Jeraldini): `FH-PowerBox_Schematic_schematic.{pdf,png,svg}` shows the current power-rails design (3x relay rails on GPIO 25/26/27, mirroring BSH-PowerBox) — not the old lights-and-fan wiring. The old `_Draft.pdf` (lights-and-fan) has been removed from this folder.

## Files that belong in this folder

- `FH-PowerBox_Schematic.fzz` (not yet made for this design)
- `FH-PowerBox_Schematic.pdf` and/or `.png` (the current PDF is the old design)
- This file
