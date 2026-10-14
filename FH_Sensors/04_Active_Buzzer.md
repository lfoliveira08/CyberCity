# Active Buzzer

**Used in:** FH-FireAlarm node (Founder's Hall mirror of BSH-FireAlarm)

## Function
Audible alarm output, sounded when a fire/intrusion event is triggered — either locally via the pull-station button or remotely via a hub (or attacker) command.

## Key Specs
- Active buzzer (has its own internal oscillator — no tone-generation code needed)
- 3.3V or 5V, digital drive (HIGH = on)
- ~30mA draw

## Wiring / Pins (FH-FireAlarm ESP32)
| Signal | GPIO |
|---|---|
| Signal | GPIO 23 |
| GND | Common ground |

Safe to power directly from the ESP32's 3.3V pin at this current draw.

## Modbus Register Tie-In
Driven by whichever fires first:
- Coil `ALARM_ACTIVATE` (hub/remote command), or
- Discrete Input `MANUAL_PULL_TRIGGERED` (local button)

Firmware should logically OR these two sources.

## Notes
Because it's an *active* buzzer, a simple `digitalWrite(HIGH)` is enough to sound it — no PWM tone generation required. Keeps this node's firmware simple, in line with "stability over complexity."

This doc mirrors `BSH_Sensors/04_Active_Buzzer.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native, with one catch — `wokwi-buzzer`

**How to use it:** Pin `2` (+) to the GPIO, pin `1` (−) to GND. Wokwi's buzzer behaves like a *passive* piezo: a constant `digitalWrite(HIGH)` stays silent. The simulation sketches therefore use `tone(PIN, 2000)` / `noTone(PIN)` behind a `WOKWI_SIM` flag; the real firmware keeps `digitalWrite`.

**What the simulation hides:** Real active buzzers need only HIGH/LOW. Do not copy the `tone()` calls into the bench firmware.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
