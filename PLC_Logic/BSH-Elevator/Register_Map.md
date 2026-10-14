# BSH-Elevator -- Register map and address translation

> DRAFT Oct 11, 2026. Source of truth for the ESP32 side: `BSH_Systems/01_BSH-Elevator.md`. If this file and that doc disagree, the doc wins.

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | ELEVATOR_MOVING | Motor currently running |
| Discrete Input | DI1 | ELEVATOR_FAULT | Fault / stall condition |
| Coil | Coil0 | CALL_FLOOR_1 | Call to floor 1 |
| Coil | Coil1 | CALL_FLOOR_2 | Call to floor 2 |
| Coil | Coil2 | CALL_FLOOR_3 | Call to floor 3 |
| Coil | Coil3 | ELEVATOR_DISABLE | Lock out the elevator |
| Input Register | IR0 | CURRENT_FLOOR | Reported floor 1-3 |
| Input Register | IR1 | HEARTBEAT | Counts up once a second; frozen = node lost |
| Input Register | IR2 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `BSH-Elevator-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.11 / 502 |
| Slave ID | TBD -- confirm in Phase C (record the value that works in `Build_Log.md`) |
| Discrete inputs | start 0, size 2 |
| Coils | start 0, size 4 |
| Input registers | start 0, size 3 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-BSH-Elevator` at 192.168.50.111, Web UI port 8080, Modbus server port 502.

## 3. Address translation

**UNVERIFIED:** the slave-block addresses (`%IX100.x`, `%QX100.x`, `%IW100+`, `%QW100+`) are the OpenPLC v3 convention, and the current OpenPLC documentation does not restate them. Confirm each one on the Monitoring page and write the result in `Build_Log.md`.
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address (verify) | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 CALL_FLOOR_1 | %QX100.0 | `SCADA_CALL_FLOOR_1` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| Coil1 CALL_FLOOR_2 | %QX100.1 | `SCADA_CALL_FLOOR_2` at %QX0.1 (SCADA writes) | coil 1 (FC 5/15) |
| Coil2 CALL_FLOOR_3 | %QX100.2 | `SCADA_CALL_FLOOR_3` at %QX0.2 (SCADA writes) | coil 2 (FC 5/15) |
| Coil3 ELEVATOR_DISABLE | %QX100.3 | `SCADA_ELEVATOR_DISABLE` at %QX0.3 (SCADA writes) | coil 3 (FC 5/15) |
| DI0 ELEVATOR_MOVING | %IX100.0 | `PUB_ELEVATOR_MOVING` at %QX0.4 | coil 4 (FC 1) |
| DI1 ELEVATOR_FAULT | %IX100.1 | `PUB_ELEVATOR_FAULT` at %QX0.5 | coil 5 (FC 1) |
| IR0 CURRENT_FLOOR | %IW100 | `PUB_CURRENT_FLOOR` at %QW0 | holding register 0 (FC 3) |
| IR1 HEARTBEAT | %IW101 | `PUB_HEARTBEAT` at %QW1 | holding register 1 (FC 3) |
| IR2 RECONNECT_COUNT | %IW102 | `PUB_RECONNECT_COUNT` at %QW2 | holding register 2 (FC 3) |
