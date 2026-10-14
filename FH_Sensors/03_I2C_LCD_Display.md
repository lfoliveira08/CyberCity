# I2C LCD Display (16x2)

> **NOT USED IN FOUNDER'S HALL as of Sep 26, 2026.** FH slot 1 changed from
> Elevator to an HVAC zone unit, and the elevator was this status display's only
> FH consumer. Kept, not deleted, for two reasons: its BSH mirror
> (`BSH_03_I2C_LCD_Display.md`) is still live and in
> use on BSH-Elevator, and the specs/wiring here would otherwise have to be
> re-researched if FH ever regains an elevator. Same "annotate, don't delete"
> treatment the retired MQ-135 sheet got on Sep 14. See
> `FH_Systems/01_FH-HVAC.md`.

**Used in:** nothing, currently — the FH-Elevator node it served was retired on
Sep 26, 2026 (see the banner above). Note that even on BSH the bench build
substituted an SSD1306 OLED for this LCD (Sep 25, 2026), so the part is now used
only by the Wokwi simulation.

## Function
Shows elevator status text — current floor, "doors closing," "arrived," etc.

## Key Specs
- 16x2 character LCD with I2C backpack (PCF8574-style)
- Typical I2C address: `0x27`
- Backpack usually regulates/level-shifts for you; confirm your specific module is fine with 3.3V I2C logic from the ESP32 (most are)

## Wiring / Pins (retired FH-Elevator ESP32 — reference only)
| Signal | GPIO |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 5V |
| GND | Common ground |

## Modbus Register Tie-In
None directly — this is a local display only. It mirrors the same floor number the hub separately reads via Input Register `CURRENT_FLOOR`, so the physical display and the dashboard should always agree.

## Notes
If you ever add another I2C device to this same board, double-check it doesn't also default to address `0x27` — that would collide with the LCD backpack.

This doc mirrors `BSH_Sensors/03_I2C_LCD_Display.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-lcd1602` with the attribute `"pins": "i2c"`

**How to use it:** Connect SDA→GPIO 21, SCL→GPIO 22, VCC, GND. Default I2C address is 0x27, same as most real backpacks. Uses the same `LiquidCrystal_I2C` library (add it to `libraries.txt`).

**What the simulation hides:** Real backpacks are sometimes 0x3F, and the contrast potentiometer on the back can make a working display look blank. The simulation has neither problem.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
