# FH-FireAlarm — Build Folder

Physical build documentation for the **FH-FireAlarm** node (Founder's Hall,
slot 2), organized one subfolder per component — the same layout as
`Firmware/BSH-FireAlarm/`, so nothing has to be re-learned between buildings.

> **Status (Oct 6, 2026): Phase B complete, bench-confirmed.** Built ahead of the
> BSH build order at Luis's request. `FH_FireAlarm_Node.ino` is the WiFi + Modbus
> version, a mirror of BSH-FireAlarm's sketch with a static IP of
> **192.168.50.22**. Phase C (OpenPLC + SCADA) has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

## Where this node's files are

- **`FH_FireAlarm_Node/FH_FireAlarm_Node.ino`** — this node's firmware. It is
  the WiFi + Modbus version, mirrored from BSH-FireAlarm (the standalone Phase A
  sketch it replaced is recorded in `BSH-FireAlarm/SYSTEM_REPORT.md`). Read its
  header before flashing: check the Serial output says `.22`, not `.12`.
- **`Firmware/FH_FireAlarm_Bringup_Checklist.md`** (one level up) — the Phase
  A/B/C bring-up procedure for this node.
- **`FH_Systems/02_FH-FireAlarm.md`** — the system doc: IP (192.168.50.22),
  OpenPLC container (192.168.50.122), register map, role.
- **`FH_Sensors/`** (02, 04, 05, 15, 16) — the general "how this component works"
  reference sheets for Founder's Hall.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `FH_FireAlarm_Node/` | The node's firmware | The `.ino` itself (canonical path, `Firmware/WORKFLOW.md` Section 8) |
| `Active_Buzzer/` | Active buzzer | Wiring notes + build/test log |
| `RGB_LED/` | Common-cathode RGB LED | Wiring notes + build/test log |
| `Push_Button/` | Pull-station push button | Wiring notes + build/test log |
| `Flame_Sensor/` | Digital flame sensor | Wiring notes + build/test log |
| `MQ-2_Gas_Smoke_Sensor/` | Analog gas/smoke sensor | Wiring notes + build/test log |
| `ESP32_Core_Modbus/` | The board itself + WiFi/Modbus | Build log only — nothing component-specific to wire |

No `Wokwi/` folder yet: FH mirrors BSH pin-for-pin, so this node reuses
`../BSH-FireAlarm/Wokwi/`. Copy it here when FH-FireAlarm becomes active, per
`Firmware/WORKFLOW.md` Section 7.

## How this differs from BSH-FireAlarm

Nothing electrically — same five components, same GPIO map, same register
semantics. Only three things change, and all three are network-layer:

| | BSH-FireAlarm | FH-FireAlarm |
|---|---|---|
| ESP32 static IP | 192.168.50.12 | **192.168.50.22** |
| OpenPLC instance | `OpenPLC-BSH-FireAlarm` | `OpenPLC-FH-FireAlarm` |
| OpenPLC container IP | 192.168.50.112 | **192.168.50.122** |

That is exactly why the two nodes are kept as separate folders and separate
`.ino` files rather than one shared sketch with a build flag: the only
difference is a single hardcoded address, and getting it wrong is silent until
two nodes collide on the network. It has already happened twice in this project
(FH-Door shipped with BSH-Door's `.13`; the retired FH-Elevator shipped with
BSH-Elevator's `.11`).

Each `Wiring.md` here is a copy of BSH's, re-pointed at FH's docs and IP, and
carries a banner saying so. **If a wiring correction is found while building
either building, fix both copies** — that is the standing cost of mirroring, and
it is cheaper than a shared file nobody is sure applies to the node in front of
them.

## Suggested build order for this node

When FH-FireAlarm becomes active, follow
`Firmware/FH_FireAlarm_Bringup_Checklist.md`. In outline: copy BSH's Wokwi
simulation and re-run it (Phase A0) → bench-test each component in isolation,
reading BSH's Build_Log for each one first (Phase A) → combined standalone test
with `FH_FireAlarm_Node.ino` → `ESP32_Core_Modbus/` for WiFi and Modbus
(Phase B) → OpenPLC and SCADA (Phase C). Write
`FH-FireAlarm/SYSTEM_REPORT.md` when Phase A completes, per
`Firmware/WORKFLOW.md` Section 4.
