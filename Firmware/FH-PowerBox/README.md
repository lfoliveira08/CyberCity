# FH-PowerBox -- Build Folder

Founder's Hall mirror of BSH-PowerBox (slot 4): same relays, buttons and pins, one static IP different.

> **Status (Oct 7, 2026): Phase B complete** (bench-confirmed at 192.168.50.24; relay boards active-HIGH). Built ahead of the BSH build order at Luis's request. No Wokwi folder: FH reuses `Firmware/BSH-PowerBox/Wokwi/` as-is. FH's standalone Phase A was never logged separately.

## Where this node's files are
- **`FH_PowerBox_Node/FH_PowerBox_Node.ino`** -- the firmware (WiFi + Modbus, 192.168.50.24).
- **`Firmware/FH_PowerBox_Bringup_Checklist.md`** (one level up) -- the bring-up procedure.
- **`FH_Systems/04_FH-PowerBox.md`** -- IP, OpenPLC container (192.168.50.124), register map.

## Subfolders
| Folder | What |
|---|---|
| `FH_PowerBox_Node/` | The firmware |
| `Lights_Relay/` | Lights relay wiring + log (copied from BSH) |
| `Fan_Relay/` | Fan relay wiring + log (copied from BSH) |
| `ESP32_Core_Modbus/` | Board, WiFi and Modbus Build_Log |

## Watch out
- Serial must say `.24`, not `.14`.
- BSH's relay boards were active-HIGH; FH's boards must be checked on their own.
