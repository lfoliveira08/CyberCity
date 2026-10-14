# BSH-Perimeter — Build Folder

Physical build documentation for the BSH-Perimeter node, one subfolder per
component — same convention as `Firmware/BSH-FireAlarm/`. This folder
**replaces** `Firmware/BSH-AirQuality/`, which is being retired (see
`Piranga_Workflow_v2.md` Section 14 for the swap rationale — Air Quality
is being folded out so a future Fire Alarm upgrade can absorb flame/smoke/
gas detection properly, and Perimeter/Motion takes this system's slot
instead).

Companion to, not a replacement for:
- **`BSH_Perimeter_Node/BSH_Perimeter_Node.ino`** — the firmware to flash. **Not in this folder
  yet:** this node's first-pass sketch currently exists only in the Claude
  project, not on local disk. Copy it into `BSH_Perimeter_Node/` when this system
  becomes active, per `Firmware/WORKFLOW.md` Section 8.
- **`BSH_Systems/05_BSH-Perimeter.md`** — full system doc (IP,
  register map, role).
- **`BSH_Sensors/13_PIR_Motion_Sensor.md`** and
  **`14_HC-SR04_Ultrasonic_Sensor.md`** — general component reference docs.

## What's different about this node vs. the one it replaces

Unlike the single-sensor AirQuality node, this node has **two** independent
sensors feeding two independent Modbus points — deliberately not fused
into one "intrusion" bit locally (see the DESIGN NOTE at the top of
`BSH_Perimeter_Node.ino`). It's still a no-actuator, sensor-only node like
AirQuality was, so it stays one of the simpler five to bring up.

**Read this before wiring the ultrasonic sensor:** its ECHO pin outputs
5V, which is not safe to wire directly into an ESP32 GPIO. A voltage
divider (1kΩ / 2kΩ) is required — full detail in
`HC-SR04_Ultrasonic_Sensor/Wiring.md`. The PIR sensor's output needs no
such divider (its module regulates its own logic-level output).

## Subfolders

| Folder | Component | What's in it |
|---|---|---|
| `Wokwi/` | Wokwi simulation of the whole system (Phase A0) | `diagram.json` + `sketch.ino` + a guide — **start here**, before any real wiring |
| `PIR_Motion_Sensor/` | PIR motion sensor | Wiring notes + build/test log |
| `HC-SR04_Ultrasonic_Sensor/` | Ultrasonic ranger | Wiring notes + build/test log — **read the voltage-divider note before wiring** |
| `ESP32_Core_Modbus/` | ESP32 board + WiFi/Modbus | Board bring-up, WiFi join, Modbus TCP verification (Phase B) |

## Suggested build order for this node

0. `Wokwi/` — run the simulation first (Phase A0, `Firmware/WORKFLOW.md`
   Section 7) to learn each part and its wiring before touching real hardware
1. `PIR_Motion_Sensor/` — simplest, get a clean digital motion trigger over
   Serial first
2. `HC-SR04_Ultrasonic_Sensor/` — wire the voltage divider *before*
   connecting ECHO to the ESP32, then confirm sane distance readings over
   Serial
3. `ESP32_Core_Modbus/` — WiFi join + Modbus TCP verification (Phase B).
   Pay attention to whether the ultrasonic sensor's bounded `pulseIn()`
   read causes any Modbus responsiveness issues in practice — flagged as
   an open question in the firmware's BOUNDED-BLOCKING NOTE
4. OpenPLC + SCADA integration (Phase C) — follow the same pattern as
   `Firmware/BSH_FireAlarm_PILOT_Bringup.md` Phase C, substituting this
   node's IP (192.168.1.15) and register map

Once BSH-Perimeter is built and tested, all 5 BSH systems are done — the
same pattern then repeats for the Founder's Hall mirrors, starting with
FH-FireAlarm, per the project's chosen build order (Fire Alarm -> Elevator -> Door -> Power Box -> Perimeter/Motion).
