# Reed Switch — Build Log (FH-Door)

> **Phase 2 — written ahead of its turn.** This log was created on Oct 9, 2026 as a
> mirror of `Firmware/BSH-Door/Reed_Switch/Build_Log.md`. FH systems normally begin after
> all BSH systems are complete (`Firmware/WORKFLOW.md` Section 1); the Door code
> was built for both buildings at once. Only the Wokwi box is checked.

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (BSH's identical simulation ran fine; FH's copy not separately run)
- [ ] Component in hand
- [ ] **External 10kΩ pull-up resistor installed** (GPIO 34 has none
      internally — see `Wiring.md`)
- [ ] Reed part checked: the Phase A build uses a 3-pin S/V/G module -- does it
      have its own pull-up? (or is the external 10k added) -- **open** as of 2026-10-09
- [ ] Wired + mounted per `Wiring.md`
- [ ] Polarity confirmed (`REED_LOW_MEANS_CLOSED` matches reality)
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `FH_Door_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | FH-Door folder created as a mirror of BSH-Door's. `FH_Door_Node.ino` (Phase A sketch) is in place and the Wokwi simulation was copied from BSH's. No FH bench result is recorded; every unchecked box above is genuinely open |

## Lessons Learned

## Troubleshooting Tips
