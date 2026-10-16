# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-Door)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
component, tracked separately from the five sensor/actuator folders.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] `ModbusIP_ESP8266`, `ESP32Servo`, and
      `MFRC522` (miguelbalboa/rfid) libraries installed
- [ ] All five components (RC522 reader, servo, RGB LED, IR sensor, reed
      switch) bench-tested standalone first (per their own Build_Log.md
      checklists) — this node has the most moving parts of any system,
      don't debug hardware and network problems at the same time
- [x] Real card code(s) already in `AUTHORIZED_CARDS[]`

## Status
- [x] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_Door_Node.ino`
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.13`
- [x] Badging an authorized card sets `ACCESS_GRANTED` (DI2) and opens the
      gate
- [x] `DOOR_POSITION` (DI1) tracks the actual reed switch state, not just
      the commanded state (confirmed by Luis on the bench)
- [x] `IR_DETECTED` (DI0) tracks the IR sensor independently (confirmed by Luis on the bench)
- [x] From a laptop on the same subnet, a Modbus TCP client can write
      Coil 0 (`DOOR_OPEN_CMD`) and open the gate **with no card badge** —
      confirming the vulnerability this node is built to demonstrate
- [x] Write Coil 1 (`DOOR_CLOSE_CMD`) and confirm it force-closes
- [x] `HEARTBEAT` (IR0) counts up ~1/second; `RECONNECT_COUNT` (IR1) reads 0 after boot and 1 after a Wi-Fi drop and recovery

## Door Phase B (written 2026-10-09, bench-confirmed 2026-10-09)
Phase B firmware is in place at `Firmware/BSH-Door/BSH_Door_Node/BSH_Door_Node.ino`: Modbus TCP slave at 192.168.50.13, non-blocking gate state machine, momentary coils, DI2 pulse, WiFi watchdog (gate closes after 5 s of link loss), heartbeat/reconnect counters. Phase A sketch archived in `Archive/Door_PhaseA_sketches/`. Tick the Status boxes above only from real laptop results.

## Date log
| Date | What happened |
|---|---|
| 2026-10-09 | **Phase B bench-confirmed at 192.168.50.13** from PowerShell raw-Modbus tests, confirmed by Luis: port 502 open, HEARTBEAT counting, DOOR_OPEN_CMD opens the gate with no card (ACCESS_GRANTED stays 0 -- the weakness this node exists to show), DOOR_CLOSE_CMD closes it, a card badge raises ACCESS_GRANTED (DI2) for about 2 s, and the offline/watchdog and reset checks passed per Luis. IR_DETECTED (DI0) and DOOR_POSITION (DI1) also confirmed by Luis on the bench. Note for the record: the laptop capture he pasted showed both held at 1 for the whole run (his sensor setup), so the transitions were confirmed by Luis directly, not by that capture. |

## Lessons Learned

## Troubleshooting Tips
