# IR Sensor — Build Log (BSH-Door)

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (Luis ran it, looked fine)
- [ ] Component in hand
- [ ] Wired per `Wiring.md`
- [ ] Sensitivity potentiometer tuned to intended approach distance
- [ ] Polarity confirmed (`IR_ACTIVE_LOW` matches reality) -- **open** as of 2026-10-09
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_Door_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | Phase A sketch received and checked against the pin map: IR is on GPIO 14 and `IR_ACTIVE_LOW` is still at its default (`true`). Earlier drafts had the IR on GPIO 27 and assumed HIGH = detected; both were corrected. Polarity not yet checked on hardware |

## Lessons Learned

## Troubleshooting Tips
