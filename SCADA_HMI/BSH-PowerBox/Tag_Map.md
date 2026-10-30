# BSH-PowerBox -- Tag map

Device in Ignition: `BSH-PowerBox`, host 192.168.50.114, port 502, **Zero-based Addressing ON**. Source: `PLC_Logic/BSH-PowerBox/Register_Map.md` section 3 (Oct 28, 2026 power-rails design). Rewritten Oct 29, 2026 from the live tags (`SCADA_HMI/Exports/BSH-PowerBox_tags_2026-10-29.json`). **Replaces the Oct 27 lights-and-fan version, which was stale.** Tag names were proposed by the agent and accepted as built by Luis.

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_ELEV_RAIL_CMD` | `C0` | `BSH/PowerBox/ElevRailCmd` | Boolean | write |
| `SCADA_FIRE_RAIL_CMD` | `C1` | `BSH/PowerBox/FireRailCmd` | Boolean | write |
| `SCADA_DOOR_RAIL_CMD` | `C2` | `BSH/PowerBox/DoorRailCmd` | Boolean | write |
| `PUB_ELEV_RAIL_STATUS` | `C3` | `BSH/PowerBox/ElevRailStatus` | Boolean | read |
| `PUB_FIRE_RAIL_STATUS` | `C4` | `BSH/PowerBox/FireRailStatus` | Boolean | read |
| `PUB_DOOR_RAIL_STATUS` | `C5` | `BSH/PowerBox/DoorRailStatus` | Boolean | read |
| `PUB_HEARTBEAT` | `HR0` | `BSH/PowerBox/Heartbeat` | Short (Int2) | read |
| `PUB_RECONNECT_COUNT` | `HR1` | `BSH/PowerBox/ReconnectCount` | Short (Int2) | read |

Rail command 1 = node powered (relay NO, no inversion). Status is the rail as the ESP32 drives it (commanded state), not a measured voltage. Rails stay off after a power-up until commanded on.

Command rule: the ESP32 counts a coil write only when its value differs from the last hub value (`BSH_Systems/04_BSH-PowerBox.md`, authority rule, unchanged Oct 21): write the opposite value, then the original.

## Verified 2026-10-29 (Luis-reported unless noted)
- `Heartbeat` increasing, so Zero-based is ON. `ReconnectCount` not read.
- Status tags read false at rest. Writing `true` to each rail command made its relay click and its status turn true. Not measured: no voltage or current. The rails are **not connected to the real nodes** (Luis, Oct 29). The `false` direction was not tested.
