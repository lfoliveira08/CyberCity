# RGB LED -- Build Log (FH-HVAC)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

## Status
- [ ] Simulated in Wokwi first (Phase A0)
- [x] Component in hand (RGB LED confirmed available 2026-10-07; common cathode vs anode: ______)
- [x] Wired per `Wiring.md` (2026-10-07)
- [x] Bench-tested (as part of the full-node flash, 2026-10-07)
- [x] Confirmed with the real `FH_HVAC_Node.ino`

## Date log
| Date | What happened |
|---|---|
| 2026-10-07 | Folder created, BenchTest written. Briefly changed to two plain LEDs (none seen in the parts photo), then reverted the same day when an RGB LED turned up. |
| 2026-10-07 | Full node sketch (`FH_HVAC_Node.ino`, relay active-HIGH) flashed on the bench: idle, cooling (relay + fan + blue LED) and `HVAC_ENABLE` toggle all behaved as designed. Fault path (unplug DHT11) not yet recorded. |

## Lessons Learned

## Troubleshooting Tips
- Colours inverted or everything lit: common-anode part wired as cathode.
- One channel dim or dead: missing resistor or wrong leg; legs are R, common, G, B in physical order.
