# BSH-Door -- Tag map

Device in Ignition: `BSH-Door`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/BSH-Door/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Unverified in Ignition until imported and read.**

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_DOOR_OPEN_CMD` | `C0` | `BSH/Door/DoorOpenCmd` | Boolean | write |
| `SCADA_DOOR_CLOSE_CMD` | `C1` | `BSH/Door/DoorCloseCmd` | Boolean | write |
| `PUB_REMOTE_OPEN_NO_CARD` | `C2` | `BSH/Door/RemoteOpenNoCard` | Boolean | read |
| `PUB_IR_DETECTED` | `C3` | `BSH/Door/IrDetected` | Boolean | read |
| `PUB_DOOR_POSITION` | `C4` | `BSH/Door/DoorPosition` | Boolean | read |
| `PUB_ACCESS_GRANTED` | `C5` | `BSH/Door/AccessGranted` | Boolean | read |
| `PUB_HEARTBEAT` | `HR0` | `BSH/Door/Heartbeat` | Integer | read |
| `PUB_RECONNECT_COUNT` | `HR1` | `BSH/Door/ReconnectCount` | Integer | read |

Note: command tags must be pulses of 300 ms or longer; a held open bit keeps the gate open and blocks the card reader.
