# FH-HVAC -- Tag map

Device in Ignition: `FH-HVAC`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/FH-HVAC/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Unverified in Ignition until imported and read.**

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_HVAC_ENABLE` | `C0` | `FH/HVAC/HvacEnable` | Boolean | write |
| `PUB_COMPRESSOR_RUNNING` | `C1` | `FH/HVAC/CompressorRunning` | Boolean | read |
| `PUB_HVAC_FAULT` | `C2` | `FH/HVAC/HvacFault` | Boolean | read |
| `SCADA_TARGET_TEMP_C` | `HR0` | `FH/HVAC/TargetTempC` | Integer | write |
| `PUB_TEMP_C` | `HR1` | `FH/HVAC/TempC` | Integer | read |
| `PUB_HUMIDITY_PCT` | `HR2` | `FH/HVAC/HumidityPct` | Integer | read |
| `PUB_HEARTBEAT` | `HR3` | `FH/HVAC/Heartbeat` | Integer | read |
| `PUB_RECONNECT_COUNT` | `HR4` | `FH/HVAC/ReconnectCount` | Integer | read |

Notes: `TempC` and `TargetTempC` are tenths of a degree C (240 = 24.0 C). Write the target first, then enable.
