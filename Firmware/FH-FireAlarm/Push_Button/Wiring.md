# Push Button (Pull Station) — FH-FireAlarm

> **Founder's Hall mirror.** Identical component and identical wiring to
> BSH-FireAlarm — this file was copied from
> `Firmware/BSH-FireAlarm/Push_Button/Wiring.md` on Sep 26, 2026 and re-pointed at FH's
> own docs and IP (192.168.50.22). **Nothing here has been verified on FH
> hardware yet** — FH systems are Phase 2, and Phase 2 has not started. If a
> correction is found while building either building, fix both copies.


Reference doc: `FH_Sensors/02_Push_Button.md`

## What it does here
Stands in for a manual fire-alarm pull station — the "authorized/expected"
local trigger source, contrasted against a remote/hub-issued
`ALARM_ACTIVATE` command with no matching button press (the false-alarm
injection demo, Scenario 4 in the workflow doc).

## Key specs
- Standard momentary push button
- Wired `INPUT_PULLUP`, active LOW (pressed = reads LOW)
- ~50ms software debounce (already in the firmware, no hardware debounce
  capacitor needed)

## Wiring
| Button leg | ESP32 pin |
|---|---|
| Leg 1 | GPIO 4 |
| Leg 2 | GND |

No external resistor needed — `INPUT_PULLUP` is set in firmware, and GPIO 4
has a working internal pull-up (unlike GPIO 34-39, see the note in the Door
node's Reed Switch build folder once you get to that system).

## Firmware tie-in
`FH_FireAlarm_Node.ino`: `PIN_PULL_BUTTON = 4`, read with
`digitalRead(PIN_PULL_BUTTON) == LOW`, debounced over `DEBOUNCE_MS = 50`,
published to Modbus Discrete Input `MANUAL_PULL_TRIGGERED` (DI0).

## Bench-test checklist (Phase A, this component only)
- [ ] Button wired per the table above
- [ ] Open Serial Monitor at 115200 baud with the real firmware flashed
- [ ] Press and hold — buzzer sounds, RGB LED blinks red (once those two
      are also wired) or watch `MANUAL_PULL_TRIGGERED` flip in the Serial
      output / a Modbus client if you're at Phase B already
- [ ] Release — state clears back to normal

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "button always reads pressed" -> check it's
wired to GND, not accidentally to 3.3V)*
