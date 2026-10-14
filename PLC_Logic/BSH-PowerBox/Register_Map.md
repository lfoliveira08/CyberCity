# BSH-PowerBox -- Register map and address translation

> DRAFT Oct 11, 2026. Source of truth for the ESP32 side: `BSH_Systems/04_BSH-PowerBox.md`. If this file and that doc disagree, the doc wins.

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | LIGHTS_STATUS | Lights' actual physical state |
| Discrete Input | DI1 | FAN_STATUS | Fan's actual physical state |
| Coil | Coil0 | LIGHTS_ON_CMD | Lights on/off command |
| Coil | Coil1 | FAN_CMD | Fan on/off command |
| Input Register | IR0 | HEARTBEAT | Counts up once a second; frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `BSH-PowerBox-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.14 / 502 |
| Slave ID | TBD -- confirm in Phase C (record the value that works in `Build_Log.md`) |
| Discrete inputs | start 0, size 2 |
| Coils | start 0, size 2 |
| Input registers | start 0, size 2 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-BSH-PowerBox` at 192.168.50.114, Web UI port 8080, Modbus server port 502.

## 3. Address translation

**UNVERIFIED:** the slave-block addresses (`%IX100.x`, `%QX100.x`, `%IW100+`, `%QW100+`) are the OpenPLC v3 convention, and the current OpenPLC documentation does not restate them. Confirm each one on the Monitoring page and write the result in `Build_Log.md`.
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address (verify) | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 LIGHTS_ON_CMD | %QX100.0 | `SCADA_LIGHTS_ON_CMD` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| Coil1 FAN_CMD | %QX100.1 | `SCADA_FAN_CMD` at %QX0.1 (SCADA writes) | coil 1 (FC 5/15) |
| (computed) LIGHTS_MISMATCH | - | `PUB_LIGHTS_MISMATCH` at %QX0.2 | coil 2 (FC 1) |
| (computed) FAN_MISMATCH | - | `PUB_FAN_MISMATCH` at %QX0.3 | coil 3 (FC 1) |
| DI0 LIGHTS_STATUS | %IX100.0 | `PUB_LIGHTS_STATUS` at %QX0.4 | coil 4 (FC 1) |
| DI1 FAN_STATUS | %IX100.1 | `PUB_FAN_STATUS` at %QX0.5 | coil 5 (FC 1) |
| IR0 HEARTBEAT | %IW100 | `PUB_HEARTBEAT` at %QW0 | holding register 0 (FC 3) |
| IR1 RECONNECT_COUNT | %IW101 | `PUB_RECONNECT_COUNT` at %QW1 | holding register 1 (FC 3) |
