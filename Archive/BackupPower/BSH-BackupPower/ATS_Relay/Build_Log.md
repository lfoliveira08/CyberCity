# ATS Relay — Build Log (BSH-BackupPower)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-05
- [ ] Component in hand
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_BackupPower_Node.ino` firmware
      (Modbus `ATS_TRANSFER_CMD` coil write drives the relay)

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Wokwi run: typing 1/0 in Serial Monitor (stand-in for Coil0) turns the relay and green demo-load LED on/off. Wokwi relay is active-HIGH (RELAY_ACTIVE_LOW=false); real module must be checked on the bench. Command accepted with no outage (scenario 9). Matches scenarios 4, 5, 9. |

## Lessons Learned

## Troubleshooting Tips
