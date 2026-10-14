# L293D Motor Driver — Build Log (BSH-Elevator)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-09-23
- [x] Component in hand (driver + DG01D-A130 motor + external power supply)
- [x] Wired per `Wiring.md`
- [x] Bench-tested standalone (see checklist in `Wiring.md`)
- [x] Confirmed working with the real `BSH_Elevator_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-09-23 | Wokwi simulation run and confirmed working (LED stand-ins for ENABLE/DIRA/DIRB). |
| 2026-09-24 | Confirmed motor in hand is a DG01D-A130 48:1 gearmotor — 2-wire, matches the design with zero conflict. Real-world reference photo of L293D wiring pulled up for the user to follow (lastminuteengineers.com tutorial). |
| 2026-09-25 | Wired debug LEDs, fixed a loose LED jumper. Motor initially did not spin — isolated with the standard checklist (test motor alone, common ground, chip orientation, voltage at the chip, OUT1/OUT2 seating) — see Lessons Learned. Once fixed, confirmed motor spins both directions correctly in the combined Phase A integration sketch, reversing direction between up/down travel as designed. |

## Lessons Learned
- Motor doesn't spin but everything else looks right (debug LEDs toggle correctly) is a *wiring/power* symptom, not a code one — the debug LEDs are wired straight to the ESP32 pins, not through the L293D, so they only prove the code's signal generation is correct; they say nothing about whether the L293D itself has power. The real culprit list, in order of likelihood: missing common ground between the external motor supply and ESP32/L293D GND, chip inserted backwards (pin 1 orientation), voltage not actually reaching pin 8 (VS) despite the supply being on, and OUT1/OUT2 wires not fully seated.
- The DG01D-A130's two motor leads are NOT polarized like a battery (no inherent "red/positive, black/negative") — either lead can go to OUT1 or OUT2. If the motor spins the wrong direction, just swap the two leads (or swap which constant maps to which output in firmware) — no damage risk either way.
- Confirmed on the bench: reversing DIRA/DIRB between floor-up and floor-down travel does correctly reverse the physical motor direction, as the design assumed.
- A stray `*/` sequence inside a C-style block comment (in a file path like `Firmware/BSH-Elevator/*/Wiring.md`) silently closes the comment early and corrupts everything after it until the next real `*/` — cascades into confusing, seemingly-unrelated compiler errors far from the actual mistake. Worth remembering for any future header comments that reference wildcarded paths.
- Non-ASCII characters (em dashes, etc.) in `.ino` comments can fail to compile on some Windows Arduino IDE setups ("extended character is not valid in an identifier") if the file isn't read as UTF-8 — stick to plain ASCII in Arduino source comments.

## Troubleshooting Tips
- "Motor doesn't move at all, but debug LEDs work correctly" — check common ground first (external motor supply's GND tied to the same rail as ESP32/L293D GND pins 4/5/12/13), then chip orientation, then measure voltage at pin 8 directly on the chip (not at the battery).
- "Motor spins the wrong direction" — swap the two motor leads on OUT1/OUT2, or swap the DIRA/DIRB pin assignment in firmware. Either works, no damage risk.
- If your specific ESP32 board/shield reserves GPIO 5/18/19 for something else (e.g. default VSPI pins), those need to move — but on this bench they were kept as the original 18/19/5 per the Elegoo DC-motor tutorial being followed.
