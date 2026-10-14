# ACS712 Current Sensor — BSH-BackupPower

Reference doc: `BSH_Sensors/19_ACS712_Current_Sensor.md`

## What it does here
Reports load current drawn by the demo backup-power circuit. Feeds
`LOAD_CURRENT_MA` (IR1) directly.

## Key specs
- Common 5A/20A/30A ACS712 breakout variants — check the board's own
  datasheet for its actual mV/A sensitivity and center voltage
- **Never wire in series with mains power** — demo load only
- **Measure the board's real OUT swing with a multimeter before wiring**
  — some 5V-supplied boards can swing close to or above 3.3V

## Wiring
| Signal | ESP32 pin |
|---|---|
| OUT (analog) | GPIO 35 |

Add a voltage divider here if the measured swing exceeds 3.3V, same
pattern as the HC-SR04's ECHO divider used on the retired Perimeter node.

## Firmware tie-in
`BSH_BackupPower_Node.ino` (not written yet): `PIN_ACS712_ADC = 35`.
Scaled to `LOAD_CURRENT_MA` (IR1).

## Bench-test checklist (Phase A, this component only)
- [ ] OUT swing measured with a multimeter at zero load — confirmed under
      3.3V
- [ ] Baseline `analogRead()` logged with no load current flowing
- [ ] Confirmed the reading moves when the demo load is toggled

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go)*
