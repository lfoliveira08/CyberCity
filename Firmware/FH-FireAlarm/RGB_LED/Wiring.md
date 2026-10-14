# RGB LED — FH-FireAlarm

> **Founder's Hall mirror.** Identical component and identical wiring to
> BSH-FireAlarm — this file was copied from
> `Firmware/BSH-FireAlarm/RGB_LED/Wiring.md` on Sep 26, 2026 and re-pointed at FH's
> own docs and IP (192.168.50.22). **Nothing here has been verified on FH
> hardware yet** — FH systems are Phase 2, and Phase 2 has not started. If a
> correction is found while building either building, fix both copies.


Reference doc: `FH_Sensors/05_RGB_LED.md`

## What it does here
Visual alarm status: steady green = normal, blinking red = alarm active
(local button or remote `ALARM_ACTIVATE`, same OR logic as the buzzer).

## Key specs
- Common-cathode RGB LED (or 3 separate LEDs)
- One 220Ω current-limiting resistor per color channel — do not skip these,
  they protect both the LED and the GPIO pin
- PWM-capable pins used here, though this node only needs on/off (no fade)

## Wiring
| LED pin | Through | ESP32 pin |
|---|---|---|
| Red anode/leg | 220Ω resistor | GPIO 25 |
| Green anode/leg | 220Ω resistor | GPIO 26 |
| Blue anode/leg | 220Ω resistor | GPIO 27 |
| Common cathode | — | GND |

(If your LED is common-**anode** instead of common-cathode, the polarity
and firmware logic both flip — check which you have before wiring; a
common-anode LED wired like this will not light correctly.)

## Firmware tie-in
`FH_FireAlarm_Node.ino`: `PIN_RGB_RED/GREEN/BLUE = 25/26/27`, driven by the
`setRgb(r, g, b)` helper. Boot state is steady green
(`setRgb(false, true, false)`); alarm state blinks red at 300ms via
`BLINK_INTERVAL_MS`.

## Bench-test checklist (Phase A, this component only)
- [ ] All three resistors + LED wired per the table above
- [ ] Power up the ESP32 with the real firmware flashed (or a throwaway
      test sketch) — LED should be steady green at boot
- [ ] Trigger the alarm (press the pull-station button once wired, or just
      short GPIO 4 to GND briefly) — LED should blink red
- [ ] Release — LED returns to steady green

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "wrong color lights" -> check R/G/B legs aren't
swapped; "LED very dim" -> check resistor value, not a wiring fault)*
