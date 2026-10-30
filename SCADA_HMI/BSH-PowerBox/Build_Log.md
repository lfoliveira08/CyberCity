# BSH-PowerBox -- SCADA build log

| Date | Step | Result |
|---|---|---|
| (none yet) | | |
| 2026-10-29 | Started PLC .114, created Modbus TCP device `BSH-PowerBox` (192.168.50.114:502, Zero-based ON) and built folder `BSH/PowerBox` with 8 tags from the **Oct 28 power-rails** register map (names proposed by the agent). | Connected (Luis-reported). `Heartbeat` (`HR0`) increasing (Luis-reported), so Zero-based is ON. Status tags, rail commands not checked. The old Tag_Map/Tags.json (lights+fan) were not used. |
| 2026-10-29 | Luis wrote `true` to each of the three rail command tags from Ignition (before the agent gave a write step). | Luis-reported: each relay clicked ("passing power") and each matching status tag turned true. **Not reported:** the order, whether any node was affected, what each rail feeds, whether a `false` write was done and what it did, whether the commands were left on. The earlier contradiction (statuses false while the nodes ran) is unexplained. |
| 2026-10-29 | Luis said the rails are **not connected to the real nodes** (earlier he had answered "Real nodes"; this corrects it) and deferred the `false` write test. | Explains why the statuses read false while the nodes ran. |
| 2026-10-29 | Luis exported the `PowerBox` folder to `SCADA_HMI/Exports/BSH-PowerBox_tags_2026-10-29.json`. The agent read it and rewrote `Tags.json` and `Tag_Map.md` from it. | Verified from the file: 8 tags, names, item paths and types as in the new `Tag_Map.md`. The Oct 27 lights-and-fan files are replaced; their content survives in `Logs/2026-10-27.md` history only. |

