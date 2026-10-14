# BSH-FireAlarm — Build Folder

This folder holds the **physical build** documentation for the BSH-FireAlarm
node, organized one subfolder per sensor/component — matching how the parts
are laid out on the bench, so it's easy to find "what does this wire go to"
and "what have I already tested" while building.

This is a companion to, not a replacement for, the two master files one
level up in `Firmware/`:
- **`BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`** (in this folder — see
  `Firmware/WORKFLOW.md` Section 8 for the canonical layout) — the actual
  firmware to flash. Pin numbers
  and register names here all trace back to that file.
- **`BSH_FireAlarm_PILOT_Bringup.md`** — the full Phase A/B/C bring-up
  procedure (standalone hardware -> Modbus verification -> OpenPLC/SCADA
  integration). The per-sensor checklists below are Phase A of that
  document, broken out sensor-by-sensor instead of as one combined step.

Also see `BSH_Systems/02_BSH-FireAlarm.md` (this node's full system
doc: IP, register map, role) and the individual sensor reference docs in
`BSH_Sensors/` (02, 04, 05) — those are the general "how this sensor
works" docs shared across nodes; the files in each subfolder here are the
**BSH-FireAlarm-specific** wiring notes and your own build/test log.

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `Active_Buzzer/` | Active buzzer | Wiring notes + build/test log |
| `RGB_LED/` | Common-cathode RGB LED | Wiring notes + build/test log |
| `Push_Button/` | Pull-station push button | Wiring notes + build/test log |
| `ESP32_Core_Modbus/` | ESP32 board itself + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) — not tied to one sensor |

Each `Build_Log.md` has three sections: a checklist (check items off as you
go), a **Lessons Learned** section, and a **Troubleshooting Tips** section —
per the project's Integration Strategy, fill those in as you go, not just
at the end.

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real hardware
1. `Active_Buzzer/` and `RGB_LED/` and `Push_Button/` — wire and bench-test
   each in isolation (Phase A, per-sensor)
2. `ESP32_Core_Modbus/` — join WiFi, verify as a Modbus TCP slave from a
   laptop tool (Phase B)
3. Back to `BSH_FireAlarm_PILOT_Bringup.md` Phase C — OpenPLC + SCADA
   integration (this part isn't per-sensor, so it stays in that master doc)

Once BSH-FireAlarm is fully built and tested, this same folder pattern gets
reused for the next system (Elevator), per the project's chosen build
order (Fire Alarm -> Elevator -> Door -> Power Box) — see `Firmware/WORKFLOW.md`.
