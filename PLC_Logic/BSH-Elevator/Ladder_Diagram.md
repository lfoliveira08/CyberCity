# BSH-Elevator -- Ladder diagram

> DRAFT Oct 11, 2026. Not yet built or tested in OpenPLC Editor. Drawn from `BSH_Systems/01_BSH-Elevator.md`.
> Addresses marked in `Register_Map.md` as "verify" must be confirmed on a real container before this is trusted.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: forward SCADA command to the ESP32 (Call to floor 1)
    %QX0.0                 %QX100.0
 +--[ ]--------------------( )---+
    SCADA_CALL_FLOOR_1     CALL_FLOOR_1

RUNG 2: forward SCADA command to the ESP32 (Call to floor 2)
    %QX0.1                 %QX100.1
 +--[ ]--------------------( )---+
    SCADA_CALL_FLOOR_2     CALL_FLOOR_2

RUNG 3: forward SCADA command to the ESP32 (Call to floor 3)
    %QX0.2                 %QX100.2
 +--[ ]--------------------( )---+
    SCADA_CALL_FLOOR_3     CALL_FLOOR_3

RUNG 4: forward SCADA command to the ESP32 (Lock out the elevator)
    %QX0.3                     %QX100.3
 +--[ ]------------------------( )---+
    SCADA_ELEVATOR_DISABLE     ELEVATOR_DISABLE

RUNG 5: publish ELEVATOR_MOVING for SCADA
    %IX100.0            %QX0.4
 +--[ ]-----------------( )---+
    ELEVATOR_MOVING     PUB_ELEVATOR_MOVING

RUNG 6: publish ELEVATOR_FAULT for SCADA
    %IX100.1           %QX0.5
 +--[ ]----------------( )---+
    ELEVATOR_FAULT     PUB_ELEVATOR_FAULT

RUNG 7: publish CURRENT_FLOOR for SCADA
 +----| |-----[ MOVE  CURRENT_FLOOR (%IW100)  ->  PUB_CURRENT_FLOOR (%QW0) ]------+
   (TRUE)

RUNG 8: publish HEARTBEAT for SCADA
 +----| |-----[ MOVE  HEARTBEAT (%IW101)  ->  PUB_HEARTBEAT (%QW1) ]------+
   (TRUE)

RUNG 9: publish RECONNECT_COUNT for SCADA
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW102)  ->  PUB_RECONNECT_COUNT (%QW2) ]------+
   (TRUE)

```

## In plain language

1. When SCADA sets `SCADA_CALL_FLOOR_1`, the program sets the ESP32's `CALL_FLOOR_1` coil (Call to floor 1).
2. When SCADA sets `SCADA_CALL_FLOOR_2`, the program sets the ESP32's `CALL_FLOOR_2` coil (Call to floor 2).
3. When SCADA sets `SCADA_CALL_FLOOR_3`, the program sets the ESP32's `CALL_FLOOR_3` coil (Call to floor 3).
4. When SCADA sets `SCADA_ELEVATOR_DISABLE`, the program sets the ESP32's `ELEVATOR_DISABLE` coil (Lock out the elevator).
5-9. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

Pure pass-through. Fault handling and the comms-loss behavior live in the ESP32 firmware.
