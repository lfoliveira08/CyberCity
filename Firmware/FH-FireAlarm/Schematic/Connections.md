# FH-FireAlarm -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.22
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `FH-FireAlarm/*/Wiring.md`; `FH_Systems/02_FH-FireAlarm.md`; `FH-FireAlarm/FH_FireAlarm_Node/FH_FireAlarm_Node.ino`

## Parts

- ESP32 DevKit 30-pin
- Active buzzer
- RGB LED, common cathode
- 3x 220 ohm resistors
- Push button (pull station)
- Flame sensor module (3 pin, D0)
- Gas sensor module (confirm MQ-2 or MQ-9) (4 pin: AO, DO, GND, VCC)

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
| 12 | Gas sensor module (confirm MQ-2 or MQ-9) | AO | GPIO 34 | ADC1, input-only. Divider 1k/2k may be needed; see checks |
| 13 | Gas sensor module (confirm MQ-2 or MQ-9) | DO | GPIO 19 | Wired for future use, firmware does not read it |
| 14 | Gas sensor module (confirm MQ-2 or MQ-9) | VCC | VIN (5V) | Heater needs 5V |
| 15 | Gas sensor module (confirm MQ-2 or MQ-9) | GND | GND |  |

## Power and ground

- GND is common to every part.
- 5V (VIN) feeds only the gas sensor heater.
- 3V3 feeds the flame sensor only if its module is rated for it (see checks).

## Check before drawing (open questions found in the docs)

- [ ] Measure the gas sensor AO with a multimeter (powered from 5V, clean air). If above 3.3V, add 1k ohm from AO to GPIO 34 and 2k ohm from GPIO 34 to GND, and draw it. Status in the docs: wired, check not confirmed.
- [ ] Flame sensor VCC: 3V3 or 5V? The docs say to check the module. Draw what is on the bench.
- [ ] Which gas sensor is on FH-FireAlarm: MQ-2 or MQ-9? The BSH node has an MQ-9; the FH docs are copies of the BSH sheet and do not say.
- [ ] FH pull button: `FH_FireAlarm_Node.ino` comment says plain INPUT with a Keyestudio module that has its own resistor, but `Push_Button/Wiring.md` says bare button on INPUT_PULLUP. Which is on the bench? If it is a module, draw 3 pins (S, VCC, GND) instead of leg 1 / leg 2.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.

## Files that belong in this folder

- `FH-FireAlarm_Schematic.fzz` (the Fritzing sketch)
- `FH-FireAlarm_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
