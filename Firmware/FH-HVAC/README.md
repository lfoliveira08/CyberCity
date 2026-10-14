# FH-HVAC -- Build Folder

Founder's Hall slot 1: HVAC zone unit (DHT11 on the bench, DHT22 in Wokwi; relay + DC motor fan; RGB LED). No BSH counterpart.

- Firmware: `FH_HVAC_Node/FH_HVAC_Node.ino` (Phase B, WiFi + Modbus at 192.168.50.21; bench-confirmed 2026-10-07)
- System doc: `FH_Systems/01_FH-HVAC.md`; checklist: `Firmware/FH_HVAC_Bringup_Checklist.md`

Design decisions (2026-10-07): tenths-of-a-degree scaling; fail-off on comms loss.

| Folder | What |
|---|---|
| `Wokwi/` | Phase A0 simulation -- start here |
| `DHT11_Sensor/` | Sensor wiring, log, BenchTest |
| `RGB_LED/` | Status LED wiring, log, BenchTest |
| `Relay_Module/` | Relay wiring, polarity test, log, BenchTest |
| `ESP32_Core_Modbus/` | Phase B log (complete, bench-confirmed) + `fh_hvac_modbus_check.py` (read/write the registers from a PC: `--ip 192.168.50.21 --watch`, `--target`, `--enable`) |

Suggested order: Wokwi -> RGB_LED -> DHT11_Sensor -> Relay_Module -> combined `FH_HVAC_Node.ino`
-> unplug DHT11 to prove the fault path -> `SYSTEM_REPORT.md`.
