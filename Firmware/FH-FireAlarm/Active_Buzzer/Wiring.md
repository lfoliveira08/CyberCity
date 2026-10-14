# Active Buzzer — FH-FireAlarm

> **Founder's Hall mirror.** Identical component and identical wiring to
> BSH-FireAlarm — this file was copied from
> `Firmware/BSH-FireAlarm/Active_Buzzer/Wiring.md` on Sep 26, 2026 and re-pointed at FH's
> own docs and IP (192.168.50.22). **Nothing here has been verified on FH
> hardware yet** — FH systems are Phase 2, and Phase 2 has not started. If a
> correction is found while building either building, fix both copies.


Reference doc: `FH_Sensors/04_Active_Buzzer.md`

## What it does here
Sounds when the alarm is active — triggered by the local pull-station
button OR a remote/hub Modbus command (`ALARM_ACTIVATE`), whichever fires
first (firmware ORs the two sources).

## Key specs
- Active buzzer — has its own internal oscillator, no tone-generation code
  needed
- 3.3V or 5V, digital drive (HIGH = on), ~30mA draw
- Safe to power directly from the ESP32's 3.3V pin at this current draw

## Wiring
| Buzzer pin | ESP32 pin |
|---|---|
| Signal (+) | GPIO 23 |
| GND (–) | GND |

No resistor needed — this is a digital on/off drive, not an LED.

## Firmware tie-in
`FH_FireAlarm_Node.ino`: `PIN_BUZZER = 23`, driven by
`digitalWrite(PIN_BUZZER, HIGH/LOW)` inside the combined
`alarmActive = debouncedButtonState || remoteAlarm` logic.

## Bench-test checklist (Phase A, this component only)
- [ ] Buzzer wired per the table above, ESP32 not yet powered
- [ ] Power up the ESP32, buzzer stays silent at boot (confirms boot state
      is OFF, not a wiring short driving it on)
- [ ] Temporarily jumper GPIO 23 to 3.3V by hand (ESP32 unpowered while you
      do this) or run a throwaway one-line test sketch
      (`digitalWrite(23, HIGH)` in `setup()`) — buzzer should sound
      immediately, no PWM/tone code required
- [ ] Confirms: correct pin, correct polarity, buzzer itself is good

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "no sound" -> check GND is actually common with
the ESP32, not just the buzzer's own supply)*
