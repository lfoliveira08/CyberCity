# Register map — DRAFT (BSH-PowerBox power rails)

**Draft, Oct 27, 2026. Not decided, not built, not on any container.** The real map is `BSH_Systems/04_BSH-PowerBox.md` and `PLC_Logic/BSH-PowerBox/Register_Map.md`; they win until this is adopted.

Meaning: a rail command of **1 = node powered**. The ESP32 firmware turns that into the right relay level (with the NO contact, decided Oct 27, 2026, there is no inversion: rail on = coil on); the PLC only forwards the command (`Firmware/CLAUDE.md`: the ESP32 owns safe-state behaviour).

## 1. ESP32 registers

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | LIGHTS_ON_CMD | unchanged |
| Coil | Coil1 | ELEV_RAIL_CMD | 1 = Elevator node powered |
| Coil | Coil2 | FIRE_RAIL_CMD | 1 = FireAlarm node powered |
| Coil | Coil3 | DOOR_RAIL_CMD | 1 = Door node powered |
| Discrete Input | DI0 | LIGHTS_STATUS | unchanged |
| Discrete Input | DI1 | ELEV_RAIL_STATUS | rail state as the ESP32 drives it |
| Discrete Input | DI2 | FIRE_RAIL_STATUS | same |
| Discrete Input | DI3 | DOOR_RAIL_STATUS | same |
| Input Register | IR0 | HEARTBEAT | unchanged; stays among the last two |
| Input Register | IR1 | RECONNECT_COUNT | unchanged |

Rail status is the ESP32's own relay state, not a voltage measurement. It does not prove the node has power unless a voltage-sense input is added (not designed).

## 2. OpenPLC slave-device settings (changes from the real container)

| Setting | Real today | Draft |
|---|---|---|
| Discrete inputs | start 0, size 2 | start 0, **size 4** |
| Coils | start 0, size 2 | start 0, **size 4** |
| Input registers | start 0, size 2 | start 0, size 2 |
| Slave block | %IX100.0-1, %QX100.0-1, %IW100-101 | %IX100.0-3, %QX100.0-3, %IW100-101 |

## 3. Published block (what Ignition reads and writes)

| PLC name | Address |
|---|---|
| SCADA_LIGHTS_ON_CMD | %QX0.0 (C0, SCADA writes) |
| SCADA_ELEV_RAIL_CMD | %QX0.1 (C1, SCADA writes) |
| SCADA_FIRE_RAIL_CMD | %QX0.2 (C2, SCADA writes) |
| SCADA_DOOR_RAIL_CMD | %QX0.3 (C3, SCADA writes) |
| PUB_LIGHTS_MISMATCH | %QX0.4 (C4) |
| PUB_LIGHTS_STATUS | %QX0.5 (C5) |
| PUB_ELEV_RAIL_STATUS | %QX0.6 (C6) |
| PUB_FIRE_RAIL_STATUS | %QX0.7 (C7) |
| PUB_DOOR_RAIL_STATUS | %QX1.0 (C8; coil 8 = %QX1.0 per `PLC_Logic/BSH-PowerBox/Register_Map.md` §3) |
| PUB_HEARTBEAT | %QW0 (HR0) |
| PUB_RECONNECT_COUNT | %QW1 (HR1) |

No rail mismatch bits: with no rail button or voltage sense, a rail mismatch could never be true. Add them only if one of those is added.

## Not decided

Whether to keep this ordering; whether the rail commands should also have a "momentary" or "confirm" rule; whether FH-PowerBox uses the same layout (it mirrors BSH, with HVAC in place of Elevator).
