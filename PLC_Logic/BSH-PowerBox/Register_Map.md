# BSH-PowerBox -- Register map and address translation

> **Rewritten Oct 28, 2026** for the power-rails design (adoption Step 4, `Prototype_PowerRails/ADOPTION_CHECKLIST.md`). Replaces the lights-and-fan version, archived in `Archive/BSH-PowerBox_PLC_lights-and-fan/`. Source of truth for the ESP32 side is the header of `Firmware/BSH-PowerBox/BSH_PowerBox_Node/BSH_PowerBox_Node.ino`; `BSH_Systems/04_BSH-PowerBox.md` still describes the old design until adoption Step 3 is done, so if they disagree this file and the sketch header win.
>
> **State today:** `Program.st` and the Editor project `BSH-PowerBox-ESP32/` in this folder now hold the power-rails logic: Luis built the 8 rungs in the Editor (saved Oct 28, 2026, 18:25) and `Program.st` was regenerated from it. Container .114 currently runs a draft program from `Prototype_PowerRails/PLC/Program.st` that uses the addresses below (Luis-reported).

Meaning: a rail command of **1 = node powered**. The relay contact is NO, so there is no inversion anywhere: rail on = relay coil on. The PLC only forwards the command; the ESP32 owns safe-state behaviour (`Firmware/CLAUDE.md`). After a power-up all rails stay off until commanded on (decided Oct 28, option A); they go off after 5 s without WiFi (WiFi link loss only).

Rail-to-node map: Coil0 / DI0 = Elevator (GPIO 25), Coil1 / DI1 = FireAlarm (GPIO 26), Coil2 / DI2 = Door (GPIO 27).

## 1. ESP32 registers (what the node exposes)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | ELEV_RAIL_CMD | 1 = Elevator node powered |
| Coil | Coil1 | FIRE_RAIL_CMD | 1 = FireAlarm node powered |
| Coil | Coil2 | DOOR_RAIL_CMD | 1 = Door node powered |
| Discrete Input | DI0 | ELEV_RAIL_STATUS | rail as the ESP32 drives it (commanded state, not a measured voltage) |
| Discrete Input | DI1 | FIRE_RAIL_STATUS | same |
| Discrete Input | DI2 | DOOR_RAIL_STATUS | same |
| Input Register | IR0 | HEARTBEAT | counts up once a second; frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot |

## 2. OpenPLC slave-device settings (Settings > Slave Devices)

| Setting | Value |
|---|---|
| Device name | `BSH-PowerBox-ESP32` |
| Protocol | Generic Modbus TCP Device (the `TCP` type in the dropdown, not the `ESP32` preset, which locks the sizes) |
| IP address / port | 192.168.50.14 / 502 |
| Slave ID | 0 (worked on this container, Oct 21, 2026) |
| Discrete inputs | start 0, size **3** (was 2) |
| Coils | start 0, size **3** (was 2) |
| Input registers | start 0, size 2 (includes HEARTBEAT and RECONNECT_COUNT) |
| Holding registers - read | start 0, size 0 |
| Holding registers - write | start 0, size 0 |

This PLC itself: container `OpenPLC-BSH-PowerBox` at 192.168.50.114, Web UI port 8080, Modbus server port 502. The ESP32 at .14 is now the former spare board on a breadboard (Luis, Oct 28); the old lights-and-fan board must stay off.

## 3. Address translation

**VERIFIED Oct 28, 2026 (Luis screenshot):** the container's Slave Devices list shows one device, `BSH-PowerBox-ESP32`, type TCP, DI `%IX100.0 to %IX100.2`, DO `%QX100.0 to %QX100.2`, AI `%IW100 to %IW101`, AO none. The convention (`%IX100.x`, `%QX100.x`, `%IW100+`) is the OpenPLC v3 one (`PLC_Logic/Templates/Address_Mapping.md`).
**From the OpenPLC docs:** the PLC's own Modbus server maps coil n to `%QX` bit n (coil 8 = `%QX1.0`) and holding register n to `%QWn`, both starting at 0. That is why this program publishes everything SCADA needs into `%QX0.x` and `%QW0..`. Some SCADA tools show these 1-based or in 0xxxx / 4xxxx notation, so subtract one or drop the prefix if values look shifted.

| ESP32 register | OpenPLC slave-block address | Published for SCADA at | Modbus function / address (0-based) |
|---|---|---|---|
| Coil0 ELEV_RAIL_CMD | %QX100.0 | `SCADA_ELEV_RAIL_CMD` at %QX0.0 (SCADA writes) | coil 0 (FC 5/15) |
| Coil1 FIRE_RAIL_CMD | %QX100.1 | `SCADA_FIRE_RAIL_CMD` at %QX0.1 (SCADA writes) | coil 1 (FC 5/15) |
| Coil2 DOOR_RAIL_CMD | %QX100.2 | `SCADA_DOOR_RAIL_CMD` at %QX0.2 (SCADA writes) | coil 2 (FC 5/15) |
| DI0 ELEV_RAIL_STATUS | %IX100.0 | `PUB_ELEV_RAIL_STATUS` at %QX0.3 | coil 3 (FC 1) |
| DI1 FIRE_RAIL_STATUS | %IX100.1 | `PUB_FIRE_RAIL_STATUS` at %QX0.4 | coil 4 (FC 1) |
| DI2 DOOR_RAIL_STATUS | %IX100.2 | `PUB_DOOR_RAIL_STATUS` at %QX0.5 | coil 5 (FC 1) |
| IR0 HEARTBEAT | %IW100 | `PUB_HEARTBEAT` at %QW0 | holding register 0 (FC 3) |
| IR1 RECONNECT_COUNT | %IW101 | `PUB_RECONNECT_COUNT` at %QW1 | holding register 1 (FC 3) |

No mismatch flags: with no local buttons and no voltage sense, a rail's command and status could never disagree, so a mismatch bit could never be true. Add one only if a button or a voltage sense is added.

Note for SCADA: the SCADA command bits are the PLC's own coils (`%QX0.0` to `%QX0.2`) and start at 0 after a PLC restart (assumed, not checked), so after a PLC restart the rails are commanded off.
