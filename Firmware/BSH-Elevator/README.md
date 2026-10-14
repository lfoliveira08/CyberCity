# BSH-Elevator — Build Folder

Physical build documentation for the BSH-Elevator node — same convention
as `Firmware/BSH-FireAlarm/`.

Companion to, not a replacement for:
- **`BSH_Elevator_Node/BSH_Elevator_Node.ino`** (in this folder, per
  `Firmware/WORKFLOW.md` Section 8) — the firmware to flash. Phase A
  bench-confirmed 2026-09-25; Phase B bench-confirmed 2026-10-07 (192.168.50.11); see `SYSTEM_REPORT.md`.
- **`BSH_Systems/01_BSH-Elevator.md`** — full system doc (IP,
  register map, role).
- **`BSH_Sensors/01_L293D_Motor_Driver.md`**, **`02_Push_Button.md`**,
  **`03_I2C_LCD_Display.md`** — general component reference docs.

## Important — this firmware is a rewrite, not the original prototype

If you built anything from the original standalone `Elevator_Motor.ino`
prototype (no networking, no Modbus, a 7-segment display), **that's not
what `BSH_Elevator_Node.ino` uses.** The rewrite keeps the same
motor/button/LCD wiring role but drives movement as a non-blocking state
machine instead of `delay()`, and drops the 7-segment display entirely
(it's not in this node's documented BOM). If you already wired a
7-segment display from the old prototype, it's not connected to anything
in the current firmware — fine to leave unwired for this build.

See also `Wiring_Diagram.svg` (this folder) for a visual wiring reference covering all three components, including optional bench debug LEDs on their own dedicated pins (GPIO 23/25/27 -- mirrored in firmware, not tapped off the L293D lines).

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `L293D_Motor_Driver/` | Motor driver + DC motor (cab movement) | Wiring notes + build/test log |
| `Push_Buttons/` | 3x floor-call buttons | Wiring notes + build/test log |
| `I2C_LCD_Display/` | 16x2 LCD (status text) | Wiring notes + build/test log |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) |

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real hardware
1. `Push_Buttons/` — simplest, get all 3 reading correctly first
2. `I2C_LCD_Display/` — confirms I2C bus works before adding the motor
   driver's own wiring nearby
3. `L293D_Motor_Driver/` — the one component here needing an external
   motor supply; wire and bench-test last among the three, since a
   mis-wired motor driver is the easiest of the three to accidentally
   short something on
4. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (Phase B)
5. OpenPLC + SCADA integration (Phase C) — follow the same pattern as
   `Firmware/BSH_FireAlarm_PILOT_Bringup.md` Phase C, substituting this
   node's IP (192.168.50.11) and register map

Once BSH-Elevator is built and tested, this pattern moves on to Door, per
the project's chosen build order (Fire Alarm -> Elevator -> Door -> Power Box).
