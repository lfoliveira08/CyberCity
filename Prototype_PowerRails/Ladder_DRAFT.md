# Ladder — DRAFT (BSH-PowerBox power rails)

**Draft, Oct 27, 2026. Not built in the OpenPLC Editor, no `Program.st`.** Luis builds the rungs himself, one at a time. The real ladder is `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md` (8 rungs) and stays as is until adoption.

The other six systems' ladders do **not** change. Cutting a node's power leaves its container running; the node's values freeze, and SCADA sees the frozen heartbeat (`BSH_Systems/04_BSH-PowerBox.md`, Notes: OpenPLC keeps answering with stale values).

## Rung list (11 rungs, same task `T#20ms`)

| Rung | Name | Same shape as | What it does |
|---|---|---|---|
| R1 | ForwardLightsCommand | real R1 | copy `SCADA_LIGHTS_ON_CMD` to the ESP32 coil |
| R2 | ForwardElevRailCommand | real R2 (fan) | copy `SCADA_ELEV_RAIL_CMD` to `ELEV_RAIL_CMD` |
| R3 | ForwardFireRailCommand | real R2 | copy `SCADA_FIRE_RAIL_CMD` to `FIRE_RAIL_CMD` |
| R4 | ForwardDoorRailCommand | real R2 | copy `SCADA_DOOR_RAIL_CMD` to `DOOR_RAIL_CMD` |
| R5 | ComputeLightsMismatch | real R3 | command XOR status for lights only |
| R6 | PublishLightsStatus | real R5 | `LIGHTS_STATUS` to `PUB_LIGHTS_STATUS` |
| R7 | PublishElevRailStatus | real R6 | `ELEV_RAIL_STATUS` to `PUB_ELEV_RAIL_STATUS` |
| R8 | PublishFireRailStatus | real R6 | same for FireAlarm |
| R9 | PublishDoorRailStatus | real R6 | same for Door |
| R10 | PublishHeartbeat | real R7 | MOVE `HEARTBEAT` (%IW100) to `PUB_HEARTBEAT` (%QW0) |
| R11 | PublishReconnectCount | real R8 | MOVE `RECONNECT_COUNT` (%IW101) to `PUB_RECONNECT_COUNT` (%QW1) |

The fan rungs R2, R4 and R6 are what disappear; the three rail forwards (R2-R4) and three rail status publishes (R7-R9) replace them. The mismatch rung for rails is dropped on purpose (see `Register_Map_DRAFT.md` §3).

## Teaching notes for when it is built

- Rail forwards are plain level copies, exactly like the as-built fan rung. The PLC forwards the command as is. With the NO contact there is no inversion anywhere: rail on = coil on.
- The ESP32 counts a coil write as a hub command only when its value differs from the last hub value (Oct 21 rule, `Firmware/CLAUDE.md`). To force the same rail state again, write the opposite value, then the original.
- Addresses: `%QX0.x` is what SCADA reads and writes (the PLC's own server); `%IX100.x`, `%QX100.x`, `%IW100+` are the ESP32 block the PLC polls.
