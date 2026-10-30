# BSH-FireAlarm -- Tag map

Device in Ignition: `BSH-FireAlarm`, host 192.168.50.112, port 502. Source: `PLC_Logic/BSH-FireAlarm/Register_Map.md` section 3 (PLC published block). Draft Oct 26, 2026. **Oct 29: the Ignition addresses below are correct only with the device's Zero-based Addressing ON (it is ON on this device).** With the Gateway default (OFF) every address is one higher and `HR1` is the gas value.

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

## Verified 2026-10-29 (Zero-based ON; Luis-reported unless "screenshot")
- Built by hand under `[default]BSH/FireAlarm/`. Register tags are type Short (Int2).
- `Heartbeat` HR1: rising (screenshot: 2,157 then 2,421). `GasRaw` HR0: live value (16 in a screenshot), not shown to follow the sensor. `ReconnectCount` HR2: 0, consistent with the map, not proven.
- `ManualPull` C2: went true on a real pull-station press. `RemoteOnly` C1 and `AlarmActivate` C0: a Write Once of `true` sounded the buzzer and `RemoteOnly` went true; `false` stopped it.
- **Not seen to change on Oct 29 morning:** `Flame` C3, `GasSmokeAlert` C4. **Later Oct 29 (Luis-reported): both turned true when triggered**, so C3 and C4 are confirmed; the trigger method and the `GasRaw` value at that moment were not recorded.
- The Oct 26 line "`HR1` (PUB_HEARTBEAT) confirmed live" above is **superseded**: with the Gateway default (Zero-based OFF) `HR1` is the gas value.
