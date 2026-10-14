# ACS712 Current Sensor — Build Log (BSH-BackupPower)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-05
- [ ] Component in hand
- [ ] OUT swing measured with a multimeter (see checklist in `Wiring.md`)
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_BackupPower_Node.ino` firmware
      (`LOAD_CURRENT_MA` tracks the demo load)

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Wokwi run: potentiometer on GPIO 35 moves LOAD_CURRENT_MA (0 at mid-scale, clamped 5000). Matches scenario 8. Pots start at minimum in Wokwi, so boot reads 5000 mA. Real zero baseline and 5V swing still to check on the bench. |

## Lessons Learned

## Troubleshooting Tips
