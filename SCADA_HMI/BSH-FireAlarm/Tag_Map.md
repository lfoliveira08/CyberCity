# BSH-FireAlarm -- Tag map

Device in Ignition: `BSH-FireAlarm`, host 192.168.50.112, port 502. Source: `PLC_Logic/BSH-FireAlarm/Register_Map.md` section 3 (PLC published block). Draft Oct 26, 2026; the Ignition address column is unverified until the heartbeat test passes (0- vs 1-based addressing is an open question).

| PLC name | Modbus address (0-based) | Ignition address (if zero-based on) | Proposed tag path | Type | Access |
|---|---|---|---|---|---|
| `SCADA_ALARM_ACTIVATE` | coil 0 | `C0` | `BSH/FireAlarm/AlarmActivate` | Boolean | write |
| `PUB_REMOTE_ONLY` | coil 1 | `C1` | `BSH/FireAlarm/RemoteOnly` | Boolean | read |
| `PUB_MANUAL_PULL_TRIGGERED` | coil 2 | `C2` | `BSH/FireAlarm/ManualPull` | Boolean | read |
| `PUB_FLAME_DETECTED` | coil 3 | `C3` | `BSH/FireAlarm/Flame` | Boolean | read |
| `PUB_GAS_SMOKE_ALERT` | coil 4 | `C4` | `BSH/FireAlarm/GasSmokeAlert` | Boolean | read |
| `PUB_GAS_RAW_VALUE` | holding reg 0 | `HR0` | `BSH/FireAlarm/GasRaw` | Int2 (0-4095) | read |
| `PUB_HEARTBEAT` | holding reg 1 | `HR1` | `BSH/FireAlarm/Heartbeat` | Int2 | read |
| `PUB_RECONNECT_COUNT` | holding reg 2 | `HR2` | `BSH/FireAlarm/ReconnectCount` | Int2 | read |

Test to run first: read `[BSH-FireAlarm]HR1` in the Quick Client; it must rise about once per second.

## Verified 2026-10-26
- `HR1` (PUB_HEARTBEAT) confirmed live in Ignition: the value rose over time. The other rows in the table are still unverified.
- The temporary tag `TestHeartbeat` (default provider, root) is to be deleted once the real tag tree exists.
