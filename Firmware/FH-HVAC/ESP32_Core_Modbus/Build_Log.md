# ESP32 Board + WiFi + Modbus TCP -- Build Log (FH-HVAC)

Phase B started 2026-10-07 at Luis's request. `FH_HVAC_Node/FH_HVAC_Node.ino` is now the Phase B sketch
(WiFi + Modbus), written and logic-tested off-board; **flashed and bench-confirmed 2026-10-07 (see Date log).**
The Phase A sketch is preserved in full in `../SYSTEM_REPORT.md`. Decisions:

- Static IP **192.168.50.21**, gateway 192.168.50.1 (network SCT5; the checklist's older
  192.168.1.21 is stale -- project moved to 192.168.50.x on Oct 6, 2026). Ping before first use.
- Scaling: `TEMP_C` (IR0) and `TARGET_TEMP_C` (HR0) both in tenths of a degree C.
- Fail direction: fail-off. On comms loss `HVAC_ENABLE` is treated as clear and the relay opens.
- Registers: IR0 TEMP_C, IR1 HUMIDITY_PCT, IR2 HEARTBEAT, IR3 RECONNECT_COUNT, DI0 COMPRESSOR_RUNNING,
  DI1 HVAC_FAULT, HR0 TARGET_TEMP_C, Coil0 HVAC_ENABLE. OpenPLC counts: DI 2, coil 1, IR 4, HR 1.

## Prerequisites
- [x] Libraries installed: `ModbusIP_ESP8266` (plus the DHT libraries from Phase A)
- [x] Phase A fault test done (pull the DHT11 data wire -> `HVAC_FAULT` latches) and no board resets when the motor switches
- [x] Laptop on the SCT5 network; **ping 192.168.50.21 BEFORE flashing** -- a reply means an address collision (router DHCP pool overlaps)

## Status
- [x] Phase A complete and `SYSTEM_REPORT.md` written
- [x] WiFi + Modbus added, Serial prints `192.168.50.21`
- [x] Laptop Modbus client: IR0/IR1 live, HR0 write changes relay, Coil0 clear forces relay open
- [x] Unplug sensor -> DI1 latches
- [x] Cut network -> relay opens (fail-off)
- [x] HEARTBEAT counts ~1/s; RECONNECT_COUNT 0 after boot

## Date log
| Date | What happened |
|---|---|
| 2026-10-07 | Placeholder with the pre-settled decisions. |
| 2026-10-07 | Phase B sketch written: WiFi (SCT5, static 192.168.50.21) + ModbusIP_ESP8266 with the full register map, HEARTBEAT/RECONNECT_COUNT, fail-off on fault / `HVAC_ENABLE` clear / WiFi down >10 s, last-good `TEMP_C` held on fault. Off-board logic test passed (HR0 and Coil0 writes drive the relay, comms-loss fail-off, reconnect count, fault latch). Not flashed yet. Known limit: comms loss is detected from the WiFi link only, not from a master that stops polling. |
| 2026-10-07 | **Phase B bench-confirmed at 192.168.50.21.** Flashed and tested from PowerShell with raw Modbus frames: port 502 open; IR0/IR1 live and matching Serial; HEARTBEAT counts ~1/s with RECONNECT_COUNT 0 after boot; HR0 write below the room temperature closes the relay (LED blue, DI0=1) and above it opens it; clearing Coil0 forces the relay open (LED yellow) regardless of the setpoint; router off for ~10 s fails the unit off and it resumes on rejoin with RECONNECT_COUNT 1; reset returns both counters to 0 and HR0 to 24.0 C; unplugging the DHT11 latches DI1, opens the relay and TEMP_C holds its last good value. No resets when the motor relay switches. Known limit unchanged: comms loss is detected from the WiFi link only. |

## Known limitation (recorded 2026-10-07)
Comms loss is detected from the WiFi link only (down > 10 s -> HVAC_ENABLE treated as clear, relay opens).
If WiFi stays up but the Modbus master (OpenPLC) stops polling, the node cannot tell and keeps its last
setpoint and enable value. Deferred on purpose: add a "no Modbus request for N seconds" check in/after
Phase C, once OpenPLC's polling period is known (today only a laptop script polls).

## Lessons Learned

## Troubleshooting Tips
