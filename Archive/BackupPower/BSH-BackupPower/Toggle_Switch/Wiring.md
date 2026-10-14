# Utility-Power-Loss Toggle Switch — BSH-BackupPower

Reference doc: `BSH_Sensors/17_Toggle_Switch.md`

## What it does here
Stands in for a real power-monitoring relay's dry contact — flipped
manually to simulate a utility outage. Feeds `UTILITY_POWER_LOST` (DI0)
directly, reported raw and honestly.

## Wiring
| Switch terminal | ESP32 pin |
|---|---|
| Terminal 1 | GPIO 26 (`INPUT_PULLUP`) |
| Terminal 2 | GND |

No external pull-up needed — GPIO 26 has an internal one available.

## Firmware tie-in
`BSH_BackupPower_Node.ino` (not written yet): `PIN_UTILITY_SWITCH = 26`,
active LOW. Feeds Discrete Input `UTILITY_POWER_LOST` (DI0).

## Bench-test checklist (Phase A, this component only)
- [ ] Wired per the table above
- [ ] Flip the switch, confirm the pin reads LOW/HIGH cleanly over Serial
      with no bounce issues

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go)*
