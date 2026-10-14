# FH-HVAC -- Register map and address translation

> DRAFT Oct 11, 2026. Source of truth for the ESP32 side: `FH_Systems/01_FH-HVAC.md`. If this file and that doc disagree, the doc wins.

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | COMPRESSOR_RUNNING | Relay/fan output state, reported honestly |
| Discrete Input | DI1 | HVAC_FAULT | Sensor timeout or out-of-range reading |
| Coil | Coil0 | HVAC_ENABLE | Forces unit off while clear |
| Input Register | IR0 | TEMP_C | Raw temperature |
| Input Register | IR1 | HUMIDITY_PCT | Raw humidity |
| Input Register | IR2 | HEARTBEAT | Counts up once a second; frozen = node lost |
| Input Register | IR3 | RECONNECT_COUNT | WiFi rejoins since boot |
| Holding Register | HR0 | TARGET_TEMP_C | Master-set setpoint |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `FH-HVAC-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.21 / 502 |
| Slave ID | TBD -- confirm in Phase C (record the value that works in `Build_Log.md`) |
| Discrete inputs | start 0, size 2 |
| Coils | start 0, size 1 |
| Input registers | start 0, size 4 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 1 |

This PLC itself: container `OpenPLC-FH-HVAC` at 192.168.50.121, Web UI port 8080, Modbus server port 502.

## 3. Address translation

**UNVERIFIED:** the slave-block addresses (`%IX100.x`, `%QX100.x`, `%IW100+`, `%QW100+`) are the OpenPLC v3 convention, and the current OpenPLC documentation does not restate them. Confirm each one on the Monitoring page and write the result in `Build_Log.md`.
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address (verify) | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 HVAC_ENABLE | %QX100.0 | `SCADA_HVAC_ENABLE` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| HR0 TARGET_TEMP_C | %QW100 | `SCADA_TARGET_TEMP_C` at %QW0 (SCADA writes) | holding register 0 (FC 6/16) |
| DI0 COMPRESSOR_RUNNING | %IX100.0 | `PUB_COMPRESSOR_RUNNING` at %QX0.1 | coil 1 (FC 1) |
| DI1 HVAC_FAULT | %IX100.1 | `PUB_HVAC_FAULT` at %QX0.2 | coil 2 (FC 1) |
| IR0 TEMP_C | %IW100 | `PUB_TEMP_C` at %QW1 | holding register 1 (FC 3) |
| IR1 HUMIDITY_PCT | %IW101 | `PUB_HUMIDITY_PCT` at %QW2 | holding register 2 (FC 3) |
| IR2 HEARTBEAT | %IW102 | `PUB_HEARTBEAT` at %QW3 | holding register 3 (FC 3) |
| IR3 RECONNECT_COUNT | %IW103 | `PUB_RECONNECT_COUNT` at %QW4 | holding register 4 (FC 3) |
