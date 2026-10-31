# FH-Door -- SCADA build log

| Date | Step | Result |
|---|---|---|
| 2026-10-30 | Created device `FH-Door`, folders and 8 tags; Luis exported the folder to `SCADA_HMI/Exports/FH-Door_tags_2026-10-30.json` and the agent compared it with `Tags.json`. | Luis-reported: Connected, Heartbeat increasing. Verified from the file: same 8 names, item paths and types as `Tags.json` (`Heartbeat`, `ReconnectCount` (draft lacked `Int2`, added)). Live values, real-change tests and writes **not done**. |

