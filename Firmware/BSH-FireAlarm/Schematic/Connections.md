# BSH-FireAlarm -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.12
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `BSH-FireAlarm/*/Wiring.md`; `BSH_Systems/02_BSH-FireAlarm.md`; `BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`

## Parts

- ESP32 DevKit 30-pin
- Active buzzer
- RGB LED, common cathode
- 3x 220 ohm resistors
- Push button (pull station)
- Flame sensor module (3 pin, D0)
- Gas sensor module (MQ-9 on the bench; folder is named MQ-2) (4 pin: AO, DO, GND, VCC)

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | Active buzzer | + (signal) | GPIO 23 | Digital drive, HIGH = on |
| 2 | Active buzzer | -  | GND |  |
| 3 | RGB LED (common cathode) | R | 220 ohm, then GPIO 25 |  |
| 4 | RGB LED (common cathode) | G | 220 ohm, then GPIO 26 |  |
| 5 | RGB LED (common cathode) | B | 220 ohm, then GPIO 27 |  |
| 6 | RGB LED (common cathode) | common cathode | GND |  |
| 7 | Pull-station push button | leg 1 | GPIO 4 | INPUT_PULLUP in firmware, active LOW |
| 8 | Pull-station push button | leg 2 | GND | No external resistor |
| 9 | Flame sensor module (LM393) | D0 | GPIO 18 | Active LOW, plain INPUT |
| 10 | Flame sensor module (LM393) | VCC | 3V3 or 5V | Module rating not recorded; see checks |
| 11 | Flame sensor module (LM393) | GND | GND |  |
| 12 | Gas sensor module (MQ-9 on the bench; folder is named MQ-2) | AO | GPIO 34 | ADC1, input-only. Divider 1k/2k may be needed; see checks |
| 13 | Gas sensor module (MQ-9 on the bench; folder is named MQ-2) | DO | GPIO 19 | Wired for future use, firmware does not read it |
| 14 | Gas sensor module (MQ-9 on the bench; folder is named MQ-2) | VCC | VIN (5V) | Heater needs 5V |
| 15 | Gas sensor module (MQ-9 on the bench; folder is named MQ-2) | GND | GND |  |

## Power and ground

- GND is common to every part.
- 5V (VIN) feeds only the gas sensor heater.
- 3V3 feeds the flame sensor only if its module is rated for it (see checks).

## Check before drawing (open questions found in the docs)

- [ ] Measure the gas sensor AO with a multimeter (powered from 5V, clean air). If above 3.3V, add 1k ohm from AO to GPIO 34 and 2k ohm from GPIO 34 to GND, and draw it. Status in the docs: wired, check not confirmed.
- [ ] Flame sensor VCC: 3V3 or 5V? The docs say to check the module. Draw what is on the bench.
- [ ] System doc `BSH_Systems/02_BSH-FireAlarm.md` says MQ-2 and omits GPIO 19; the bench part is an MQ-9 and DO is wired to GPIO 19. This list follows the bench.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.

## Files that belong in this folder

- `BSH-FireAlarm_Schematic.fzz` (the Fritzing sketch)
- `BSH-FireAlarm_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
