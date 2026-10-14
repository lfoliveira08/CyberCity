# Battery Voltage Divider — Build Log (BSH-BackupPower)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-05
- [ ] R1/R2 calculated for the actual battery pack in hand
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone against a multimeter reading (see checklist
      in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_BackupPower_Node.ino` firmware
      (`UPS_BATTERY_PCT` / `LOW_BATTERY` track the real battery)

## Real R1/R2 values used
*(fill in once calculated — don't leave this as a placeholder)*

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Wokwi run: potentiometer on GPIO 34 sweeps UPS_BATTERY_PCT; LOW_BATTERY flips at the 20% placeholder (16% YES, 68% no). Raw % keeps reporting while flag is set. Matches scenarios 6, 7. Real divider math still to do on the bench. |

## Lessons Learned

## Troubleshooting Tips
