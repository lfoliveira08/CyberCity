# Relay Module -- Build Log (FH-HVAC)

## Status
- [ ] Simulated in Wokwi first (Phase A0)
- [ ] Component in hand
- [x] Wired per `Wiring.md` (2026-10-07)
- [x] Polarity determined on THIS board: active-HIGH (2026-10-07)
- [x] Bench-tested (as part of the full-node flash, 2026-10-07)
- [x] Confirmed with the real `FH_HVAC_Node.ino`

## Date log
| Date | What happened |
|---|---|
| 2026-10-07 | Parts in hand: keyestudio 1-channel relay, small DC motor with fan blade, 1N4007 diode. Motor to run from the board's 5V pin (no separate supply). Folder created, BenchTest written. Wokwi relay energizes on HIGH, so the sim sketch uses `RELAY_ACTIVE_LOW=false`; the real default `true` is untested. |
| 2026-10-07 | First flash of the full node sketch: the relay/fan worked but in reverse (the real default was active-LOW, untested). Set `RELAY_ACTIVE_LOW = false` in `FH_HVAC_Node.ino` and `Relay_Module/BenchTest.ino`. Same result as BSH/FH-PowerBox boards and Wokwi, but confirmed on this board. |
| 2026-10-07 | Full node sketch (`FH_HVAC_Node.ino`, relay active-HIGH) flashed on the bench: idle, cooling (relay + fan + blue LED) and `HVAC_ENABLE` toggle all behaved as designed. Fault path (unplug DHT11) not yet recorded. |

## Lessons Learned

## Troubleshooting Tips
- Inverted behaviour: flip `RELAY_ACTIVE_LOW`. Check the relay's own onboard LED, not only the load.
- No click at all: coil needs 5V, not 3.3V; check common ground.
- ESP32 resets when the relay switches: load is sharing the ESP32 supply, or no flyback/decoupling.
