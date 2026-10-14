# ACS712 Current Sensor

**Used in:** FH-BackupPower node (load current sense)

## Function
Hall-effect current sensor reporting the current drawn by the demo
backup-power load — models the load-monitoring telemetry a real UPS or
generator system reports.

## Key Specs
- Common 5A/20A/30A ACS712 breakout variants; pick whichever matches the
  demo load's current draw
- Analog output centered near half of its own supply voltage (~2.5V on a
  5V-supplied board), sensitivity varies by variant — check the specific
  board's datasheet before scaling in firmware
- **Never wire this in series with mains power.** This project's "load" is
  a low-voltage demo circuit only

## Wiring / Pins (FH-BackupPower ESP32)
| Signal | GPIO |
|---|---|
| OUT (analog) | GPIO 35 |

## Modbus Register Tie-In
Feeds Input Register `LOAD_CURRENT_MA` directly.

## Notes
FH mirrors BSH exactly — see `BSH_Sensors/19_ACS712_Current_Sensor.md`,
including the wiring caution about 5V-supplied boards swinging close to
or above 3.3V.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — `wokwi-potentiometer` (no ACS712 part in Wokwi)

**How to use it:** Wire the potentiometer's wiper to GPIO 35. Turn it to sweep `LOAD_CURRENT_MA` up and down.

**What the simulation hides:** The real Hall-effect sensing, the board's actual center-voltage/sensitivity, and the 5V-swing hazard noted above.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
