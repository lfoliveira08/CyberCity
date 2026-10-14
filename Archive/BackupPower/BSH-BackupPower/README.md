# BSH-BackupPower — Build Folder

Physical build documentation for the BSH-BackupPower node, one subfolder
per component — same convention as `Firmware/BSH-FireAlarm/`. Replaces
`Firmware/BSH-Perimeter/`, which was retired (see `Piranga_Workflow_v2.md`
Section 14, Sep 28, 2026 — Perimeter/Motion was a pure sensor-only node
with no actuator; BackupPower gives the project a genuine ATS-relay
actuator and a real-world OT attack surface instead). The old folder's
preliminary PIR/HC-SR04 bench files are kept in `Archive/BSH-Perimeter/`.

Companion to, not a replacement for:
- **`BSH_BackupPower_Node/BSH_BackupPower_Node.ino`** — the firmware to
  flash. **Not written yet.** Create it in `BSH_BackupPower_Node/` when
  this system becomes active, per `Firmware/WORKFLOW.md` Section 8.
- **`BSH_Systems/05_BSH-BackupPower.md`** — full system doc (IP,
  register map, role).
- **`BSH_Sensors/17_Toggle_Switch.md`, `18_Voltage_Divider_Battery_Sense.md`,
  `19_ACS712_Current_Sensor.md`, `09_Relay_Module.md`** — general
  component reference docs.

## What's different about this node vs. the one it replaces

Unlike the sensor-only Perimeter node, this node has a genuine actuator
(the ATS transfer relay) alongside its inputs. Two of its four signals are
analog (`UPS_BATTERY_PCT`, `LOAD_CURRENT_MA`) and need real bench
calibration against whatever battery pack and demo load are actually used
— there is no fixed "correct" divider ratio the way there was a fixed pin
map for Perimeter's digital sensors. No new third-party library is needed
(`ModbusIP_ESP8266` + built-in `analogRead` only), so this node's risk is
almost entirely in the analog calibration, not the firmware.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `Toggle_Switch/` | Utility-power-loss toggle switch | Wiring notes + build/test log |
| `ATS_Relay/` | ATS transfer relay | Wiring notes + build/test log |
| `Battery_Voltage_Divider/` | UPS battery voltage divider | Wiring notes + build/test log — **calibrate against the real battery before trusting the percentage math** |
| `ACS712_Current_Sensor/` | Load current sensor | Wiring notes + build/test log — **read the 5V-swing wiring caution before wiring** |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) |

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real
   hardware
1. `Toggle_Switch/` — simplest, get a clean digital flip over Serial first
2. `ATS_Relay/` — plain digital output, confirm polarity against your
   module
3. `Battery_Voltage_Divider/` — verify against a multimeter reading of
   the real battery before trusting any scaling math
4. `ACS712_Current_Sensor/` — measure OUT swing with a multimeter first,
   per the wiring caution, then confirm it moves under load
5. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (Phase B),
   once all four signals work standalone

Once BSH-BackupPower is built and tested, this is the last of the five BSH
systems — Phase 2 (Founder's Hall) begins next, starting with
FH-FireAlarm, per the project's chosen build order (Fire Alarm -> Elevator
-> Door -> Power Box -> Backup Power).
