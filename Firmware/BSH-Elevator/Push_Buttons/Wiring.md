# Push Buttons (3x Floor Call) — BSH-Elevator

Reference doc: `BSH_Sensors/02_Push_Button.md`

## What it does here
Three momentary buttons let a person locally call the elevator to floor 1,
2, or 3 — the local/authorized counterpart to a remote `CALL_FLOOR_1/2/3`
Modbus coil write.

## Key specs
- Standard momentary push buttons, `INPUT_PULLUP`, active LOW
- ~50ms software debounce (already in firmware)

## Wiring
| Button | ESP32 pin | Other leg |
|---|---|---|
| Floor 1 | GPIO 4 | GND |
| Floor 2 | GPIO 16 | GND |
| Floor 3 | GPIO 17 | GND |

No external resistors needed — `INPUT_PULLUP` is set in firmware, and all
three of these pins have working internal pull-ups (unlike GPIO 34-39;
see the Door node's Reed Switch notes once you build that system).

## Firmware tie-in
`BSH_Elevator_Node.ino`: `PIN_BTN_FLOOR_1/2/3 = 4/16/17`, each debounced
independently (`btn1`/`btn2`/`btn3` structs), and a rising-edge press calls
`requestFloor(N)` — the same entry point a remote `CALL_FLOOR_N` coil write
uses.

## Bench-test checklist (Phase A, this component only)
- [ ] All 3 buttons wired per the table above
- [ ] Flash the real firmware, open Serial Monitor at 115200 baud
- [ ] Press each button once (elevator starts on Floor 1, so pressing
      Floor 1 again should just print "Already on Floor 1" on the LCD once
      that's wired) — confirm each button reaches `requestFloor()` in the
      Serial log
- [ ] Confirm debounce is working — a single press shouldn't trigger
      multiple floor requests

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "button does nothing" -> check it's wired to
GND, not 3.3V, same as the Fire Alarm pull-station button)*
