# Push Buttons — Build Log (BSH-Elevator)

## Status

> **Bench-test sketch:** `BenchTest.ino` in this folder — the sketch that
> exercised this component on its own during Phase A. Moved here Sep 26, 2026
> from an untracked `Claude outputs/` download folder; component folders may now
> hold an optional `BenchTest.ino` (`Firmware/WORKFLOW.md` Sections 3 and 8).
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-09-23
- [x] All 3 buttons in hand
- [x] Wired per `Wiring.md`
- [x] Bench-tested standalone (see checklist in `Wiring.md`)
- [x] Confirmed working with the real `BSH_Elevator_Node.ino` firmware

## Date log
| Date | What happened |
|---|---|
| 2026-09-23 | Wokwi simulation run and confirmed working (all 3 floor buttons, debounce, requestFloor()). |
| 2026-09-25 | Wired on the real bench per `Wiring.md`, confirmed working in the combined Phase A integration sketch alongside the OLED and L293D/motor. |

## Lessons Learned
- Buttons were the first component wired and gave no trouble — confirms `INPUT_PULLUP` + software debounce (50ms) was the right call, no external resistors needed on GPIO 4/16/17.

## Troubleshooting Tips
- None needed — this component worked first try on the bench.
