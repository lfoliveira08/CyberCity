# BSH-Elevator -- Tag map

Device in Ignition: `BSH-Elevator`, host per `SCADA_HMI/README.md`, port 502. Source: `PLC_Logic/BSH-Elevator/Register_Map.md` section 3 (PLC published block). Generated Oct 27, 2026 together with `Tags.json`. **Unverified in Ignition until imported and read.**

| PLC name | Ignition address | Tag path | Type | Access |
|---|---|---|---|---|
| `SCADA_CALL_FLOOR_1` | `C0` | `BSH/Elevator/CallFloor1` | Boolean | write |
| `SCADA_CALL_FLOOR_2` | `C1` | `BSH/Elevator/CallFloor2` | Boolean | write |
| `SCADA_CALL_FLOOR_3` | `C2` | `BSH/Elevator/CallFloor3` | Boolean | write |
| `SCADA_ELEVATOR_DISABLE` | `C3` | `BSH/Elevator/ElevatorDisable` | Boolean | write |
| `PUB_ELEVATOR_MOVING` | `C4` | `BSH/Elevator/ElevatorMoving` | Boolean | read |
| `PUB_ELEVATOR_FAULT` | `C5` | `BSH/Elevator/ElevatorFault` | Boolean | read |
| `PUB_CURRENT_FLOOR` | `HR0` | `BSH/Elevator/CurrentFloor` | Integer | read |
| `PUB_HEARTBEAT` | `HR1` | `BSH/Elevator/Heartbeat` | Integer | read |
| `PUB_RECONNECT_COUNT` | `HR2` | `BSH/Elevator/ReconnectCount` | Integer | read |

Note: call-floor commands become a 300 ms pulse in the PLC; the disable tag is a plain level.
