# FH-Door -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.23
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `FH-Door/*/Wiring.md`; `FH_Systems/03_FH-Door.md`; `FH-Door/FH_Door_Node/FH_Door_Node.ino`

## Parts

- ESP32 DevKit 30-pin
- SG90 servo (gate arm)
- RGB LED, common cathode
- 3x 220 ohm resistors
- IR obstacle sensor
- Reed switch + 10k ohm pull-up resistor
- RC522 RFID reader (SPI)
- External 5V supply for the servo

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | SG90 servo | signal (orange) | GPIO 13 |  |
| 2 | SG90 servo | VCC (red) | External 5V supply | Not from the ESP32 |
| 3 | SG90 servo | GND (brown) | Common GND | External supply GND tied to ESP32 GND |
| 4 | RGB LED (common cathode) | R | 220 ohm, then GPIO 25 |  |
| 5 | RGB LED (common cathode) | G | 220 ohm, then GPIO 26 |  |
| 6 | RGB LED (common cathode) | B | 220 ohm, then GPIO 27 | Wired, never driven by firmware |
| 7 | RGB LED (common cathode) | common cathode | GND |  |
| 8 | IR obstacle sensor | signal (OUT) | GPIO 14 |  |
| 9 | IR obstacle sensor | VCC | 3V3 |  |
| 10 | IR obstacle sensor | GND | GND |  |
| 11 | Reed switch | leg 1 | GPIO 34 and one end of 10k ohm pull-up | GPIO 34 has no internal pull: external resistor required |
| 12 | 10k ohm pull-up | other end | 3V3 |  |
| 13 | Reed switch | leg 2 | GND |  |
| 14 | RC522 RFID reader | SCK | GPIO 18 | VSPI default |
| 15 | RC522 RFID reader | MISO | GPIO 19 | VSPI default |
| 16 | RC522 RFID reader | MOSI | GPIO 23 | VSPI default |
| 17 | RC522 RFID reader | SDA / SS | GPIO 32 | Chip select |
| 18 | RC522 RFID reader | RST | GPIO 33 |  |
| 19 | RC522 RFID reader | 3.3V | 3V3 | 3.3V only, never 5V |
| 20 | RC522 RFID reader | GND | GND | IRQ pin not connected |

## Power and ground

- GND is common to ESP32, servo supply, and every part.
- 3V3 feeds IR sensor, RC522 and the reed pull-up.
- Servo power comes only from the external 5V supply.

## Check before drawing (open questions found in the docs)

- [ ] Reed switch pull-up: `Wiring.md` says 10k ohm to 3V3; the firmware comment says module or 10k. Draw the 10k resistor unless your reed module has its own.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.
- Fritzing core has an SG90-style servo and a basic reed switch; if not, use a 3-pin header (servo) and a 2-pin header (reed).

## Files that belong in this folder

- `FH-Door_Schematic.fzz` (the Fritzing sketch)
- `FH-Door_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
