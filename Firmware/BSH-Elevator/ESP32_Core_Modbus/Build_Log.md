# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-Elevator)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
component, tracked separately from the three sensor/actuator folders.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] `ModbusIP_ESP8266`, `Adafruit SSD1306` and `Adafruit GFX` libraries installed (the display is an OLED, not the older I2C LCD)
- [x] Push Buttons, I2C LCD, and L293D Motor Driver all bench-tested
      standalone first (per their own Build_Log.md checklists)

## Status
- [x] Real WiFi SSID/password filled into
      `BSH_Elevator_Node.ino` (2026-09-30) -- network: Linksys24 at the time; changed to SCT5 on 2026-10-05
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.11`
- [x] From a laptop on the same subnet, a Modbus TCP client can write
      Coil 0/1/2 (`CALL_FLOOR_1/2/3`) and see the elevator move — and see
      the coil self-clear afterward (momentary, not held)
- [x] `ELEVATOR_MOVING` (DI0) reads true only while actually traveling
- [x] `CURRENT_FLOOR` (IR0) updates once travel completes
- [x] Write Coil 3 (`ELEVATOR_DISABLE`) = 1 and confirm floor calls are
      ignored while it's set
- [ ] (Cannot be triggered in this sketch: the normal arrival timeout always fires before the fault timer, so DI1 stays 0. Not tested; revisit if the fault logic changes.) confirm `ELEVATOR_FAULT`
      (DI1) is a genuinely separate condition from `ELEVATOR_DISABLE` — see
      the system doc's notes on why these must stay distinguishable

- [x] `HEARTBEAT` (IR1) counts up ~1/second; `RECONNECT_COUNT` (IR2) reads 0 after boot and 1 after a Wi-Fi drop and recovery

## Date log
| Date | What happened |
|---|---|
| 2026-09-30 | Phase B firmware written: WiFi (real SSID/password filled in) + Modbus TCP (`ModbusIP_ESP8266`) added on top of the confirmed-working Phase A sketch, same register map as `BSH_Systems/01_BSH-Elevator.md`. Also picked up the distance-scaled travel-time fix from the project's original Phase B template (`expectedTravelMs = floorsToTravel * FLOOR_TRAVEL_MS`), which resolves the "every floor call takes the same time regardless of distance" gap flagged as open in `SYSTEM_REPORT.md`. `FLOOR_TRAVEL_MS` also halved (6000 -> 3000ms) per request. Not yet flashed to real hardware -- remaining checklist items below still need the board on the actual project WiFi AP. |
| 2026-10-07 | **Phase B bench-confirmed at 192.168.50.11.** Flashed (OLED libraries installed first). Laptop PowerShell raw-Modbus tests all passed: port 502 open, CURRENT_FLOOR/HEARTBEAT/RECONNECT_COUNT live, CALL_FLOOR coils move the car (door 3 s, 3 s per floor) with ELEVATOR_MOVING true only while travelling and the coil self-clearing, repeat call to the current floor ignored, calls during a move dropped, ELEVATOR_DISABLE blocks new calls, local buttons work, router off/on gives RECONNECT_COUNT 1, reset zeroes both counters. Known limits: ELEVATOR_FAULT (DI1) cannot latch in this sketch; ELEVATOR_DISABLE blocks new calls only and does not stop a move in progress; no fail-off on WiFi loss. |

## Lessons Learned

## Troubleshooting Tips
