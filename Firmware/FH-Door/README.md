# FH-Door -- Build Folder

Physical build documentation for the **FH-Door** (gate) node (Founder's Hall,
slot 3), organized one subfolder per component -- the same layout as
`Firmware/BSH-Door/`, so nothing has to be re-learned between buildings.

> **Status (Oct 9, 2026): Phase B bench-confirmed** at 192.168.50.23, all tests
> confirmed by Luis. Built ahead of the BSH build order. Phase C has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

## Where this node's files are

- **`FH_Door_Node/FH_Door_Node.ino`** -- this node's firmware. It is the
  Phase B sketch (WiFi + Modbus) at static IP **192.168.50.23**, not BSH's
  `.13`; the Phase A sketch is archived. The card UID in `AUTHORIZED_CARDS[]`
  (3D D2 C6 62) was confirmed as FH's during the Phase B test.
- **`Wokwi/`** -- the Door simulation, copied from `../BSH-Door/Wokwi/` (the BSH
  copy was run on Oct 9, 2026 and looked fine).
- **`SYSTEM_REPORT.md`** -- final report with the Phase B bench results (Oct 9, 2026).
- **`Firmware/FH_Door_Bringup_Checklist.md`** (one level up) -- the Phase A/B/C
  bring-up procedure for this node.
- **`FH_Systems/03_FH-Door.md`** -- the system doc: IP (192.168.50.23), OpenPLC
  container (192.168.50.123), register map, role.
- **`FH_Sensors/`** (05, 06, 07, 08, 11) -- the general "how this component
  works" reference sheets for Founder's Hall.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `FH_Door_Node/` | The node's firmware | The `.ino` itself (canonical path, `Firmware/WORKFLOW.md` Section 8) |
| `Wokwi/` | Simulation (Phase A0) | `diagram.json`, `sketch.ino`, `libraries.txt`, `README.md` |
| `RFID_RC522_Reader/` | Access card reader (SPI) | Wiring notes + build/test log |
| `SG90_Servo/` | Gate arm actuator | Wiring notes + build/test log |
| `RGB_LED/` | Status indicator (green = closed, red = open or denied) | Wiring notes + build/test log |
| `IR_Sensor/` | Approach detection | Wiring notes + build/test log |
| `Reed_Switch/` | Actual position feedback | Wiring notes + build/test log -- read the pull-up note before wiring |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Build log only (Phase B) |

## How this differs from BSH-Door

Nothing electrically -- same components, same GPIO map, same register
semantics. Only these things change:

| | BSH-Door | FH-Door |
|---|---|---|
| ESP32 static IP (Phase B) | 192.168.50.13 | **192.168.50.23** |
| OpenPLC instance | `OpenPLC-BSH-Door` | `OpenPLC-FH-Door` |
| OpenPLC container IP | 192.168.50.113 | **192.168.50.123** |
| Authorized card list | `B3 12 87 2D` | **`3D D2 C6 62`** (confirm it is FH's) |

The two nodes are kept as separate folders and separate `.ino` files rather
than one shared sketch, because the differences are hardcoded values (an
address, a card list) and getting one wrong is silent until two nodes collide
or the wrong card opens the wrong gate. A Door node already shipped once with
the other building's IP.

Each `Wiring.md` and `Build_Log.md` here is a copy of BSH's, re-pointed at FH's
docs and IP, and carries a banner saying so. **If a wiring correction is found
while building either building, fix both copies.**

## Open checks (same as BSH-Door, both still open)

- [ ] `IR_ACTIVE_LOW` and `REED_LOW_MEANS_CLOSED` set from the real modules' raw values
- [ ] Reed module: does it have its own pull-up? (GPIO 34 has none internally)
- [ ] Card `3D D2 C6 62` confirmed as FH's, read back over Serial
- [ ] Each component bench-tested, then the combined test

GPIO 14 is already confirmed available on the FH board (2026-10-07).

## Suggested build order for this node

Follow `Firmware/FH_Door_Bringup_Checklist.md`. In outline: Wokwi (done, copied
from BSH) -> bench-test each component in isolation, reading BSH's `Build_Log.md`
for each one first (Phase A) -> combined standalone test with
`FH_Door_Node.ino` -> `ESP32_Core_Modbus/` for WiFi and Modbus (Phase B) ->
OpenPLC and SCADA (Phase C). Finalize `SYSTEM_REPORT.md` when Phase A completes,
per `Firmware/WORKFLOW.md` Section 4.
