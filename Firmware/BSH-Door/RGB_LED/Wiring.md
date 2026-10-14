# RGB LED — BSH-Door

Reference doc: `BSH_Sensors/05_RGB_LED.md`

## What it does here
Local visual status only: green = gate closed and idle, red = gate open or
moving. This node deliberately does **not** try to show "authorized vs.
suspicious" locally — that judgment happens at the SCADA layer (see the
DESIGN NOTE at the top of `BSH_Door_Node.ino` and this system's README).

## Key specs
- Common-cathode RGB LED (or 3 separate LEDs)
- One 220Ω current-limiting resistor per color channel
- Same wiring convention as the Fire Alarm node's RGB LED — if you've
  already built that one, this is a repeat of the same skill

## Wiring
| LED pin | Through | ESP32 pin |
|---|---|---|
| Red anode/leg | 220Ω resistor | GPIO 25 |
| Green anode/leg | 220Ω resistor | GPIO 26 |
| Blue anode/leg | 220Ω resistor | GPIO 27 (wired for BOM consistency with Fire Alarm's RGB LED; this node's firmware never turns the blue channel on) |
| Common cathode | — | GND |

## Firmware tie-in
`BSH_Door_Node.ino` (Phase A, Oct 9, 2026): `PIN_RGB_RED/GREEN/BLUE =
25/26/27`, driven by `showClosed()` (green) and `showOpen()` (red); blue is
always LOW. In the Phase A sketch red also shows for 1 s after a denied card.
The Phase B sketch drives the LED from the gate state instead
(`setRgb(!secureClosed, secureClosed, false)` with `secureClosed = (gateState
== GATE_CLOSED)`). There is no buzzer on this node.

## Bench-test checklist (Phase A, this component only)
- [ ] LED + resistors wired per the table above
- [ ] Power up with the real firmware — LED should be steady green at boot
      (gate starts closed)
- [ ] Once the servo is also wired, trigger a gate open (badge a card, or
      briefly short the servo test) — LED should switch to red while open/
      moving, back to green once fully closed again

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — same failure modes as the Fire Alarm RGB LED: wrong
color -> check R/G/B legs; dim -> check resistor value)*
