# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-PowerBox)

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
sensor, tracked separately from `Fan_Relay/` and `Lights_Relay/`.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] `ModbusIP_ESP8266` library installed
- [x] Both `Fan_Relay/` and `Lights_Relay/` bench-tested standalone
      first (per their own Build_Log.md checklists) — don't debug WiFi/
      Modbus and hardware problems at the same time

## Status
- [x] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_PowerBox_Node.ino`
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.14`
- [x] From a laptop on the same subnet, a Modbus TCP client can write
      Coil 0 (`LIGHTS_ON_CMD`) and see the light turn on
- [x] Write Coil 1 (`FAN_CMD`) and see the fan relay respond

- [x] `HEARTBEAT` (IR0) counts up ~1/second; `RECONNECT_COUNT` (IR1) reads 0 after boot and 1 after a Wi-Fi drop and recovery

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Phase B firmware written (WiFi static IP 192.168.50.14 + Modbus TCP via ModbusIP_ESP8266), same register map as the system doc. First written Phase B sketch for this node; the authority rule uses an onSetCoil callback so a fresh write is honored even if the value is unchanged. Watchdog detects WiFi link loss only. NOT yet flashed -- checklist above still open. |
| 2026-10-06 | **Phase B bench-confirmed.** `ping 192.168.50.14` showed the address free; flashed; Serial showed `Connected. IP address: 192.168.50.14`; compiled first time (the `onSetCoil` callbacks were accepted by the library). `Test-NetConnection` port 502 succeeded. Coil0/Coil1 writes switched lights and fan, and `Status` (DI0/DI1) followed. Local buttons flipped each output without any laptop command, and a fresh `LightsOff`/`FanOff` write took control back. With the router off, both outputs went OFF after ~5s and the buttons kept working; after the router returned, heartbeat kept counting and `RECONNECT_COUNT` read 1. Nothing needed changing after the flash. |

## Lessons Learned
- The PowerShell test functions are per-node: PowerBox's registers differ from Fire Alarm's (DI0/DI1 are lights/fan status, IR0/IR1 are heartbeat/reconnects), so the Fire Alarm `Regs`/`Inputs` helpers read the wrong layout here.
- OpenPLC writes coils continuously in Phase C, which will repeatedly re-assert the hub's value and can undo a button override; check this when the PowerBox ladder logic is written.

## Troubleshooting Tips
