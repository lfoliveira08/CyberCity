# RGB LED (status) -- FH-HVAC

Reference doc: `FH_Sensors/05_RGB_LED.md`

(2026-10-07: briefly switched to two separate red/green LEDs, then reverted -- an RGB LED turned
out to be available after all.)

## What it does here
Shows node state: **green** idle, **blue** cooling, **yellow** (red+green) disabled via
`HVAC_ENABLE`, **red** latched `HVAC_FAULT`.

## Wiring (common cathode)
| LED leg | ESP32 | Notes |
|---|---|---|
| R | GPIO 16 via 220 ohm | |
| G | GPIO 17 via 220 ohm | |
| B | GPIO 18 via 220 ohm | |
| Common (longest leg) | GND | If the LED is common ANODE, wire common to 3V3 and invert the logic in `setRGB()` |

GPIO 16/17 are free on a plain ESP32 DevKit. They are used by PSRAM on WROVER modules; use a WROOM DevKit.

## Standalone test checklist (BenchTest.ino)
- [ ] Red, green, blue each light alone
- [ ] Yellow shows when red+green are on
- [ ] Colours match the legend above (if red/green look swapped, a leg is on the wrong pin)
- [ ] Confirmed common cathode (everything lit at once = common anode)
