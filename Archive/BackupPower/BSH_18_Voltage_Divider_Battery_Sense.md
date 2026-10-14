# Resistor Voltage Divider (UPS Battery Sense)

**Used in:** BSH-BackupPower node (UPS battery voltage sense)

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
- ESP32 ADC1 pin only (GPIO 34/35/32/33/36/39) — ADC2 is unreliable once
  WiFi is active

## Wiring / Pins (BSH-BackupPower ESP32)
| Signal | GPIO |
|---|---|
| Divider tap (battery+ → R1 → tap → R2 → GND) | GPIO 34 |

## Modbus Register Tie-In
Feeds Input Register `UPS_BATTERY_PCT` (scaled 0–100) directly, and also
feeds Discrete Input `LOW_BATTERY` via a firmware threshold on that same
reading — same "raw analog always reported, threshold-derived discrete
alongside it" pattern the retired Perimeter node used for
`DISTANCE_CM`/`OBJECT_IN_RANGE`.

## Notes
Recompute R1/R2 for the actual battery pack before wiring — a divider
sized for the wrong pack risks either exceeding the ADC's 3.3V limit
(possible pin damage) or wasting most of the ADC's resolution on too
small a voltage swing. Measure with a multimeter before trusting the math,
same discipline as the HC-SR04 divider and the MQ-2 caution.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — `wokwi-potentiometer` (no battery/divider part; there's no way to simulate a real discharge curve)

**How to use it:** Wire the potentiometer's wiper to GPIO 34. Turn it to sweep `UPS_BATTERY_PCT` up and down and cross the `LOW_BATTERY` threshold.

**What the simulation hides:** Everything about the real divider math and the real battery's discharge curve — the potentiometer only teaches the firmware-side threshold logic.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
