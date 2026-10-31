# FH-PowerBox -- SCADA build log

| Date | Step | Result |
|---|---|---|
| 2026-10-30 | Created device `FH-PowerBox` and 8 power-rails tags from the Oct 28 register map (not the stale lights+fan draft); Luis exported the folder; the agent rewrote `Tags.json` and `Tag_Map.md` from it. | Luis-reported: Connected, Heartbeat increasing. Verified from the file: 8 tags HvacRail/FireRail/DoorRail Cmd C0-C2, Status C3-C5, Heartbeat HR0, ReconnectCount HR1. No rail written. |

