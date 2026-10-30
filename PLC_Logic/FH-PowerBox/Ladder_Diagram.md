# FH-PowerBox -- Ladder diagram

> **Rewritten Oct 28, 2026, mirroring `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md`.** Replaces the lights-and-fan ladder, archived in `Archive/FH-PowerBox_PLC_lights-and-fan/`. **Built in the OpenPLC Editor by Luis (project `FH-PowerBox-ESP32/`, edited in place, task `T#20ms`, saved Oct 28, 2026, 19:33) and checked against the saved project file; `Program.st` was regenerated from it.** Not yet uploaded to the container from this `Program.st`.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read.

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: R1_ForwardHvacRailCommand -- forward the SCADA command to the ESP32
    %QX0.0                      %QX100.0
 +--[ ]---------------------------( )---+
    SCADA_HVAC_RAIL_CMD         HVAC_RAIL_CMD

RUNG 2: R2_ForwardFireRailCommand
    %QX0.1                      %QX100.1
 +--[ ]---------------------------( )---+
    SCADA_FIRE_RAIL_CMD         FIRE_RAIL_CMD

RUNG 3: R3_ForwardDoorRailCommand
    %QX0.2                      %QX100.2
 +--[ ]---------------------------( )---+
    SCADA_DOOR_RAIL_CMD         DOOR_RAIL_CMD

RUNG 4: R4_PublishHvacRailStatus -- publish the ESP32 status for SCADA
    %IX100.0                    %QX0.3
 +--[ ]---------------------------( )---+
    HVAC_RAIL_STATUS            PUB_HVAC_RAIL_STATUS

RUNG 5: R5_PublishFireRailStatus
    %IX100.1                    %QX0.4
 +--[ ]---------------------------( )---+
    FIRE_RAIL_STATUS            PUB_FIRE_RAIL_STATUS

RUNG 6: R6_PublishDoorRailStatus
    %IX100.2                    %QX0.5
 +--[ ]---------------------------( )---+
    DOOR_RAIL_STATUS            PUB_DOOR_RAIL_STATUS

RUNG 7: R7_PublishHeartbeat
 +----| |-----[ MOVE  HEARTBEAT (%IW100)  ->  PUB_HEARTBEAT (%QW0) ]------+
   (TRUE)

RUNG 8: R8_PublishReconnectCount
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW101)  ->  PUB_RECONNECT_COUNT (%QW1) ]------+
   (TRUE)
```

## In plain language

Identical to BSH-PowerBox's: 1-3 forward the three rail commands as plain level copies, 4-6 publish the three status bits, 7-8 publish the two counters. No mismatch rungs, same reasoning as BSH.

## Notes

Build these 8 rungs in the Editor the same way as BSH-PowerBox's rungs were built: start with R1 (same two-element shape — a normal contact into a normal coil), then repeat for R2-R6, then the two MOVE blocks for R7-R8. Renaming the existing lights/fan rungs' variables, where the old ladder used the same address pattern, is the quickest path (see how BSH-PowerBox's rungs 1 and 2 reused the old addresses).
