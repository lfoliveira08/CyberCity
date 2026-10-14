# miguelbalboa / rfid (`MFRC522`)

- **Link:** https://github.com/miguelbalboa/rfid
- **System:** Door (BSH and FH)
- **Status:** **Adopted** — replaced the Wiegand reader on Sep 15, 2026
- **License:** not yet recorded — check before copying code
- **Version installed:** not yet recorded
- **Checked:** not independently verified; written from `References/INDEX.md` and `Piranga_Workflow_v2.md` Section 14

## What it is

The most widely used Arduino library for the MFRC522/RC522 13.56MHz RFID reader
chip, over plain SPI.

## Why it is here

The Door node originally used a Wiegand reader (see
`Door_monkeyboard_Wiegand.md`). The user compared real Wiegand and RC522 hardware
and preferred the RC522; the swap also cut integration risk, since this library
is far better supported than any Wiegand one and SPI is far more forgiving than
Wiegand's pulse timing.

The security lesson survived the swap intact: a plain UID read carries no
authentication, and a MIFARE Classic UID is clonable with cheap tools — which is
Scenario 3 (`Piranga_Workflow_v2.md` Section 11). Arguably a better demo than
sniffing Wiegand pulses, because it is easier to reproduce.

## What we use it for

Reading a card UID and comparing it against `AUTHORIZED_CARDS`, which sets
`ACCESS_GRANTED` (DI2). Wiring and pins: `BSH_Systems/03_BSH-Door.md` — SPI on
the ESP32's default VSPI pins (SCK 18, MISO 19, MOSI 23) with SS on 32 and RST
on 33.

## Things to watch

- **The RC522 module is 3.3V only.** Powering it from 5V risks the module. See
  `BSH_Sensors/11_RFID_RC522_Reader.md`.
- `AUTHORIZED_CARDS` is still a placeholder — it needs real badged UIDs, which
  can only be read off the actual cards during Phase A.
- Wokwi has a native `board-mfrc522` part with 6 preset cards, which is enough
  to rehearse both the authorized and the cloned-card paths before any hardware
  exists.
