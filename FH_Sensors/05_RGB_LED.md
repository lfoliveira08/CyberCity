# RGB LED

**Used in:**
- FH-FireAlarm node (blinking red during an alarm)
- FH-Door node (green = clear, red = alert/intrusion)

(Founder's Hall mirror of BSH's RGB LED usage — identical hardware, wiring, and logic.)

## Function
Visual status indicator. On the fire alarm node it blinks red during an active alarm; on the door node it shows the gate's current security state.

## Key Specs
- Common-cathode RGB LED (or 3 separate LEDs)
- Current-limiting resistor (~220Ω) on each color channel
- PWM-capable pins recommended for blink/fade effects

## Wiring / Pins
| Node | Channel | GPIO |
|---|---|---|
| FH-FireAlarm | Red | GPIO 25 |
| FH-FireAlarm | Green | GPIO 26 |
| FH-FireAlarm | Blue | GPIO 27 |
| FH-Door | Red | GPIO 25 |
| FH-Door | Green | GPIO 26 |
| FH-Door | Blue | GPIO 27 |

This is the same pin convention used on your IRONCLAD crossing-gate RGB LED — same board role, same wiring.

## Modbus Register Tie-In
- FireAlarm node: follows the same alarm state as the buzzer (`ALARM_ACTIVATE` / `MANUAL_PULL_TRIGGERED`)
- Door node: reflects `DOOR_POSITION` / `IR_DETECTED` state and responds to `DOOR_OPEN_CMD` / `DOOR_CLOSE_CMD`

## Notes
This doc mirrors `BSH_Sensors/05_RGB_LED.md` exactly — same components, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-rgb-led` (set `"common": "cathode"`; Wokwi defaults to anode)

**How to use it:** R/G/B each through a 220Ω `wokwi-resistor` to its GPIO, COM to GND — the same wiring as the real bench, so the simulation doubles as a wiring diagram.

**What the simulation hides:** Wokwi will light the LED even without resistors. The real LED (and GPIO) can be damaged without them.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
