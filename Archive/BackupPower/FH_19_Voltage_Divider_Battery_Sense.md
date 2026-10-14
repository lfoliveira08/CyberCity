# Resistor Voltage Divider (UPS Battery Sense)

**Used in:** FH-BackupPower node (UPS battery voltage sense)

## Function
Scales the UPS/backup battery pack's voltage down into the ESP32 ADC's
safe 0–3.3V range so firmware can compute a battery percentage — models
the analog battery-telemetry point a real UPS exposes.

## Key Specs
- Simple two-resistor divider (R1 from battery+, R2 to GND, tap between
  them to the ADC pin)
- **The ratio is a placeholder** — size R1/R2 for whatever battery pack is
  actually used on the bench, so the worst-case (fully charged) voltage
  divides down to comfortably under 3.3V
- ESP32 ADC1 pin only — ADC2 is unreliable once WiFi is active

## Wiring / Pins (FH-BackupPower ESP32)
| Signal | GPIO |
|---|---|
| Divider tap (battery+ → R1 → tap → R2 → GND) | GPIO 34 |

## Modbus Register Tie-In
Feeds Input Register `UPS_BATTERY_PCT` (scaled 0–100) directly, and also
feeds Discrete Input `LOW_BATTERY` via a firmware threshold on that same
reading.

## Notes
FH mirrors BSH exactly — see `BSH_Sensors/18_Voltage_Divider_Battery_Sense.md`.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — `wokwi-potentiometer` (no battery/divider part)

**How to use it:** Wire the potentiometer's wiper to GPIO 34. Turn it to sweep `UPS_BATTERY_PCT` up and down and cross the `LOW_BATTERY` threshold.

**What the simulation hides:** Everything about the real divider math and the real battery's discharge curve.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
