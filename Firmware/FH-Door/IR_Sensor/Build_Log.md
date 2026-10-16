# IR Sensor — Build Log (FH-Door)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

> **Phase 2 — written ahead of its turn.** This log was created on Oct 9, 2026 as a
> mirror of `Firmware/BSH-Door/IR_Sensor/Build_Log.md`. FH systems normally begin after
> all BSH systems are complete (`Firmware/WORKFLOW.md` Section 1); the Door code
> was built for both buildings at once. Only the Wokwi box is checked.

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (BSH's identical simulation ran fine; FH's copy not separately run)
- [ ] Component in hand
- [ ] Wired per `Wiring.md`
- [ ] Sensitivity potentiometer tuned to intended approach distance
- [ ] Polarity confirmed (`IR_ACTIVE_LOW` matches reality) -- **open** as of 2026-10-09
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `FH_Door_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | FH-Door folder created as a mirror of BSH-Door's. `FH_Door_Node.ino` (Phase A sketch) is in place and the Wokwi simulation was copied from BSH's. No FH bench result is recorded; every unchecked box above is genuinely open |

## Lessons Learned

## Troubleshooting Tips
