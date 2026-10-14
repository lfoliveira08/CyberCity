# I2C LCD Display (16x2) — BSH-Elevator

Reference doc: `BSH_Sensors/03_I2C_LCD_Display.md`

> **Hardware substitution (2026-09-25):** the real bench build uses a
> 128x64 I2C SSD1306 OLED instead of the 16x2 character LCD documented
> below. Folder/doc kept as-is since it's still "the I2C status display"
> component and the role/firmware call signature (`lcdPrint(line1, line2)`)
> is unchanged. What's different: I2C address is **0x3C** (not 0x27),
> library is `Adafruit_SSD1306` + `Adafruit_GFX` (not `LiquidCrystal_I2C`),
> and text is drawn pixel-by-pixel (`setTextSize(1)`, cursor in pixels) not
> character-cell. SDA/SCL pins (21/22) and the rest of this page are
> unchanged and still accurate. See `Build_Log.md` for what was actually
> confirmed on the bench, and `Firmware/BSH-Elevator/SYSTEM_REPORT.md` for
> the full writeup.

## What it does here
Shows elevator status text — "Elevator Ready", "Going up...", "Arrived!",
etc. Local display only, not exposed as a Modbus register itself (it just
mirrors the same floor number the hub separately reads via `CURRENT_FLOOR`).

## Key specs
- 16x2 character LCD with I2C backpack (PCF8574-style)
- Typical I2C address: `0x27` — if you ever add a second I2C device to
  this board, double-check it doesn't also default to `0x27`
- Backpack usually regulates/level-shifts for you; confirm your specific
  module is fine with 3.3V I2C logic from the ESP32 (most are)

## Wiring
| LCD backpack pin | ESP32 pin |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 5V |
| GND | Common ground |

## Firmware tie-in
`BSH_Elevator_Node.ino`: `Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL)` with
`PIN_LCD_SDA/SCL = 21/22`, `LiquidCrystal_I2C lcd(0x27, 16, 2)`. Updated via
the `lcdPrint(line1, line2)` helper at each state-machine transition (not
every loop iteration — I2C writes are quick but there's no reason to spam
them).

## Bench-test checklist (Phase A, this component only)
- [ ] LCD wired per the table above
- [ ] Flash the real firmware — LCD should show "Elevator Ready" / "Floor
      1" at boot
- [ ] If the LCD stays blank, check the I2C address first (some backpacks
      ship as `0x3F` instead of `0x27` — an I2C scanner sketch will tell
      you which)
- [ ] Adjust the backpack's contrast potentiometer if text is invisible
      but the backlight is on

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "blank/garbled text" -> wrong I2C address or
contrast pot, not a firmware bug)*
