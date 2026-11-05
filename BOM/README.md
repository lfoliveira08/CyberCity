# BOM -- bill of materials

Created Nov 4, 2026. **Status: draft.** Status of the project lives in `../STATUS.md`, not here.

## What lives here

- `Hardware_BOM.md` -- parts for the eight ESP32 nodes (part one), then servers, network, software and licences, the 3D-printed structures and PLA (part two). Compiled from the docs, not counted on the bench. Open questions for Luis are at the end of it.

## Where the data comes from (the BOM does not own these, it summarises them)

| BOM section | Source of truth |
|---|---|
| Node parts | `../Firmware/*_Bringup*.md` (Parts Needed), `../BSH_Sensors/`, `../FH_Sensors/`, the `Build_Log.md` of each component |
| Servers, VMs, licences | `../Proxmox_Build_Log.md`, `../STATUS.md` |
| 3D-printed structures | `../Piranga_Workflow_v2.md` Section 8 (stale for the PowerBox, Elevator display and HVAC fan; the BOM follows the bring-up checklists) |

## Rules

1. **Change the pair, not the half.** If a part, pin or register changes in a system doc or bring-up checklist, update the matching BOM row in the same sitting, and the other way round.
2. **Verified vs reported vs assumed.** Every row carries a status (Doc / Bench / Open). Prices and PLA weights are Luis-reported or agent estimates until a receipt is recorded; label them.
3. **No credentials or license keys** in any file here. The Ignition activation token is a secret.
4. **File names carry no version.** Dated exports only, for example `Hardware_BOM_2026-12-01.pdf`.
5. Log every change in that day's file in `../Logs/`.
