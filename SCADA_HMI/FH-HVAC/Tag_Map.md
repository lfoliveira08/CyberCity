# FH-HVAC -- Tag map

Device in Ignition: `FH-HVAC`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/FH-HVAC/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Oct 30: tags built by hand and exported (`SCADA_HMI/Exports/FH-HVAC_tags_2026-10-30.json`); device `FH-HVAC` is at 192.168.50.121 with Zero-based Addressing ON. Register tags are Short (Int2).**

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

## Verified 2026-10-30 (Luis-reported unless "screenshot")
- Device Connected, Zero-based ON. `Heartbeat` increasing (screenshot showed 22; "increasing" Luis-reported).
- **`TempC` is tenths of a degree C:** the tag read 189 (screenshot) and Luis reported the room at 66 F = 18.9 C. So `TargetTempC` (same scale per `FH_HVAC_Node.ino` line 27) is tenths too (240 = 24.0 C). The sensor returns values that are not multiples of 10, which the firmware header says a DHT11 cannot: sensor part unexplained, scale confirmed.
- `HumidityPct` read 70 (screenshot), not checked against an instrument. `TargetTempC` read 0 at rest; `HvacEnable`, `CompressorRunning`, `HvacFault` false (screenshot): a resting value proves nothing.
- **Not tested:** the write tags `HvacEnable` and `TargetTempC` (write the target first, then enable), `CompressorRunning` and `HvacFault` on a real change, `ReconnectCount`.
