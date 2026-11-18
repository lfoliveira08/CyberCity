# BSH-PowerBox -- connection list (power rails)

**Rewritten Oct 28, 2026** for the power-rails redesign, from Luis's reports and one bench photo (not a measurement). Replaces the lights-and-fan connection list, archived in `Archive/BSH-PowerBox_PLC_lights-and-fan/` and `Archive/README.md`'s row for the old `Fan_Relay/`/`Lights_Relay/` folders. **Draft quality: several fields below are Luis's verbal report or the agent's assumption, not a verified measurement — see the "Still to fill in" list at the bottom.** Where this file and the bench disagree, the bench wins; fix this file.

**Elevator rail: SUPERSEDED Nov 17, 2026 — no longer wired this way, kept below as history only.** The Elevator rail relay's COM/NO were found reversed from row 4-5 below (COM actually went to the Elevator's 5V pin; NO went to this board's 5V source), and the relay's power path brownout-looped the Elevator node under motor load even after correcting that (bench-confirmed with a multimeter: direct USB power was stable, the relay rail was not). **Luis decided to power the Elevator over a permanent dedicated 5V USB supply instead of continuing to fix the rail.** The Elevator rail relay (GPIO 25, coil `ELEV_RAIL_CMD`) is now wired to nothing — left in place, unused. The cascade and Scenario 5/6/7 now use the Elevator's own `ELEVATOR_DISABLE` coil instead of this rail. See `STATUS.md`, `SCADA_HMI/Cascade/README.md`, and `Logs/2026-11-17.md`. The FireAlarm and Door rails below are unaffected and still power those nodes.

- **Node IP:** 192.168.50.14
- **Board:** ESP32 DevKit on a breadboard (the former spare board, not the original lights-and-fan board). Pin count and exact model not recorded.
- **Sources:** Luis's reports (`Logs/2026-10-28.md`), one bench photo, `Prototype_PowerRails/AsBuilt_Wiring_DRAFT.md` (the working draft this file was promoted from)

## Parts

- ESP32 DevKit (breadboard build)
- 3 × 1-channel 5V relay module (Songle SRD-05VDC-SL-C type, reported)
- No LEDs, no fan motor, no push buttons

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | Elevator rail relay | IN | GPIO 25 | Polarity not individually confirmed on this board |
| 2 | Elevator rail relay | VCC | this board's 5V/VIN rail | assumed, not confirmed as the same rail as #3 |
| 3 | Elevator rail relay | GND | this board's GND rail | assumed |
| 4 | Elevator rail relay | COM | this board's **VIN pin** | **WRONG — bench found reversed Nov 17, 2026: COM actually went to the Elevator's 5V pin (row 5), NO went to this board's VIN. Superseded; this whole rail is now unused — see note at top of file.** |
| 5 | Elevator rail relay | NO | Elevator node's 5V pin on its expansion board | **WRONG — see row 4. Superseded, rail unused.** |
| 6 | Elevator node | GND | this board's GND, joined directly (not through the relay) | Still accurate — the Elevator's GND stays on the common ground rail even on dedicated USB power; only its 5V feed moved off this board entirely |
| 7 | FireAlarm rail relay | IN | GPIO 26 | same pattern as rows 2-6 |
| 8 | FireAlarm rail relay | VCC/GND/COM/NO | same as Elevator (rows 2-6), for FireAlarm | assumed same pattern |
| 9 | FireAlarm node | GND | this board's GND | assumed same pattern |
| 10 | Door rail relay | IN | GPIO 27 | same pattern |
| 11 | Door rail relay | VCC/GND/COM/NO | same as Elevator, for Door | assumed same pattern |
| 12 | Door node | GND | this board's GND | assumed same pattern |

Rows 8-9 and 11-12 are written as "same pattern" because Luis reported all three relays wired the same way; the individual per-relay wiring for FireAlarm and Door was not itemised separately.

## Power and ground

- GND is common across the PowerBox board and all three downstream nodes.
- **COM for all three relays is fed from this board's own VIN pin** — so all three downstream nodes draw through this ESP32's own 5V path and whatever USB source is behind it. That path's current limit is not known; no current has been measured on this build.
- Each downstream node's own USB cable is **unplugged** while its 5V pin is fed this way. Connecting both at once is unverified and avoided.

## Contact type

**NO (normally open)**, decided Oct 27, 2026. Relay de-energised = no power to the node; the coil must be actively energised (commanded "on") for the node to be powered. See `BSH_Systems/04_BSH-PowerBox.md` Notes for the consequences.

## Check before trusting this file (open questions)

- [ ] Pins per side on the DevKit, and its exact model
- [ ] Is relay VCC/GND actually on the same rail as COM (the VIN pin), or separate?
- [ ] The exact pin label on each node's expansion board that the NO wire lands on
- [ ] Relay contact rating and polarity confirmed per board (only one of the three was bench-tested with a multimeter, during the prototype phase)
- [ ] Current draw: each node, and the PowerBox board itself
- [ ] Voltage measured at a node's power pins with its rail commanded on (confirms power actually arrives, not just that the relay's coil is energised)
- [ ] Whether the DevKit's VIN path has a diode or fuse, and its limit

## Fritzing notes

Re-drawn Nov 15, 2026 in Fritzing (by Jeraldini): `BSH-PowerBox_Schematic_schematic.{pdf,png,svg}` shows the current power-rails design (3x relay rails on GPIO 25/26/27, matching `BSH_PowerBox_Node.ino` and this file's pin table) — not the old lights-and-fan wiring. The old `_Draft.pdf` (lights-and-fan) has been removed from this folder.

## Files that belong in this folder

- `BSH-PowerBox_Schematic.fzz` (not yet made for this design)
- `BSH-PowerBox_Schematic.pdf` and/or `.png` (the current PDF is the old design)
- This file
