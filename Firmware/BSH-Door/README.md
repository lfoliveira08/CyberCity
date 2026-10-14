# BSH-Door — Build Folder

Physical build documentation for the BSH-Door (gate) node — same
convention as `Firmware/BSH-FireAlarm/`. Built third in the build order
(after Fire Alarm and Elevator) — it's the project's single highest-risk
node.

Companion to, not a replacement for:
- **`BSH_Door_Node/BSH_Door_Node.ino`** — the firmware to flash. Since Oct 9,
  2026 it holds the Phase A standalone sketch (no WiFi, no Modbus); it is
  replaced in place by the WiFi+Modbus version when Phase B starts, per
  `Firmware/WORKFLOW.md` Section 8. The FH mirror is
  `Firmware/FH-Door/FH_Door_Node/FH_Door_Node.ino`. Before flashing, set
  `IR_ACTIVE_LOW` and `REED_LOW_MEANS_CLOSED` from the raw values the sketch
  prints.
- **`BSH_Systems/03_BSH-Door.md`** — full system doc (IP, register
  map, role).
- **`BSH_Sensors/06_SG90_Servo.md`**, **`07_IR_Sensor.md`**,
  **`08_Reed_Switch.md`**, **`11_RFID_RC522_Reader.md`** — general
  component reference docs.

> **Sep 2026 update:** the card reader was swapped from Wiegand to an
> MFRC522/RC522 RFID module (SPI) — see
> `Piranga_Workflow_v2.md` Section 14 for the rationale. The
> `Wiegand_Card_Reader/` subfolder below has been replaced by
> `RFID_RC522_Reader/`.

> **Sep 17 2026 note:** `Stepper_Motor_Reference.ino` (kept only in the Claude
> project, not in this folder) is a
> user-written reference sketch (stepper motor + PIR trigger) saved for
> when Door work actually starts — it is **not** the current design and
> is **not** ready to flash as-is. It conflicts with the documented SG90
> servo actuator, uses blocking `delay()`/`step()` calls that would freeze
> Modbus polling, collides with the RFID reader's SPI pins (GPIO 18/19),
> and triggers on plain motion rather than RFID access. See the header
> comment in that file for the full rundown before building from it.

## Current status (Oct 9, 2026)

**Phase B bench-confirmed (Oct 9, 2026)** at 192.168.50.13. The sketch follows the
pin map (RC522 on 18/19/23/32/33, IR on 14, reed on 34, servo on 13, RGB LED on
25/26/27, no buzzer). The bench checks below were confirmed by Luis:

- [x] `IR_ACTIVE_LOW` (default `true`) matches the real IR module (confirmed by Luis)
- [x] `REED_LOW_MEANS_CLOSED` (default `true`) matches the real reed module (confirmed by Luis)
- [x] The reed part is a 3-pin module (S/V/G), not the bare switch in the
      plan -- pull-up confirmed working by Luis (see
      `Reed_Switch/Wiring.md`)
- [x] Wokwi simulation (Phase A0) -- built and run Oct 9, 2026 in `Wokwi/` (Luis: looks fine)
- [x] `SYSTEM_REPORT.md` -- finalized Oct 9, 2026 with the Phase B bench results

## Why this node is flagged high-risk, and what's already flagged

Door was originally built last, after four other nodes had proven the
Modbus/watchdog/state-machine pattern — because Wiegand had the least
precedent of any library in the project. That's less true now that the
reader is MFRC522 (one of the most common, best-supported ESP32 RFID
libraries), and as of Sep 23, 2026 Door is built third (Fire Alarm ->
Elevator -> Door -> Power Box — see
`Firmware/WORKFLOW.md`). Two things are already known and documented in
the firmware/wiring docs, not left for you to discover the hard way:

1. **GPIO 34 (reed switch) has no usable internal pull-up.** It's one of
   the ESP32's input-only ADC pins (34-39), which ship with no internal
   pull resistors in silicon. `INPUT_PULLUP` on this pin compiles but does
   nothing — an external ~10kΩ pull-up resistor is required. Full detail
   in `Reed_Switch/Wiring.md`.
2. **The design deliberately does not judge "intrusion" locally.** The
   firmware reports `IR_DETECTED`, `DOOR_POSITION`, and `ACCESS_GRANTED`
   as three honest, independent values — correlating them (e.g. approach
   with no matching access) is the SCADA layer's job per Section 7 of the
   workflow doc, not something built into this ESP32.

A third thing worth flagging up front for the *new* reader specifically:
the MFRC522 module is a **3.3V-only part** — verify VCC is wired to the
ESP32's 3.3V pin, never 5V, before powering it up.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `RFID_RC522_Reader/` | Access card reader | Wiring notes + build/test log (build this one first) |
| `SG90_Servo/` | Gate arm actuator | Wiring notes + build/test log |
| `RGB_LED/` | Status indicator (green=closed, red=open) | Wiring notes + build/test log |
| `IR_Sensor/` | Approach detection | Wiring notes + build/test log |
| `Reed_Switch/` | Actual position feedback | Wiring notes + build/test log — **read the pull-up note before wiring** |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) |

`Stepper_Motor_Reference.ino` is not a build-order item, just a saved
reference that lives only in the Claude project; see the Sep 17 note above.

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real hardware
1. `RFID_RC522_Reader/` first, standalone, over Serial only — get a real
   card's UID printed out and confirm you can read it reliably *before*
   wiring anything else. Double-check the reader's VCC is 3.3V, not 5V.
2. `RGB_LED/` and `IR_Sensor/` — simple GPIO, same patterns already proven
   on Fire Alarm/other nodes
3. `Reed_Switch/` — wire the external pull-up per that folder's notes
4. `SG90_Servo/` — external 5V supply, same brownout caution as every
   other servo in this project
5. Put your real card's UID into `AUTHORIZED_CARDS[]` in
   `BSH_Door_Node.ino` (done Oct 9, 2026: `{0xB3, 0x12, 0x87, 0x2D}`;
   confirm it against a bench read)
6. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (Phase B)
7. OpenPLC + SCADA integration (Phase C) — follow the same pattern as
   `Firmware/BSH_FireAlarm_PILOT_Bringup.md` Phase C, substituting this
   node's IP (192.168.50.13) and register map

Once BSH-Door is built and tested, this pattern moves on to Power Box,
per the project's chosen build order (Fire Alarm -> Elevator -> Door -> Power Box).
