# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-Perimeter)

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
component, tracked separately from the two sensor folders.

## Prerequisites
- [ ] Dedicated project WiFi AP already broadcasting
- [ ] `ModbusIP_ESP8266` library installed
- [ ] Both sensors (PIR, HC-SR04) bench-tested standalone first (per their
      own Build_Log.md checklists)

## Status
- [ ] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_Perimeter_Node.ino`
- [ ] Flashed, Serial Monitor shows `Connected. IP address: 192.168.1.15`
- [ ] `MOTION_DETECTED` (DI0) tracks the PIR sensor live over Modbus
- [ ] `OBJECT_IN_RANGE` (DI1) and `DISTANCE_CM` (IR0) track the ultrasonic
      sensor live over Modbus
- [ ] Writing Coil0 (`ZONE_DISABLE`) forces both discrete inputs false
      while comms stay live
- [ ] Disconnecting WiFi (comm loss) causes `ZONE_DISABLE` to be ignored —
      confirm the discrete inputs go back to reflecting the raw sensors
      rather than staying forced false
- [ ] No observed Modbus timeout/slave-not-responding symptoms during
      normal ultrasonic sampling — the open question flagged in the
      firmware's BOUNDED-BLOCKING NOTE

## Date log
| Date | What happened |
|---|---|
| | |

## Lessons Learned

## Troubleshooting Tips
