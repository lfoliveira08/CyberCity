# BSH-Elevator -- SCADA build log

| Date | Step | Result |
|---|---|---|
| 2026-10-29 | Started the PLC on .111 by hand (it was stopped, cause unknown). Created Modbus TCP device `BSH-Elevator` (192.168.50.111:502) with Zero-based Addressing meant to be ON. | Connected (Luis-reported). The Zero-based box was not shown to the agent. |
| 2026-10-29 | Built folder `BSH/Elevator` and 9 OPC tags: `CurrentFloor` HR0, `Heartbeat` HR1, `ReconnectCount` HR2 (Short/Int2), `CallFloor1-3` C0-C2, `ElevatorDisable` C3, `ElevatorMoving` C4, `ElevatorFault` C5 (Boolean). | Tags exist (export). **No values were reported yet.** |
| 2026-10-29 | Luis exported the `Elevator` folder to `SCADA_HMI/Exports/BSH-Elevator_tags_2026-10-29.json`; the agent compared it with `Tags.json`. | Verified from the file: same 9 names, item paths and types as `Tags.json` (the draft lacked `dataType` on the 3 registers; Int2 added). The export root is the `Elevator` folder, not `BSH`. **Not verified:** that any tag reads correctly or that Zero-based is ON. |
| 2026-10-29 | Read the live tags. | `Heartbeat` increasing and `CurrentFloor` matching the car (Luis-reported, no values given). This also shows Zero-based Addressing is ON (at OFF `HR1` would be the floor number). `ReconnectCount`, `ElevatorMoving`, `ElevatorFault` and the four write tags not yet checked. |
| 2026-10-29 | Pressed a local floor button on the node and watched the tags. | `CurrentFloor` updated to the new floor (Luis-reported). `ElevatorFault` stayed false (Luis-reported; a resting false proves nothing, fault needs a real stall). **`ElevatorMoving` answer not given** ("it does move" only), so it is not recorded as proven. |
| 2026-10-29 | Write test from Ignition on a `CallFloor` tag (Write Once), car path reported clear. | Luis reported "I checked and its moving". **Not recorded:** which call tag, start and target floor, whether `ElevatorMoving` went true, whether `CurrentFloor` updated. Treat the write path as **reported to work, details unknown**. `ElevatorDisable` not tested. |

