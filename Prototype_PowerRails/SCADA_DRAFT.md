# SCADA — DRAFT (power rails)

**Draft, Oct 27, 2026. Nothing imported into Ignition. No Ignition step here was looked up in `References/Ignition/`; each one must be, when built (`SCADA_HMI/CLAUDE.md`).** Real files: `SCADA_HMI/BSH-PowerBox/` (`Tag_Map.md`, `Tags.json`, `HMI_View.md` not designed yet).

## What stays the same

Eight devices, same IPs, shared Device UDT, tag paths `<Bldg>/<System>/<Name>`, and every tag of the other six systems.

## What changes

| Item | Change |
|---|---|
| `Tag_Map.md` + `Tags.json` (BSH-PowerBox) | `FanCmd`, `FanMismatch`, `FanStatus` out. In: `ElevRailCmd`, `FireRailCmd`, `DoorRailCmd` (write) and `ElevRailStatus`, `FireRailStatus`, `DoorRailStatus` (read). Addresses per `Register_Map_DRAFT.md` §3. Both files change in the same sitting. |
| PowerBox screen | three rail controls, each showing powered or cut (lights dropped Oct 28, 2026) |
| Campus_Map | each node gets a power state beside its alive/dead state |

## Derived node state (idea)

| Rail status | Heartbeat | Meaning |
|---|---|---|
| powered | ticking | healthy |
| cut | frozen | expected blackout |
| powered | frozen | **fault** (WiFi drop, crash, or unplugged) |

Needs a "heartbeat frozen" check built in Ignition. Not designed.

## Cascade ideas (none built; `SCADA_HMI/Cascade/` has no paths yet)

- Fire alarm active in a building: SCADA cuts the Elevator rail, keeps the FireAlarm rail powered.
- Rule of thumb: never let a cascade cut the rail of the node that raised the alarm.

## Security angle (idea, mechanism level)

The rail coils live in the PowerBox container's Modbus server (port 502). A write there would cut a building system. A rail change in Ignition with no operator action behind it is a detection angle. Not designed; belongs with `Security_Capture/` scenarios if adopted.

## Resilience

`Piranga_Workflow_v2.md` §7 says each ESP32 + OpenPLC pair keeps running if the hub goes offline. With the NO contact (decided Oct 27, 2026) a failed or rebooting PowerBox darkens its three nodes, and rails also turn off on comms loss (decision 7), so that claim no longer holds for the PowerBox: it is a single point of failure for three systems. Accepted by Luis.

## Timing

The real SCADA order in `STATUS.md` is unchanged: Device UDT and tag path rule, then BSH-FireAlarm tags. Fix the PowerBox register layout **before** importing the PowerBox tags.
