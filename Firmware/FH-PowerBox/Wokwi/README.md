# FH-PowerBox — power rails (Wokwi)

**Mirrors `Firmware/BSH-PowerBox/Wokwi/`, rebuilt Oct 28, 2026.** Rail names changed to FH-HVAC/FireAlarm/Door (FH has no Elevator). Not yet run in Wokwi for FH specifically — the identical BSH version was run and worked (`Logs/2026-10-27.md`, `2026-10-28.md`); this copy has not been independently re-tested.

## How to open it

1. wokwi.com → new ESP32 project.
2. Replace `diagram.json` and `sketch.ino` with the two files in this folder.
3. Start the simulation, open the Serial Monitor.

## What to try

| Type + Enter | Expected |
|---|---|
| (nothing, at boot) | all three rail LEDs **dark** (option A: off until commanded) |
| `1` | FH-HVAC LED lights (rail powered); `1` again cuts it |
| `2` / `3` | same for FireAlarm / Door |
| `a` | all three LEDs dark |
| `r` | all three lit |

## Real bench differences

Same as BSH-PowerBox's Wokwi README: current draw and relay contact rating are not modelled, boot-time pin behaviour is not modelled, and relay polarity must be checked fresh on each real board (Wokwi energises on HIGH).
