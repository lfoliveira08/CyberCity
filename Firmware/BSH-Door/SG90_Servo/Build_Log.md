# SG90 Servo — Build Log (BSH-Door)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (Luis ran it, looked fine)
- [ ] Servo + external 5V supply in hand
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] `GATE_OPEN_ANGLE` tuned to the physical gate arm if needed
- [ ] Confirmed working with the real `BSH_Door_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | Phase A sketch received: servo on GPIO 13, attached only while moving, sweeps 0 -> 90 -> 0 in 5-degree steps (matches `GATE_OPEN_ANGLE = 90`). Earlier draft swept to 100 degrees. Bench result not recorded |

## Lessons Learned

## Troubleshooting Tips
