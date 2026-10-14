# RGB LED

**Used in:**
- BSH-FireAlarm node (blinking red during an alarm)
- BSH-Door node (green = clear, red = alert/intrusion)

## Function
Visual status indicator. On the fire alarm node it blinks red during an active alarm; on the door node it shows the gate's current security state.

## Key Specs
- Common-cathode RGB LED (or 3 separate LEDs)
- Current-limiting resistor (~220Ω) on each color channel
- PWM-capable pins recommended for blink/fade effects

## Wiring / Pins
| Node | Channel | GPIO |
|---|---|---|
| BSH-FireAlarm | Red | GPIO 25 |
| BSH-FireAlarm | Green | GPIO 26 |
| BSH-FireAlarm | Blue | GPIO 27 |
| BSH-Door | Red | GPIO 25 |
| BSH-Door | Green | GPIO 26 |
| BSH-Door | Blue | GPIO 27 |

This is the same pin convention used on your IRONCLAD crossing-gate RGB LED — same board role, same wiring.

## Modbus Register Tie-In
- FireAlarm node: follows the same alarm state as the buzzer (`ALARM_ACTIVATE` / `MANUAL_PULL_TRIGGERED`)
- Door node: reflects `DOOR_POSITION` / `IR_DETECTED` state and responds to `DOOR_OPEN_CMD` / `DOOR_CLOSE_CMD`

## Notes
Because this matches your IRONCLAD gate wiring exactly, you can likely reuse a good chunk of that gate's RGB-blink firmware logic directly for the BSH-Door node.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-rgb-led` (set `"common": "cathode"`; Wokwi defaults to anode)

**How to use it:** R/G/B each through a 220Ω `wokwi-resistor` to its GPIO, COM to GND — the same wiring as the real bench, so the simulation doubles as a wiring diagram.

**What the simulation hides:** Wokwi will light the LED even without resistors. The real LED (and GPIO) can be damaged without them.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
