# Exports -- backups and project exports

Dated file names only, for example `CyberCity_project_2026-10-26.zip`, `Gateway_backup_2026-10-26.gwbk`. Never put licence keys, tokens or passwords in here; check a Gateway backup before committing it, since it can contain credentials. Keep this repo private.

| Date | File | What | Taken by |
|---|---|---|---|
| 2026-10-29 | `BSH-FireAlarm_tags_2026-10-29.json` | Designer tag export of the `BSH` folder (8 tags), `platform/tags/exporting-and-importing-tags.md`. Identical to `SCADA_HMI/BSH-FireAlarm/Tags.json` in names, item paths and types | Luis |
| 2026-10-29 | `BSH-Elevator_tags_2026-10-29.json` | Designer tag export of the `Elevator` folder (9 tags). Same names, item paths and types as `SCADA_HMI/BSH-Elevator/Tags.json`. Live values not checked | Luis |
| 2026-10-29 | `BSH-Door_tags_2026-10-29.json` | Designer tag export of the `Door` folder (8 tags). Same names, item paths and types as `SCADA_HMI/BSH-Door/Tags.json`. `DoorPosition` known to read true open and closed | Luis |
| 2026-10-29 | `BSH-PowerBox_tags_2026-10-29.json` | Designer tag export of the `PowerBox` folder (8 power-rails tags). Source for the rewritten `SCADA_HMI/BSH-PowerBox/Tags.json` | Luis |
| 2026-10-30 | `FH-HVAC_tags_2026-10-30.json` | Designer tag export of the `HVAC` folder (8 tags). Same names, item paths and types as `SCADA_HMI/FH-HVAC/Tags.json` | Luis |
| 2026-10-30 | `Ignition-SCADA-HMI_Ignition-backup-20261030-1337.gwbk` | **Gateway backup** (Platform > System > Backup & Restore), 3,699,623 bytes, taken with all eight devices and their tags. Contents not opened or verified; may hold credentials; **git-ignored** (`*.gwbk` in `.gitignore`). A restore overwrites the whole Gateway | Luis |
| 2026-10-30 | `FH-FireAlarm_tags_2026-10-30.json` | Designer tag export of the `FireAlarm` folder (8 tags), same names, paths and types as `FH-FireAlarm/Tags.json` | Luis |
| 2026-10-30 | `FH-Door_tags_2026-10-30.json` | Designer tag export of the `Door` folder (8 tags), same names, paths and types as `FH-Door/Tags.json` | Luis |
| 2026-10-30 | `FH-PowerBox_tags_2026-10-30.json` | Designer tag export of the `PowerBox` folder (8 tags); source of the rewritten `FH-PowerBox/Tags.json` | Luis |
| 2026-10-26 | `tags.json` | **Stale:** the temp tag `TestHeartbeat` only, deleted Oct 29. Kept as history, do not import | Luis |
