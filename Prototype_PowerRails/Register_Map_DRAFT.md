# Register map — DRAFT (BSH-PowerBox power rails)

**Draft, rewritten Oct 28, 2026 after the lights were dropped. Not decided, not built, not on any container.** The real map is `BSH_Systems/04_BSH-PowerBox.md` and `PLC_Logic/BSH-PowerBox/Register_Map.md`; they win until this is adopted. Replaces the Oct 27 version, which still had a lights coil.

Meaning: a rail command of **1 = node powered**. With the NO contact (decided Oct 27) there is no inversion: rail on = relay coil on. The PLC only forwards the command (`Firmware/CLAUDE.md`: the ESP32 owns safe-state behaviour). Decided Oct 27: rails turn OFF on comms loss. Open: how rails come on after power-up (decision 8).

Rail-to-pin map (Luis, Oct 28, prototype): GPIO 25 = Elevator, 26 = FireAlarm, 27 = Door.

## 1. ESP32 registers

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | ELEV_RAIL_CMD | 1 = Elevator node powered |
| Coil | Coil1 | FIRE_RAIL_CMD | 1 = FireAlarm node powered |
| Coil | Coil2 | DOOR_RAIL_CMD | 1 = Door node powered |
| Discrete Input | DI0 | ELEV_RAIL_STATUS | rail state as the ESP32 drives it |
| Discrete Input | DI1 | FIRE_RAIL_STATUS | same |
| Discrete Input | DI2 | DOOR_RAIL_STATUS | same |
| Input Register | IR0 | HEARTBEAT | unchanged; stays among the last two |
| Input Register | IR1 | RECONNECT_COUNT | unchanged |

Rail status is the ESP32's own relay state, not a voltage measurement. It does not prove the node has power.

## 2. OpenPLC slave-device settings (changes from the real container)

| Setting | Real today | Draft |
|---|---|---|
| Discrete inputs | start 0, size 2 | start 0, **size 3** |
| Coils | start 0, size 2 | start 0, **size 3** |
| Input registers | start 0, size 2 | start 0, size 2 |
| Slave block | %IX100.0-1, %QX100.0-1, %IW100-101 | %IX100.0-2, %QX100.0-2, %IW100-101 |

## 3. Published block (what Ignition reads and writes)

| PLC name | Address |
|---|---|
| SCADA_ELEV_RAIL_CMD | %QX0.0 (C0, SCADA writes) |
| SCADA_FIRE_RAIL_CMD | %QX0.1 (C1, SCADA writes) |
| SCADA_DOOR_RAIL_CMD | %QX0.2 (C2, SCADA writes) |
| PUB_ELEV_RAIL_STATUS | %QX0.3 (C3) |
| PUB_FIRE_RAIL_STATUS | %QX0.4 (C4) |
| PUB_DOOR_RAIL_STATUS | %QX0.5 (C5) |
| PUB_HEARTBEAT | %QW0 (HR0) |
| PUB_RECONNECT_COUNT | %QW1 (HR1) |

No mismatch bits: with no local buttons and no voltage sense, a rail mismatch could never be true. Add them only if one of those is added.

## Not decided

Whether to keep this ordering; whether rail commands should have a "momentary" or "confirm" rule; whether FH-PowerBox uses the same layout (it mirrors BSH, with HVAC in place of Elevator).
