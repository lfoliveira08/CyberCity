# Reed Switch — Build Log (BSH-Door)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (Luis ran it, looked fine)
- [ ] Component in hand
- [ ] **External 10kΩ pull-up resistor installed** (GPIO 34 has none
      internally — see `Wiring.md`)
- [ ] Reed part checked: the Phase A build uses a 3-pin S/V/G module -- does it
      have its own pull-up? (or is the external 10k added) -- **open** as of 2026-10-09
- [ ] Wired + mounted per `Wiring.md`
- [ ] Polarity confirmed (`REED_LOW_MEANS_CLOSED` matches reality)
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with the real `BSH_Door_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | Phase A sketch received: reed on GPIO 34 with plain `INPUT`, `REED_LOW_MEANS_CLOSED` still at its default (`true`), raw value printed each half second. The part is a 3-pin module rather than a bare switch; pull-up and polarity both unconfirmed on hardware |

## Lessons Learned

## Troubleshooting Tips
