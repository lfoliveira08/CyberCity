# FH-PowerBox -- Register map and address translation

> **Rewritten Oct 28, 2026, mirroring `PLC_Logic/BSH-PowerBox/Register_Map.md`** for the power-rails design. Replaces the lights-and-fan version, archived in `Archive/FH-PowerBox_PLC_lights-and-fan/`. **Not yet confirmed on FH's own container** — the slave-device sizes and addresses below are the BSH pattern, not yet verified on 192.168.50.124's Slave Devices page.

Meaning: a rail command of **1 = node powered**. The relay contact is NO, so there is no inversion anywhere: rail on = relay coil on. The PLC only forwards the command; the ESP32 owns safe-state behaviour. After a power-up all rails stay off until commanded on (option A); they go off after 5 s without WiFi.

Rail-to-node map: Coil0 / DI0 = FH-HVAC (GPIO 25), Coil1 / DI1 = FH-FireAlarm (GPIO 26), Coil2 / DI2 = FH-Door (GPIO 27).

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | HVAC_RAIL_CMD | 1 = FH-HVAC node powered |
| Coil | Coil1 | FIRE_RAIL_CMD | 1 = FH-FireAlarm node powered |
| Coil | Coil2 | DOOR_RAIL_CMD | 1 = FH-Door node powered |
| Discrete Input | DI0 | HVAC_RAIL_STATUS | rail as the ESP32 drives it (commanded state, not a measured voltage) |
| Discrete Input | DI1 | FIRE_RAIL_STATUS | same |
| Discrete Input | DI2 | DOOR_RAIL_STATUS | same |
| Input Register | IR0 | HEARTBEAT | counts up once a second; frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `FH-PowerBox-ESP32` |
| Protocol | Generic Modbus TCP Device |
| IP address / port | 192.168.50.24 / 502 |
| Slave ID | 0 (worked on this container Oct 21, 2026, for the old design; not re-verified for the power rails) |
| Discrete inputs | start 0, size **3** (was 2) |
| Coils | start 0, size **3** (was 2) |
| Input registers | start 0, size 2 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-FH-PowerBox` at 192.168.50.124, Web UI port 8080, Modbus server port 502. **Not yet edited for the power rails on this container.**

## 3. Address translation

**VERIFIED Oct 28, 2026 (Luis screenshot):** FH-PowerBox container's Slave Devices list confirms `%IX100.0 to %IX100.2`, `%QX100.0 to %QX100.2`, `%IW100 to %IW101`, matching the table below exactly. The convention (`%IX100.x`, `%QX100.x`, `%IW100+`) is the OpenPLC v3 one (`PLC_Logic/Templates/Address_Mapping.md`).

| ESP32 register | OpenPLC slave-block address | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 HVAC_RAIL_CMD | %QX100.0 | `SCADA_HVAC_RAIL_CMD` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| Coil1 FIRE_RAIL_CMD | %QX100.1 | `SCADA_FIRE_RAIL_CMD` at %QX0.1 (SCADA writes) | coil 1 (FC 5/15) |
| Coil2 DOOR_RAIL_CMD | %QX100.2 | `SCADA_DOOR_RAIL_CMD` at %QX0.2 (SCADA writes) | coil 2 (FC 5/15) |
| DI0 HVAC_RAIL_STATUS | %IX100.0 | `PUB_HVAC_RAIL_STATUS` at %QX0.3 | coil 3 (FC 1) |
| DI1 FIRE_RAIL_STATUS | %IX100.1 | `PUB_FIRE_RAIL_STATUS` at %QX0.4 | coil 4 (FC 1) |
| DI2 DOOR_RAIL_STATUS | %IX100.2 | `PUB_DOOR_RAIL_STATUS` at %QX0.5 | coil 5 (FC 1) |
| IR0 HEARTBEAT | %IW100 | `PUB_HEARTBEAT` at %QW0 | holding register 0 (FC 3) |
| IR1 RECONNECT_COUNT | %IW101 | `PUB_RECONNECT_COUNT` at %QW1 | holding register 1 (FC 3) |

No mismatch flags, same reasoning as BSH-PowerBox.
