# RFID Reader (MFRC522/RC522) — Build Log (BSH-Door)

> Replaces `Wiegand_Card_Reader/Build_Log.md` — see this folder's
> `Wiring.md` for the swap rationale.

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: 2026-10-09 (Luis ran it, looked fine)
- [ ] Reader in hand, confirmed it's a 3.3V part (check the datasheet/
      listing — do not assume, and do not power from 5V)
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone over Serial (see checklist + UID table in
      `Wiring.md`)
- [x] Real card UID(s) copied into `AUTHORIZED_CARDS[]` in
      `BSH_Door_Node.ino`
- [ ] Confirmed working with the real firmware (authorized card opens the
      gate once the servo is wired; unauthorized card does not)

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | Phase A sketch received: SPI RC522 with the `MFRC522` library, pins 18/19/23/32/33, BSH card `B3 12 87 2D` in `AUTHORIZED_CARDS[]`. Earlier BSH draft used an I2C RFID module and a text-string UID compare that dropped leading zeros; both were replaced. Bench read not logged yet |

## Lessons Learned

## Troubleshooting Tips
