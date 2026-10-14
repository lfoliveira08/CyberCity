# FH-PowerBox -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.24
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `FH-PowerBox/Lights_Relay/Wiring.md`; `FH-PowerBox/Fan_Relay/Wiring.md`; `FH_Systems/04_FH-PowerBox.md`; `FH-PowerBox/FH_PowerBox_Node/FH_PowerBox_Node.ino`

## Parts

- ESP32 DevKit 30-pin
- Relay module, 1 channel (lights)
- Relay module, 1 channel (fan)
- 3 LEDs + 3 series resistors (lights load)
- Small DC fan motor
- 2 push buttons (local overrides)

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | Lights relay module (1 ch) | IN | GPIO 13 | Active-HIGH (confirmed Oct 5, 2026) |
| 2 | Lights relay module (1 ch) | VCC | 5V (VIN) | Own module, not shared with fan relay |
| 3 | Lights relay module (1 ch) | GND | GND |  |
| 4 | Lights relay module (1 ch) | COM / NO | 3 LED branches in parallel | Each LED has its own series resistor; value not recorded |
| 5 | Fan relay module (1 ch) | IN | GPIO 15 | Strapping pin; see checks |
| 6 | Fan relay module (1 ch) | VCC | 5V (VIN) |  |
| 7 | Fan relay module (1 ch) | GND | GND |  |
| 8 | Fan relay module (1 ch) | COM | 5V rail | Fan on its own power rail |
| 9 | Fan relay module (1 ch) | NO | DC fan motor + |  |
| 10 | DC fan motor | - | GND rail |  |
| 11 | Lights override button | leg 1 | GPIO 4 | firmware pinMode: INPUT_PULLUP |
| 12 | Lights override button | leg 2 | GND | Assumed, see checks |
| 13 | Fan override button | leg 1 | GPIO 16 | firmware pinMode: INPUT_PULLUP |
| 14 | Fan override button | leg 2 | GND | Assumed, see checks |

## Power and ground

- GND is common to everything, including the fan rail ground.
- 5V (VIN) powers both relay module coils and is the fan's COM supply.
- What supplies the lights LED branch (3V3, 5V or its own rail) is not recorded; see checks.

## Check before drawing (open questions found in the docs)

- [ ] Series resistor value for each of the 3 lights LEDs, and the rail feeding them via COM. Not recorded.
- [ ] FH-PowerBox has no SYSTEM_REPORT.md and no button Wiring.md: confirm button wiring on the bench (pinMode shown above).
- [ ] GPIO 15 (fan relay IN) is a strapping pin; keep the note on the schematic.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.
- Relay modules: use a generic relay-module part or a 3-pin header (IN, VCC, GND) plus a 3-pin screw terminal (NO, COM, NC).

## Files that belong in this folder

- `FH-PowerBox_Schematic.fzz` (the Fritzing sketch)
- `FH-PowerBox_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
