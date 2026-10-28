# BSH-PowerBox -- Tag map

Device in Ignition: `BSH-PowerBox`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/BSH-PowerBox/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Unverified in Ignition until imported and read.**

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_LIGHTS_ON_CMD` | `C0` | `BSH/PowerBox/LightsOnCmd` | Boolean | write |
| `SCADA_FAN_CMD` | `C1` | `BSH/PowerBox/FanCmd` | Boolean | write |
| `PUB_LIGHTS_MISMATCH` | `C2` | `BSH/PowerBox/LightsMismatch` | Boolean | read |
| `PUB_FAN_MISMATCH` | `C3` | `BSH/PowerBox/FanMismatch` | Boolean | read |
| `PUB_LIGHTS_STATUS` | `C4` | `BSH/PowerBox/LightsStatus` | Boolean | read |
| `PUB_FAN_STATUS` | `C5` | `BSH/PowerBox/FanStatus` | Boolean | read |
| `PUB_HEARTBEAT` | `HR0` | `BSH/PowerBox/Heartbeat` | Integer | read |
| `PUB_RECONNECT_COUNT` | `HR1` | `BSH/PowerBox/ReconnectCount` | Integer | read |

Note: the ESP32 counts a write only when its value differs from the last hub value. Write the opposite value, then the original.
