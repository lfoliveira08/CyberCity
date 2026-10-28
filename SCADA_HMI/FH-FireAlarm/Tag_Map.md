# FH-FireAlarm -- Tag map

Device in Ignition: `FH-FireAlarm`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/FH-FireAlarm/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Unverified in Ignition until imported and read.**

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_ALARM_ACTIVATE` | `C0` | `FH/FireAlarm/AlarmActivate` | Boolean | write |
| `PUB_REMOTE_ONLY` | `C1` | `FH/FireAlarm/RemoteOnly` | Boolean | read |
| `PUB_MANUAL_PULL_TRIGGERED` | `C2` | `FH/FireAlarm/ManualPull` | Boolean | read |
| `PUB_FLAME_DETECTED` | `C3` | `FH/FireAlarm/Flame` | Boolean | read |
| `PUB_GAS_SMOKE_ALERT` | `C4` | `FH/FireAlarm/GasSmokeAlert` | Boolean | read |
| `PUB_GAS_RAW_VALUE` | `HR0` | `FH/FireAlarm/GasRaw` | Integer | read |
| `PUB_HEARTBEAT` | `HR1` | `FH/FireAlarm/Heartbeat` | Integer | read |
| `PUB_RECONNECT_COUNT` | `HR2` | `FH/FireAlarm/ReconnectCount` | Integer | read |
