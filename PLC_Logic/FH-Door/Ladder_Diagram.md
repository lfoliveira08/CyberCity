# FH-Door -- Ladder diagram

> DRAFT Oct 11, 2026. Not yet built or tested in OpenPLC Editor. Drawn from `FH_Systems/03_FH-Door.md`.
> Addresses marked in `Register_Map.md` as "verify" must be confirmed on a real container before this is trusted.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: forward SCADA command to the ESP32 (Command to open (remote override))
    %QX0.0                  %QX100.0
 +--[ ]---------------------( )---+
    SCADA_DOOR_OPEN_CMD     DOOR_OPEN_CMD

RUNG 2: forward SCADA command to the ESP32 (Command to close)
    %QX0.1                   %QX100.1
 +--[ ]----------------------( )---+
    SCADA_DOOR_CLOSE_CMD     DOOR_CLOSE_CMD

RUNG 3: compute flag PUB_REMOTE_OPEN_NO_CARD
    %QX0.0                %IX100.2                %QX0.2
 +--[ ]-------------------[/]---------------------( )---+
    SCADA_DOOR_OPEN_CMD   ACCESS_GRANTED          PUB_REMOTE_OPEN_NO_CARD

RUNG 4: publish IR_DETECTED for SCADA
    %IX100.0        %QX0.3
 +--[ ]-------------( )---+
    IR_DETECTED     PUB_IR_DETECTED

RUNG 5: publish DOOR_POSITION for SCADA
    %IX100.1          %QX0.4
 +--[ ]---------------( )---+
    DOOR_POSITION     PUB_DOOR_POSITION

RUNG 6: publish ACCESS_GRANTED for SCADA
    %IX100.2           %QX0.5
 +--[ ]----------------( )---+
    ACCESS_GRANTED     PUB_ACCESS_GRANTED

RUNG 7: publish HEARTBEAT for SCADA
 +----| |-----[ MOVE  HEARTBEAT (%IW100)  ->  PUB_HEARTBEAT (%QW0) ]------+
   (TRUE)

RUNG 8: publish RECONNECT_COUNT for SCADA
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW101)  ->  PUB_RECONNECT_COUNT (%QW1) ]------+
   (TRUE)

```

## In plain language

1. When SCADA sets `SCADA_DOOR_OPEN_CMD`, the program sets the ESP32's `DOOR_OPEN_CMD` coil (Command to open (remote override)).
2. When SCADA sets `SCADA_DOOR_CLOSE_CMD`, the program sets the ESP32's `DOOR_CLOSE_CMD` coil (Command to close).
3. `PUB_REMOTE_OPEN_NO_CARD` = `SCADA_DOOR_OPEN_CMD AND NOT ACCESS_GRANTED`. Open command with no accepted card (authorized vs injected access contrast). Optional rung; ACCESS_GRANTED may be momentary, so SCADA should judge it over time.
4-8. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

The ESP32 keeps its own safe-state watchdog. The PLC forwards commands and publishes the access-vs-command contrast.
