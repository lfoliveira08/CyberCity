# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-BackupPower)

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
sensor, tracked separately from the four component folders.

## Prerequisites
- [ ] Dedicated project WiFi AP already broadcasting
- [ ] `ModbusIP_ESP8266` installed (no other third-party library needed
      for this node)
- [ ] All four components (`Toggle_Switch/`, `ATS_Relay/`,
      `Battery_Voltage_Divider/`, `ACS712_Current_Sensor/`) bench-tested
      standalone first — don't debug WiFi/Modbus and hardware/calibration
      problems at the same time

## Status
- [ ] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_BackupPower_Node.ino`
- [ ] Flashed, Serial Monitor shows `Connected. IP address: 192.168.1.15`
- [ ] From a laptop on the same subnet, a Modbus TCP client can read
      DI0/DI1 and IR0/IR1 and see them track the real hardware
- [ ] Same client can write Coil 0 (`ATS_TRANSFER_CMD`) and see the relay
      respond
- [ ] Disconnecting WiFi with `ATS_TRANSFER_CMD` active confirms it
      defaults to not-commanded once the watchdog timeout elapses

## Date log
| Date | What happened |
|---|---|
| | |

## Lessons Learned

## Troubleshooting Tips
