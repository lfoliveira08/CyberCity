# FH-HVAC -- connection list (source for the Fritzing schematic)

Written 2026-10-12 from the files below. **Draft: not yet drawn in Fritzing.** Where this list and the bench disagree, the bench wins; fix this file.

- **Node IP:** 192.168.50.21
- **Board:** ESP32 DevKit, 30-pin (DOIT style), WROOM
- **Sources:** `FH-HVAC/DHT11_Sensor/Wiring.md`; `FH-HVAC/Relay_Module/Wiring.md`; `FH-HVAC/RGB_LED/Wiring.md`; `FH_Systems/01_FH-HVAC.md (bench-confirmed pin map)`

## Parts

- ESP32 DevKit 30-pin
- DHT11 temperature/humidity module (3 pin)
- Relay module, 1 channel
- RGB LED, common cathode + 3x 220 ohm
- Small DC fan on its own supply (optional)

## Connections

| # | Part | Part pin | Connects to | Notes |
|---|---|---|---|---|
| 1 | DHT11 module | VCC / + | 3V3 |  |
| 2 | DHT11 module | DATA / OUT / S | GPIO 4 | Module has its own pull-up; do not add one |
| 3 | DHT11 module | GND / - | GND | Read the silkscreen: pin order varies by brand |
| 4 | Relay module | IN | GPIO 26 |  |
| 5 | Relay module | VCC | 5V (VIN) |  |
| 6 | Relay module | GND | GND |  |
| 7 | Relay module | COM / NO | Fan load | Fan on its own supply, grounds common |
| 8 | RGB LED (common cathode) | R | 220 ohm, then GPIO 16 |  |
| 9 | RGB LED (common cathode) | G | 220 ohm, then GPIO 17 |  |
| 10 | RGB LED (common cathode) | B | 220 ohm, then GPIO 18 |  |
| 11 | RGB LED (common cathode) | common cathode | GND | Longest leg |

## Power and ground

- GND common to ESP32, relay, DHT11, RGB LED and the fan supply.
- Never power the fan from the ESP32 3V3.

## Check before drawing (open questions found in the docs)

- [ ] `FH_Systems/01_FH-HVAC.md` and its Components list say DHT22 (needs an external 10k pull-up); the bench part and firmware are a **DHT11 3-pin module** with its own pull-up. This list follows the bench. Draw the DHT11 module, no external resistor.
- [ ] Fan supply: voltage and source not recorded. Draw it as a labeled external supply.
- [ ] Relay polarity for this board is not stated here; check `Relay_Module/Build_Log.md` and note it on the schematic.

## Fritzing notes

- Draw the real schematic in **Schematic view** (not just Breadboard): turn every dashed ratsnest line into a wire, label the nets (GND, 3V3, 5V) and the GPIO numbers.
- Use the 30-pin ESP32 DevKit part. Check its pin labels match the physical board before wiring.
- Modules with no core Fritzing part (flame, gas, IR, RC522, relay boards, DHT11 breakout) can be a labeled pin header; name each header pin exactly as the silkscreen on the real module.
- GPIO 16/17 are free only on WROOM boards (WROVER uses them for PSRAM).

## Files that belong in this folder

- `FH-HVAC_Schematic.fzz` (the Fritzing sketch)
- `FH-HVAC_Schematic.pdf` and/or `.png` (exported Schematic view)
- This file
