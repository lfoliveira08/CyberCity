# BSH-PowerBox -- Ladder diagram

> **Rewritten Oct 28, 2026** for the power-rails design (adoption Step 4). Replaces the lights-and-fan ladder, archived in `Archive/BSH-PowerBox_PLC_lights-and-fan/`. **Built in the OpenPLC Editor by Luis (project `BSH-PowerBox-ESP32/`, task `T#20ms`, saved Oct 28, 2026, 18:25) and checked against the saved project file; `Program.st` was regenerated from it.** Not yet uploaded to the container from this `Program.st`; container .114 ran a draft with the same addresses (Luis-reported). Addresses: `Register_Map.md` section 3.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: R1_ForwardElevRailCommand -- forward the SCADA command to the ESP32
    %QX0.0                      %QX100.0
 +--[ ]---------------------------( )---+
    SCADA_ELEV_RAIL_CMD         ELEV_RAIL_CMD

RUNG 2: R2_ForwardFireRailCommand
    %QX0.1                      %QX100.1
 +--[ ]---------------------------( )---+
    SCADA_FIRE_RAIL_CMD         FIRE_RAIL_CMD

RUNG 3: R3_ForwardDoorRailCommand
    %QX0.2                      %QX100.2
 +--[ ]---------------------------( )---+
    SCADA_DOOR_RAIL_CMD         DOOR_RAIL_CMD

RUNG 4: R4_PublishElevRailStatus -- publish the ESP32 status for SCADA
    %IX100.0                    %QX0.3
 +--[ ]---------------------------( )---+
    ELEV_RAIL_STATUS            PUB_ELEV_RAIL_STATUS

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

1-3. When SCADA sets a rail command bit, the program sets the matching ESP32 coil. It is a plain level copy; the program rewrites it every scan (the ESP32 only counts a write when its value changes).
4-6. Each ESP32 rail status input is copied to a `PUB_*` address so SCADA reads it from this PLC's own Modbus server.
7-8. The two counters are copied the same way.

## How this differs from the old ladder

The old ladder (8 rungs) copied lights and fan commands, computed two mismatch flags (command XOR status), published two status bits and the two counters. Rungs 1 and 2 become the Elevator and FireAlarm rail commands, a Door rung is new, the two mismatch rungs are gone (a rail command and its status can never disagree here), the status rungs now cover three rails, and the counter rungs are unchanged.

## Notes

The ESP32 turns every rail off after 5 s without WiFi and keeps them off after power-up until SCADA (or a Modbus write) commands them on. The PLC forwards commands; it does not decide safety.
