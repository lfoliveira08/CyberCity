# BSH-Door -- SCADA build log

| Date | Step | Result |
|---|---|---|
| 2026-10-29 | Started PLC .113 by hand, created Modbus TCP device `BSH-Door` (192.168.50.113:502) with Zero-based Addressing ON, built folder `BSH/Door` with the 8 tags from `Tag_Map.md` (registers as Short). | Connected (Luis-reported). `Heartbeat` (`HR0`) increasing (Luis-reported), so Zero-based is ON. Other tags and writes not checked yet. |
| 2026-10-29 | Real-change tests on the read tags (Luis-reported). | `IrDetected` went true when the sensor was triggered. `AccessGranted` went true on a card scan. `RemoteOpenNoCard` stayed false. **Not reported:** whether the gate opened, `DoorPosition`, whether `IrDetected` and `AccessGranted` returned to false, which card was used. |
| 2026-10-29 | `DoorPosition` check. | Luis reported it reads **true with the gate open and closed**. Cause not found: the agent proposed reading `%IX100.1` on the PLC Monitoring page of .113 to see whether the fault is in the ESP32/reed switch or in the PLC/Ignition; **Luis chose not to chase it ("not a major error")**. Accepted known issue, not fixed, not diagnosed. |
| 2026-10-29 | Luis exported the `Door` folder to `SCADA_HMI/Exports/BSH-Door_tags_2026-10-29.json`; the agent compared it with `Tags.json`. | Verified from the file: same 8 names, item paths and types (draft lacked `Int2` on the 2 registers, added). Export root is the `Door` folder. Live values not checked by this step. |

