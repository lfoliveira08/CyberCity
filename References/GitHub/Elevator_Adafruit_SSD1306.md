# Adafruit SSD1306 + Adafruit GFX

- **Link:** installed through the Arduino Library Manager (Adafruit publishes both on GitHub; no URL recorded in this project's docs yet)
- **System:** Elevator (BSH) — the bench status display
- **Status:** **Adopted on the bench, Sep 25, 2026.** This is the display library the real, working elevator firmware uses
- **License:** not yet recorded
- **Version installed:** not yet recorded
- **Checked:** not independently verified; written from `Firmware/BSH-Elevator/SYSTEM_REPORT.md` and the sketch itself

## What it is

Adafruit's driver for SSD1306 OLED displays, plus the `Adafruit_GFX` graphics
library it depends on. Both must be installed — installing only SSD1306 fails to
compile.

## Why it is in the project

It was not in any plan. BSH-Elevator's bring-up substituted a 128x64 I2C SSD1306
OLED for the documented 16x2 LCD, so this pair replaced `LiquidCrystal_I2C` on the
bench. It is recorded here because it is now load-bearing for a system that has
passed Phase A, and it was missing from `References/INDEX.md` until Sep 26, 2026.

## What we use it for

Two lines of status text in `BSH_Elevator_Node.ino`'s `lcdPrint()` helper —
current floor, and what the state machine is doing (door closing / moving /
arrived / fault).

## Things to watch — both learned on the bench

- **Address:** the sketch assumes 0x3C, the most common. Some modules ship at
  0x3D; a blank screen is the symptom.
- **Text size:** `setTextSize(2)` looked fine in isolation but wrapped and
  collided the two status lines for longer strings such as "Elevator Ready".
  Both lines use `setTextSize(1)` (about 21 characters at 128px wide), at y=0
  and y=16.
- **Size constant:** the sketch is written for 128x64. For a 128x32 module,
  change `SCREEN_HEIGHT`.
