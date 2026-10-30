# Ladder — DRAFT (BSH-PowerBox power rails)

**Draft, rewritten Oct 28, 2026 after the lights were dropped. Not built in the OpenPLC Editor, no `Program.st`.** Luis builds the rungs himself, one at a time. The real ladder is `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md` (8 rungs) and stays as is until adoption. Replaces the Oct 27 version (11 rungs, with lights).

The other six systems' ladders do **not** change. Cutting a node's power leaves its container running; the node's values freeze, and SCADA sees the frozen heartbeat (`BSH_Systems/04_BSH-PowerBox.md`, Notes: OpenPLC keeps answering with stale values).

## Rung list (8 rungs, same task `T#20ms`)

| Rung | Name | Same shape as | What it does |
|---|---|---|---|
| R1 | ForwardElevRailCommand | real R1 or R2 | copy `SCADA_ELEV_RAIL_CMD` to the ESP32 coil `ELEV_RAIL_CMD` |
| R2 | ForwardFireRailCommand | real R1 or R2 | same for FireAlarm |
| R3 | ForwardDoorRailCommand | real R1 or R2 | same for Door |
| R4 | PublishElevRailStatus | real R5 or R6 | `ELEV_RAIL_STATUS` to `PUB_ELEV_RAIL_STATUS` |
| R5 | PublishFireRailStatus | real R5 or R6 | same for FireAlarm |
| R6 | PublishDoorRailStatus | real R5 or R6 | same for Door |
| R7 | PublishHeartbeat | real R7 | MOVE `HEARTBEAT` (%IW100) to `PUB_HEARTBEAT` (%QW0) |
| R8 | PublishReconnectCount | real R8 | MOVE `RECONNECT_COUNT` (%IW101) to `PUB_RECONNECT_COUNT` (%QW1) |

No mismatch rungs: with no local buttons and no voltage sense a rail mismatch could never be true (see `Register_Map_DRAFT.md` §3).

## Teaching notes for when it is built

- Rail forwards are plain level copies, exactly like the as-built lights and fan rungs. The PLC forwards the command as is. With the NO contact there is no inversion anywhere: rail on = coil on.
- The ESP32 counts a coil write as a hub command only when its value differs from the last hub value (Oct 21 rule, `Firmware/CLAUDE.md`). To force the same rail state again, write the opposite value, then the original.
- Addresses: `%QX0.x` is what SCADA reads and writes (the PLC's own server); `%IX100.x`, `%QX100.x`, `%IW100+` are the ESP32 block the PLC polls.
