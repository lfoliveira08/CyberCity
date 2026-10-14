# UPS Battery Voltage Divider — BSH-BackupPower

Reference doc: `BSH_Sensors/18_Voltage_Divider_Battery_Sense.md`

## What it does here
Scales the UPS/demo battery pack's voltage into the ADC's safe 0–3.3V
range. Feeds `UPS_BATTERY_PCT` (IR0) directly, and `LOW_BATTERY` (DI1) via
a firmware threshold on that same reading.

## Key specs
- **R1/R2 are placeholders** — calculate for the actual battery pack in
  hand before wiring; recompute if the pack changes
- ESP32 ADC1 pin only (GPIO 34/35/32/33/36/39)

## Wiring
| Signal | ESP32 pin |
|---|---|
| Divider tap (battery+ → R1 → tap → R2 → GND) | GPIO 34 |

## Firmware tie-in
`BSH_BackupPower_Node.ino` (not written yet): `PIN_BATTERY_ADC = 34`.
Scaled to `UPS_BATTERY_PCT` (IR0); `LOW_BATTERY` (DI1) is a threshold on
that value, calibrated on the bench, not assumed.

## Bench-test checklist (Phase A, this component only)
- [ ] R1/R2 calculated and confirmed with a multimeter against the real
      battery pack — worst-case (fully charged) voltage stays comfortably
      under 3.3V at the ADC tap
- [ ] `analogRead()` value logged at a few known battery voltages to
      confirm the scaling math before trusting `UPS_BATTERY_PCT`
- [ ] `LOW_BATTERY` threshold chosen and recorded here, not left as a
      placeholder

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go)*
