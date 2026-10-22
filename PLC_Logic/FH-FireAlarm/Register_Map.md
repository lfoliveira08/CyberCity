# FH-FireAlarm -- Register map and address translation

> Drafted Oct 11, 2026; program built and running Oct 21, 2026 (see `Build_Log.md`). Source of truth for the ESP32 side: `FH_Systems/02_FH-FireAlarm.md`. If this file and that doc disagree, the doc wins.

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | MANUAL_PULL_TRIGGERED | Local pull station activated |
| Discrete Input | DI1 | FLAME_DETECTED | Flame sensor tripped |
| Discrete Input | DI2 | GAS_SMOKE_ALERT | MQ-2 above baseline + margin |
| Coil | Coil0 | ALARM_ACTIVATE | Remote/hub-triggered alarm |
| Input Register | IR0 | GAS_RAW_VALUE | Raw MQ-2 ADC reading 0-4095 |
| Input Register | IR1 | HEARTBEAT | Counts up once a second; frozen = node lost |
| Input Register | IR2 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `FH-FireAlarm-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.22 / 502 |
| Slave ID | 0 (entered Oct 21; proven by live polling Oct 21, 2026) |
| Discrete inputs | start 0, size 3 |
| Coils | start 0, size 1 |
| Input registers | start 0, size 3 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-FH-FireAlarm` at 192.168.50.122, Web UI port 8080, Modbus server port 502.

## 3. Address translation

**Verified on the BSH containers** (Slave Devices page: "attached to address 100 onward"). On this container the Slave Devices listing was not shown on Oct 21, 2026, but the running program is consistent with the convention (heartbeat and pull-station tests worked).
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address (verify) | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 ALARM_ACTIVATE | %QX100.0 | `SCADA_ALARM_ACTIVATE` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| (computed) REMOTE_ONLY | - | `PUB_REMOTE_ONLY` at %QX0.1 | coil 1 (FC 1) |
| DI0 MANUAL_PULL_TRIGGERED | %IX100.0 | `PUB_MANUAL_PULL_TRIGGERED` at %QX0.2 | coil 2 (FC 1) |
| DI1 FLAME_DETECTED | %IX100.1 | `PUB_FLAME_DETECTED` at %QX0.3 | coil 3 (FC 1) |
| DI2 GAS_SMOKE_ALERT | %IX100.2 | `PUB_GAS_SMOKE_ALERT` at %QX0.4 | coil 4 (FC 1) |
| IR0 GAS_RAW_VALUE | %IW100 | `PUB_GAS_RAW_VALUE` at %QW0 | holding register 0 (FC 3) |
| IR1 HEARTBEAT | %IW101 | `PUB_HEARTBEAT` at %QW1 | holding register 1 (FC 3) |
| IR2 RECONNECT_COUNT | %IW102 | `PUB_RECONNECT_COUNT` at %QW2 | holding register 2 (FC 3) |
