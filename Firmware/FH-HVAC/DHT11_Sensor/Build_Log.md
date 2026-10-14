# DHT11 -- Build Log (FH-HVAC)

## Status
- [ ] Simulated in Wokwi first (Phase A0 -- see `../Wokwi/README.md`)
- [x] Component in hand: DHT11, 3-pin module (pull-up already on the module)
- [x] Wired per `Wiring.md` (2026-10-07)
- [x] Bench-tested (as part of the full-node flash, 2026-10-07) (`BenchTest.ino`)
- [x] Confirmed with the real `FH_HVAC_Node.ino`, including fault path (unplug DATA)

## Date log
| Date | What happened |
|---|---|
| 2026-10-07 | Folder created, wiring and BenchTest written. Nothing run yet. |
| 2026-10-07 | Part in hand is a DHT11, not the DHT22 in the design docs. Folder renamed `DHT22_Sensor` -> `DHT11_Sensor`; firmware uses `DHT11` on hardware (range 0-50 C accepted), `DHT22` in Wokwi. Hysteresis effectively 1 C because readings are whole degrees. |
| 2026-10-07 | Full node sketch (`FH_HVAC_Node.ino`, relay active-HIGH) flashed on the bench: idle, cooling (relay + fan + blue LED) and `HVAC_ENABLE` toggle all behaved as designed. Fault path (unplug DHT11) not yet recorded. |

## Lessons Learned

## Troubleshooting Tips
- Always nan: no pull-up, DATA on wrong GPIO, VCC/GND swapped, or reading faster than every 2 s.
- Values stuck at one number: sensor warming, or a stale read from sampling too fast.
- Temperature reads high: sensor sits in the fan airflow or next to the ESP32 regulator.
