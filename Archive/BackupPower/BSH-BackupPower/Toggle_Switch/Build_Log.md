# Toggle Switch — Build Log (BSH-BackupPower)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-05
- [ ] Component in hand
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_BackupPower_Node.ino` firmware
      (Modbus `UTILITY_POWER_LOST` discrete input tracks the switch)

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Wokwi run: slide switch LEFT/RIGHT flips UTILITY_POWER_LOST, debounced, never moves the relay. Matches README scenarios 2, 3. |

## Lessons Learned

## Troubleshooting Tips
