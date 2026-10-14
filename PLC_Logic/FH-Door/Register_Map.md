# FH-Door -- Register map and address translation

> DRAFT Oct 11, 2026. Source of truth for the ESP32 side: `FH_Systems/03_FH-Door.md`. If this file and that doc disagree, the doc wins.

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | IR_DETECTED | Approach / presence detected |
| Discrete Input | DI1 | DOOR_POSITION | Actual open(1) / closed(0) |
| Discrete Input | DI2 | ACCESS_GRANTED | Valid card read accepted locally |
| Coil | Coil0 | DOOR_OPEN_CMD | Command to open (remote override) |
| Coil | Coil1 | DOOR_CLOSE_CMD | Command to close |
| Input Register | IR0 | HEARTBEAT | Counts up once a second; frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `FH-Door-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.23 / 502 |
| Slave ID | TBD -- confirm in Phase C (record the value that works in `Build_Log.md`) |
| Discrete inputs | start 0, size 3 |
| Coils | start 0, size 2 |
| Input registers | start 0, size 2 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-FH-Door` at 192.168.50.123, Web UI port 8080, Modbus server port 502.

## 3. Address translation

**UNVERIFIED:** the slave-block addresses (`%IX100.x`, `%QX100.x`, `%IW100+`, `%QW100+`) are the OpenPLC v3 convention, and the current OpenPLC documentation does not restate them. Confirm each one on the Monitoring page and write the result in `Build_Log.md`.
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address (verify) | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 DOOR_OPEN_CMD | %QX100.0 | `SCADA_DOOR_OPEN_CMD` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| Coil1 DOOR_CLOSE_CMD | %QX100.1 | `SCADA_DOOR_CLOSE_CMD` at %QX0.1 (SCADA writes) | coil 1 (FC 5/15) |
| (computed) REMOTE_OPEN_NO_CARD | - | `PUB_REMOTE_OPEN_NO_CARD` at %QX0.2 | coil 2 (FC 1) |
| DI0 IR_DETECTED | %IX100.0 | `PUB_IR_DETECTED` at %QX0.3 | coil 3 (FC 1) |
| DI1 DOOR_POSITION | %IX100.1 | `PUB_DOOR_POSITION` at %QX0.4 | coil 4 (FC 1) |
| DI2 ACCESS_GRANTED | %IX100.2 | `PUB_ACCESS_GRANTED` at %QX0.5 | coil 5 (FC 1) |
| IR0 HEARTBEAT | %IW100 | `PUB_HEARTBEAT` at %QW0 | holding register 0 (FC 3) |
| IR1 RECONNECT_COUNT | %IW101 | `PUB_RECONNECT_COUNT` at %QW1 | holding register 1 (FC 3) |
