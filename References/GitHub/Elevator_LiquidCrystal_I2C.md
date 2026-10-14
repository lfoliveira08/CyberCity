# LiquidCrystal_I2C

- **Link:** installed through the Arduino Library Manager; no upstream repo URL recorded in this project's docs
- **System:** Elevator (BSH) — **simulation only, as of Sep 25, 2026**
- **Status:** **Adopted, then superseded on the bench.** Still used by the Wokwi simulation; the real bench build uses an OLED and `Adafruit_SSD1306` instead (see `Elevator_Adafruit_SSD1306.md`)
- **License:** not yet recorded
- **Version installed:** not yet recorded
- **Checked:** not independently verified; written from `References/INDEX.md` and `Firmware/BSH-Elevator/SYSTEM_REPORT.md`

## What it is

The common Arduino library for driving a 16x2 character LCD over an I2C backpack
(typically at address 0x27).

## Why it is now simulation-only

BSH-Elevator's Phase A bench build substituted a 128x64 SSD1306 OLED for the
originally-documented 16x2 LCD — same SDA/SCL pins (21/22), different I2C address
(0x3C) and a different library. That was the user's preference, and the firmware's
call signature (`lcdPrint`) was kept unchanged so the swap did not ripple into the
state machine.

The Wokwi simulation (`Firmware/BSH-Elevator/Wokwi/`) still models the LCD with
`wokwi-lcd1602` and this library, because Wokwi has no SSD1306-equivalent set up
in that diagram yet. `SYSTEM_REPORT.md` Section 5 tracks that as an open
reconciliation item — the simulation and the bench currently disagree about the
display, and nothing else.

## Things to watch

- If a future build goes back to a real LCD: the address is 0x27 on most modules
  but 0x3F on some. Scan the bus rather than guessing.
- FH has no elevator any more (slot 1 became HVAC on Sep 26, 2026), so
  `FH_Sensors/03_I2C_LCD_Display.md` is annotated as unused. This library has no
  FH consumer at all now.
