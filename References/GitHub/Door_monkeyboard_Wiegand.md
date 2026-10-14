# monkeyboard / Wiegand-Protocol-Library-for-Arduino

- **Link:** https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
- **System:** Door (BSH and FH) — the card reader, until Sep 15, 2026
- **Status:** **Rejected.** The reader hardware was swapped to an MFRC522/RC522 RFID module over SPI
- **License:** not recorded; nothing was kept
- **Checked:** not independently verified; written from `References/INDEX.md` and `Piranga_Workflow_v2.md` Section 14

## What it was for

Decoding the Wiegand protocol — the legacy two-wire (D0/D1) pulse format that
commercial access-control card readers use. It was the Door node's original field
bus, chosen because Wiegand is genuinely what real building access control runs
on, and because its total lack of replay protection made a clean security demo.

## Why it was dropped

1. **The user had both readers in hand** and preferred the RC522.
2. **Integration risk.** Wiegand decoding depends on finicky pulse timing and the
   library support is comparatively niche; `miguelbalboa/rfid` is one of the
   best-supported Arduino libraries for any RFID chip, over plain SPI.
3. **The teaching point survived the swap.** A plain RFID UID read has no
   authentication either, and a MIFARE Classic UID is clonable with widely
   available tools for about $10–15. Scenario 3 was renamed from "Wiegand Replay
   Attack" to "RFID UID Clone/Replay Attack" and lost nothing.

## What changed in the design when it went

GPIO 32 and 33 were repurposed from Wiegand D0/D1 to the RC522's SS and RST, and
the ESP32's default VSPI pins (18/19/23) came into use on this node for the first
time. The register map did not change at all — `ACCESS_GRANTED` (DI2) still means
exactly what it meant. The old `Wiegand_Card_Reader/` build folder was replaced by
`RFID_RC522_Reader/`.

## If it is ever revisited

Nothing in the current design blocks going back — the field bus is independent of
the Modbus backhaul. The honest comparison is that Wiegand is more realistic to
commercial access control, and RC522 is more realistic to what a student can
clone on camera in a demo.
