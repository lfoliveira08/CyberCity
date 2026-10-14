# I2C LCD Display — Build Log (BSH-Elevator)

**Hardware substitution:** this bench build uses a 128x64 I2C SSD1306 OLED
instead of the documented 16x2 character LCD — folder/reference name kept
as-is for now since it's still "the I2C status display" component, but see
the substitution note at the top of `Wiring.md` for the actual part and
wiring differences (different I2C address, different library, pixel-based
text instead of character-cell).

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-09-23 (simulated as the 16x2 LCD — the Wokwi sim was not updated for the OLED substitution; see `Firmware/BSH-Elevator/SYSTEM_REPORT.md` Section 6)
- [x] Component in hand (OLED, not the originally-documented LCD)
- [x] Wired per `Wiring.md`
- [x] I2C address confirmed — **0x3C** (SSD1306 default), not the LCD's 0x27
- [x] Bench-tested standalone (see checklist in `Wiring.md`)
- [x] Confirmed working with the real `BSH_Elevator_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-09-23 | Wokwi simulation run and confirmed working, using the originally-planned 16x2 LCD (`LiquidCrystal_I2C`). |
| 2026-09-25 | Decided to use the I2C OLED instead of the 16x2 LCD for the real bench. Firmware swapped to `Adafruit_SSD1306`/`Adafruit_GFX`, address 0x3C, same SDA/SCL pins (21/22). Confirmed working in the combined Phase A integration sketch. |

## Lessons Learned
- The OLED's higher resolution (128x64) means text needs to be sized deliberately — `setTextSize(2)` looked good in isolation but wrapped and overlapped for longer status strings like "Elevator Ready" or "Moving to 3". Settled on `setTextSize(1)` for both lines (max ~21 chars/line at 128px wide), with lines at y=0 and y=16, which fits every message this firmware prints without wrapping.
- This is a straight library/wiring swap from the documented LCD, not a firmware logic change — `lcdPrint(line1, line2)` keeps the same two-line-status call signature the rest of the state machine uses, only its internals changed.

## Troubleshooting Tips
- Screen blank at boot: try I2C address 0x3D instead of 0x3C — some SSD1306 boards ship at the alternate address. The firmware halts with a Serial message telling you to check this if `display.begin()` fails.
- Garbled/overlapping text: check you're on `setTextSize(1)` for both lines, not `setTextSize(2)` for the first line — size-2 text overflows the 128px width for most of this firmware's status strings and wraps into the second line.
