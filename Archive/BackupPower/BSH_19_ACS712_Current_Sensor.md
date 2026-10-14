# ACS712 Current Sensor

**Used in:** BSH-BackupPower node (load current sense)

## Function
Hall-effect current sensor reporting the current drawn by the demo
backup-power load — models the load-monitoring telemetry a real UPS or
generator system reports.

## Key Specs
- Common 5A/20A/30A ACS712 breakout variants; pick whichever matches the
  demo load's current draw
- Analog output centered near half of its own supply voltage (~2.5V on a
  5V-supplied board) with a small mV/A sensitivity that varies by variant
  — check the specific board's datasheet before scaling in firmware
- **Never wire this in series with mains power.** This project's "load" is
  a low-voltage demo circuit only

## Wiring / Pins (BSH-BackupPower ESP32)
| Signal | GPIO |
|---|---|
| OUT (analog) | GPIO 35 |

## Modbus Register Tie-In
Feeds Input Register `LOAD_CURRENT_MA` directly.

## Notes
Measure the board's actual OUT swing with a multimeter before wiring
directly into GPIO 35 — some 5V-supplied boards can swing close to or
above 3.3V depending on current draw, which is unsafe for the ESP32's ADC.
Add a voltage divider if it does, same pattern already used for the
HC-SR04 and flagged for the MQ-2.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — `wokwi-potentiometer` (no ACS712 part in Wokwi)

**How to use it:** Wire the potentiometer's wiper to GPIO 35. Turn it to sweep `LOAD_CURRENT_MA` up and down.

**What the simulation hides:** The real Hall-effect sensing, the board's actual center-voltage/sensitivity, and the 5V-swing hazard above — the potentiometer only teaches the firmware-side scaling logic.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
