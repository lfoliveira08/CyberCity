# RFID Reader (MFRC522/RC522) — BSH-Door

Reference doc: `BSH_Sensors/11_RFID_RC522_Reader.md`

> **Replaces** `Wiegand_Card_Reader/` in this build folder — same role
> (grants entry via a card read), different hardware. See Section 14,
> Lessons Learned & Decisions Log, of `Piranga_Workflow_v2.md` for why.

## What it does here
Reads an access card/fob's UID over SPI; the ESP32 decides whether to
grant entry by checking the UID against a locally-stored authorized list.
Same "least-precedented library" caution that applied to Wiegand does
**not** carry over here — MFRC522 is one of the most common, best-tested
Arduino RFID libraries — but this is still the node's access-control front
end, so build and test it in isolation before wiring anything else on
this node.

## Key specs
- MFRC522-based module ("RC522"), 13.56MHz, ISO14443A (MIFARE
  Classic/Ultralight)
- SPI interface: SCK, MOSI, MISO, SDA/SS, RST, VCC, GND
- **3.3V power and logic ONLY.** Do not connect VCC to the ESP32's 5V pin.
  This is a real, easy-to-make mistake if you're used to wiring 5V modules
  — double check before powering on.
- No authentication/encryption on a plain UID read — intentional for this
  project's cybersecurity teaching goals (Scenario 3, RFID UID Clone)

## Wiring
| Reader pin | ESP32 pin |
|---|---|
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |
| SDA / SS | GPIO 32 |
| RST | GPIO 33 |
| VCC | 3.3V (**not** 5V) |
| GND | Common ground |

GPIO 18/19/23 are the ESP32's default VSPI pins — `SPI.begin()` uses them
automatically with no extra arguments needed.

## Firmware tie-in
`BSH_Door_Node.ino`: `PIN_RFID_SS = 32`, `PIN_RFID_RST = 33`,
`MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST);`, `SPI.begin(18, 19, 23, 32)` +
`mfrc522.PCD_Init()` in `setup()`. In `loop()`,
`mfrc522.PICC_IsNewCardPresent()` / `mfrc522.PICC_ReadCardSerial()` gate a
read; the UID lives in `mfrc522.uid.uidByte[]` / `mfrc522.uid.size`. A read
is checked against `AUTHORIZED_CARDS[]` via `isAuthorized(uid, len)` —
BSH's list holds `{0xB3, 0x12, 0x87, 0x2D}` (Oct 9, 2026); FH keeps its own
separate list. The sketch uses a byte-array compare and calls
`PICC_HaltA()` after each read.

## Bench-test checklist (Phase A, this component only — Serial only, no
other sensors need to be wired yet)
- [ ] Reader wired per the table above — **confirm VCC is 3.3V before
      powering on**
- [ ] Flash a minimal test sketch (or the real firmware with Serial Monitor
      open) at 115200 baud
- [ ] Badge a real card — confirm `PICC_IsNewCardPresent()` /
      `PICC_ReadCardSerial()` return true and the UID prints out, stable
      and repeatable for the same card
- [ ] Badge 2-3 different cards if you have them — confirm each gets a
      distinct UID
- [ ] Copy your real card's UID bytes into `AUTHORIZED_CARDS[]` in
      `BSH_Door_Node.ino`, replacing the placeholder `{0xDE, 0xAD, 0xBE, 0xEF}`

## Card UIDs read (fill in as you test)
| Card | UID (hex bytes) | Date |
|---|---|---|
| BSH card (in the sketch) | B3 12 87 2D | 2026-10-09 (taken from the sketch; bench read not logged yet) |

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "no reads at all" -> check SCK/MOSI/MISO
aren't swapped, confirm VCC is actually 3.3V with a multimeter, and that
SS/RST match the pins passed to the `MFRC522` constructor)*
