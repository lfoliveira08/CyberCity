# RFID Reader (MFRC522 / RC522)

**Used in:** FH-Door node (Founder's Hall mirror of BSH-Door)
**Layer:** Field bus — reader ↔ ESP32 only (SPI). Not routable, not the backhaul to OpenPLC.

> **Replaces the Wiegand Card Reader** as this node's access-control front
> end (same role — see Section 14, Lessons Learned & Decisions Log, of
> `Piranga_Workflow_v2.md` for the swap rationale). The register map is
> unchanged: a valid card read still sets `ACCESS_GRANTED` (DI2) exactly
> as before.

## Function
Reads a 13.56MHz RFID/NFC card or fob's UID over SPI and sends it to the
ESP32 (acting as the door controller), which decides whether to grant
entry (open the gate via the SG90 servo) — the same "authorized access"
event source alongside the IR sensor's "unexpected approach" event that
the Wiegand reader provided.

## Key Specs
- MFRC522-based module (commonly sold as "RC522"), 13.56MHz, ISO14443A
  (MIFARE Classic/Ultralight)
- SPI interface: SCK, MOSI, MISO, SDA/SS (chip select), RST, plus VCC/GND
- **3.3V logic and power ONLY — do not power from the ESP32's 5V pin or
  wire it to a 5V-logic bus.** Unlike the old Wiegand reader (which often
  ran 5V logic and needed a level check/divider), this module is a native
  3.3V part — this actually removes a wiring risk rather than adding one.

## Wiring / Pins (FH-Door ESP32)
| Signal | GPIO |
|---|---|
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |
| SDA / SS (chip select) | GPIO 32 |
| RST | GPIO 33 |
| VCC | 3.3V (ESP32 3.3V pin — **not** 5V) |
| GND | Common ground |

*(GPIO 18/19/23 are the ESP32's default VSPI bus pins. GPIO 32/33 carry
over from the old Wiegand D0/D1 assignment, now repurposed as SS/RST —
see `FH_Systems/03_FH-Door.md` for the current full pin map.)*

## Protocol Notes
This module reads a card's UID with **no authentication and no encryption
on the UID read itself** — the same "realistic, insecure legacy access
control" teaching point the Wiegand reader was chosen for, just via a
different, more common attack surface: a MIFARE Classic card's UID is
trivially clonable with widely available ~$10-15 tools (a "magic"
UID-writable card plus a generic RFID read/write module), no protocol
sniffing required. MIFARE Classic also has its own well-known broken
Crypto1 cipher if you want to go further into sector-level cloning. FH-Door
has its own independent `AUTHORIZED_CARDS[]` UID list in
`FH_Door_Node.ino` — badge your real card(s) to this reader specifically,
don't assume BSH-Door's badged cards carry over.

## Backhaul Tie-In
The card read event (valid/invalid, UID) is handled locally on the ESP32,
which then reports the resulting decision (`ACCESS_GRANTED`) to its
dedicated OpenPLC instance (`OpenPLC-FH-Door`) over the system's Modbus
TCP backhaul — see `FH_Systems/03_FH-Door.md` for the current point
map.

## Notes
Well-supported on ESP32 via the standard `MFRC522` Arduino library
(`miguelbalboa/rfid`) — one of the most widely used and stable RFID
libraries for this chip, arguably lower integration risk than the Wiegand
library it replaces.

This doc mirrors `BSH_Sensors/11_RFID_RC522_Reader.md` exactly — same
component, same wiring, only the building (and its independent
authorized-card list) differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `board-mfrc522`

**How to use it:** Pins 3.3V, RST, GND, IRQ, MISO, MOSI, SCK, SDA — wire SCK→18, MISO→19, MOSI→23, SDA(SS)→32, RST→33. Uses the same `MFRC522` library. During simulation, tap or hold one of six preset cards: Blue `01:02:03:04` (UID editable via the `uid` attribute), Green `11:22:33:44`, Yellow `55:66:77:88`, Red `AA:BB:CC:DD`, NFC tag, key fob `C0:FF:EE:99`. Put one of those in `AUTHORIZED_CARDS[]` and use another as the "stolen/cloned" card for Scenario 3.

**What the simulation hides:** Real reader range, antenna placement inside the printed gate, and 3.3V-only power (never 5V) are bench-only concerns.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
