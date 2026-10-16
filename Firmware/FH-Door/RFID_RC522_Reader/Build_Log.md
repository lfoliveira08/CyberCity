# RFID Reader (MFRC522/RC522) — Build Log (FH-Door)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

> **Phase 2 — written ahead of its turn.** This log was created on Oct 9, 2026 as a
> mirror of `Firmware/BSH-Door/RFID_RC522_Reader/Build_Log.md`. FH systems normally begin after
> all BSH systems are complete (`Firmware/WORKFLOW.md` Section 1); the Door code
> was built for both buildings at once. Only the Wokwi box is checked.

> Replaces `Wiegand_Card_Reader/Build_Log.md` — see this folder's
> `Wiring.md` for the swap rationale.

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (BSH's identical simulation ran fine; FH's copy not separately run)
- [ ] Reader in hand, confirmed it's a 3.3V part (check the datasheet/
      listing — do not assume, and do not power from 5V)
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone over Serial (see checklist + UID table in
      `Wiring.md`)
- [ ] Real card UID(s) copied into `AUTHORIZED_CARDS[]` in
      `FH_Door_Node.ino`
- [ ] Confirmed working with the real firmware (authorized card opens the
      gate once the servo is wired; unauthorized card does not)

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | FH-Door folder created as a mirror of BSH-Door's. `FH_Door_Node.ino` (Phase A sketch) is in place and the Wokwi simulation was copied from BSH's. No FH bench result is recorded; every unchecked box above is genuinely open |

## Lessons Learned

## Troubleshooting Tips
