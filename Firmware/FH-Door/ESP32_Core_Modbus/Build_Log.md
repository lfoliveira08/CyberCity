# ESP32 Board + WiFi + Modbus TCP — Build Log (FH-Door)

> **Phase B bench-confirmed 2026-10-09** at 192.168.50.23. Mirror of `Firmware/BSH-Door/ESP32_Core_Modbus/Build_Log.md`.

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
component, tracked separately from the five sensor/actuator folders.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] `ModbusIP_ESP8266`, `ESP32Servo`, and
      `MFRC522` (miguelbalboa/rfid) libraries installed
- [x] All five components (RC522 reader, servo, RGB LED, IR sensor, reed
      switch) bench-tested standalone first (per their own Build_Log.md
      checklists) — this node has the most moving parts of any system,
      don't debug hardware and network problems at the same time
- [x] Real card code(s) already in `AUTHORIZED_CARDS[]`

## Status
- [x] Real WiFi SSID/password filled into a **local-only** copy of
      `FH_Door_Node.ino`
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.23`
- [x] Badging an authorized card sets `ACCESS_GRANTED` (DI2) and opens the
      gate
- [x] `DOOR_POSITION` (DI1) tracks the actual reed switch state, not just
      the commanded state
- [x] `IR_DETECTED` (DI0) tracks the IR sensor independently
- [x] From a laptop on the same subnet, a Modbus TCP client can write
      Coil 0 (`DOOR_OPEN_CMD`) and open the gate **with no card badge** —
      confirming the vulnerability this node is built to demonstrate
- [x] Write Coil 1 (`DOOR_CLOSE_CMD`) and confirm it force-closes
- [x] `HEARTBEAT` (IR0) counts up ~1/second; `RECONNECT_COUNT` (IR1) reads 0 after boot and 1 after a Wi-Fi drop and recovery

## Door Phase B (written 2026-10-09, bench-confirmed 2026-10-09)
Phase B firmware is in place at `Firmware/FH-Door/FH_Door_Node/FH_Door_Node.ino`: Modbus TCP slave at 192.168.50.23, non-blocking gate state machine, momentary coils, DI2 pulse, WiFi watchdog (gate closes after 5 s of link loss), heartbeat/reconnect counters. Phase A sketch archived in `Archive/Door_PhaseA_sketches/`. Tick the Status boxes above only from real laptop results.

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | **Phase B bench-confirmed at 192.168.50.23**, all tests confirmed by Luis: Serial showed the .23 address, the FH card (3D D2 C6 62) badged in and opened the gate, port 502 open, HEARTBEAT counting, DOOR_OPEN_CMD opened the gate with no card (ACCESS_GRANTED stayed 0), DOOR_CLOSE_CMD closed it, ACCESS_GRANTED pulsed after a badge, IR_DETECTED and DOOR_POSITION tracked their sensors, WiFi-loss watchdog and reconnect count (1) and reset (0) passed. Results reported by Luis; no raw capture was pasted for this node. |

## Lessons Learned

## Troubleshooting Tips
