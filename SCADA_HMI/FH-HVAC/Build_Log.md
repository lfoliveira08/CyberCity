# FH-HVAC -- SCADA build log

| Date | Step | Result |
|---|---|---|
| 2026-10-30 | Created Modbus TCP device `FH-HVAC` (192.168.50.121:502, Zero-based ON; PLC started by hand), folders `FH` > `HVAC` and 8 OPC tags from `Tag_Map.md` (registers Short). Address Mapping page opened by mistake and closed without saving (not needed: it creates no tags, `modbus-address-mapping.md`). | Connected, Heartbeat increasing (Luis-reported). |
| 2026-10-30 | Read `TempC`, compared with room temperature. | Screenshot: `TempC` 189, `HumidityPct` 70. Luis-reported room 66 F = 18.9 C, so the unit is tenths of a degree C. |
| 2026-10-30 | Luis exported the `HVAC` folder to `SCADA_HMI/Exports/FH-HVAC_tags_2026-10-30.json`; the agent compared it with `Tags.json`. | Verified from the file: same 8 names, item paths and types (draft lacked `Int2` on the 5 registers, added). Export root is `HVAC`. Writes and real-change tests not done. |

