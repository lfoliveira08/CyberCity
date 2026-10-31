# FH-PowerBox -- Tag map

Device in Ignition: `FH-PowerBox`, host 192.168.50.124, port 502, **Zero-based Addressing ON**. Source: `PLC_Logic/FH-PowerBox/Register_Map.md` sections 1 and 3 (Oct 28, 2026 power-rails design). Rewritten Oct 30, 2026 from the live tags (`SCADA_HMI/Exports/FH-PowerBox_tags_2026-10-30.json`). **Replaces the Oct 27 lights-and-fan version, which was stale.** Tag names mirror BSH-PowerBox (HVAC in place of Elevator).

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_HVAC_RAIL_CMD` | `C0` | `FH/PowerBox/HvacRailCmd` | Boolean | write |
| `SCADA_FIRE_RAIL_CMD` | `C1` | `FH/PowerBox/FireRailCmd` | Boolean | write |
| `SCADA_DOOR_RAIL_CMD` | `C2` | `FH/PowerBox/DoorRailCmd` | Boolean | write |
| `PUB_HVAC_RAIL_STATUS` | `C3` | `FH/PowerBox/HvacRailStatus` | Boolean | read |
| `PUB_FIRE_RAIL_STATUS` | `C4` | `FH/PowerBox/FireRailStatus` | Boolean | read |
| `PUB_DOOR_RAIL_STATUS` | `C5` | `FH/PowerBox/DoorRailStatus` | Boolean | read |
| `PUB_HEARTBEAT` | `HR0` | `FH/PowerBox/Heartbeat` | Short (Int2) | read |
| `PUB_RECONNECT_COUNT` | `HR1` | `FH/PowerBox/ReconnectCount` | Short (Int2) | read |

Rail command 1 = node powered (relay NO, no inversion). Status is the rail as the ESP32 drives it (commanded state), not a measured voltage. Rails stay off after a power-up until commanded on.

Command rule: the ESP32 counts a coil write only when its value differs from the last hub value (authority rule, Oct 21): write the opposite value, then the original.

## Verified 2026-10-30 (Luis-reported)
- Device Connected and `Heartbeat` increasing ("Yes working"). **No rail command was written; the rail-to-node wiring was not given; status tags not read back to the agent.**
