# BSH-Elevator -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.11
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `BSH-Elevator/*/Wiring.md`; `BSH-Elevator/Wiring_Diagram.svg`; `BSH_Systems/01_BSH-Elevator.md`; `BSH-Elevator/BSH_Elevator_Node/BSH_Elevator_Node.ino`

## Parts

- ESP32 DevKit 30-pin
- L293D motor driver
- DC gearmotor DG01D-A130 (2 wire, 4.5V)
- External 5-6V motor supply
- 3 push buttons (floor 1/2/3)
- 16x2 I2C LCD with backpack (0x27)
- 3 debug LEDs (yellow, green, red) + 3x 220 ohm (optional)

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | Floor 1 button | leg A | GPIO 4 | INPUT_PULLUP |
| 2 | Floor 1 button | leg B | GND |  |
| 3 | Floor 2 button | leg A | GPIO 16 | INPUT_PULLUP |
| 4 | Floor 2 button | leg B | GND |  |
| 5 | Floor 3 button | leg A | GPIO 17 | INPUT_PULLUP |
| 6 | Floor 3 button | leg B | GND |  |
| 7 | I2C LCD backpack | SDA | GPIO 21 | Address 0x27 (some are 0x3F) |
| 8 | I2C LCD backpack | SCL | GPIO 22 |  |
| 9 | I2C LCD backpack | VCC | 5V (VIN) |  |
| 10 | I2C LCD backpack | GND | GND |  |
| 11 | L293D | ENABLE (1,2EN, pin 1) | GPIO 18 | PWM speed |
| 12 | L293D | DIRA (1A, pin 2) | GPIO 19 | Pin numbers assume a bare L293D DIP-16; see checks |
| 13 | L293D | DIRB (2A, pin 7) | GPIO 5 | Strapping pin |
| 14 | L293D | Vcc1 (pin 16) | 5V (VIN) | Logic supply; not stated in the docs, standard for the IC |
| 15 | L293D | Vcc2 (pin 8) | External 5-6V motor supply + | Motor power only, never from the ESP32 |
| 16 | L293D | GND (pins 4,5,12,13) | Common GND | Supply GND, ESP32 GND and L293D GND all tied |
| 17 | L293D | 1Y (pin 3) and 2Y (pin 6) | DC motor, 2 wires | Motor output pins are not written in any doc; standard single-motor hookup |
| 18 | Debug LED yellow | anode via 220 ohm | GPIO 23 | Mirrors ENABLE; cathode to GND |
| 19 | Debug LED green | anode via 220 ohm | GPIO 25 | Mirrors DIRA; cathode to GND |
| 20 | Debug LED red | anode via 220 ohm | GPIO 27 | Mirrors DIRB; cathode to GND |

## Power and ground

- Common ground between ESP32, L293D and the external motor supply is required.
- 5V (VIN) feeds the LCD and L293D logic. The motor supply is separate.

## Check before drawing (open questions found in the docs)

- [ ] Is the L293D a bare DIP-16 chip or a breakout module? The docs only list ENABLE, DIRA, DIRB, GND. If it is a module, rename the pins to its silkscreen.
- [ ] Which L293D outputs the motor is on (1Y/2Y assumed) and whether Vcc1 goes to 5V. Confirm on the bench.
- [ ] Motor supply source (3xAA, bench supply, regulator) and voltage: pick what is actually used (4.5V is the motor's rating).
- [ ] The debug LEDs are optional; delete them from the drawing if they are not on the bench build.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.
- The core library has an L293D IC and a DC motor part, and a 16x2 I2C LCD in several variants. Choose the L293D IC if yours is bare.

## Files that belong in this folder

- `BSH-Elevator_Schematic.fzz` (the Fritzing sketch)
- `BSH-Elevator_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
