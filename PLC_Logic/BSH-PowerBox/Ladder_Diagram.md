# BSH-PowerBox -- Ladder diagram

> BUILT Oct 21, 2026 by Luis in the OpenPLC Editor (project `BSH-PowerBox-ESP32/`, program `main`, task `T#20ms`) and run on the container; the drawing below is the original Oct 11 design and the saved project matches it. Rung comments in the Editor: R1_ForwardLightsCommand, R2_ForwardFanCommand, R3_ComputeLightsMismatch, R4_ComputeFanMismatch, R5_PublishLightsStatus, R6_PublishFanStatus, R7_PublishHeartbeat, R8_PublishReconnectCount. R3 and R4 are two series branches in parallel (command AND NOT status, OR NOT command AND status). The slave-block addresses were confirmed on the container's Slave Devices page. Drawn from `BSH_Systems/04_BSH-PowerBox.md`.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: forward SCADA command to the ESP32 (Lights on/off command)
    %QX0.0                  %QX100.0
 +--[ ]---------------------( )---+
    SCADA_LIGHTS_ON_CMD     LIGHTS_ON_CMD

RUNG 2: forward SCADA command to the ESP32 (Fan on/off command)
    %QX0.1            %QX100.1
 +--[ ]---------------( )---+
    SCADA_FAN_CMD     FAN_CMD

RUNG 3: compute flag PUB_LIGHTS_MISMATCH
    %QX0.0                %IX100.0                          %QX0.2
 +--[ ]-------------------[/]--------------------+----------( )---+
    SCADA_LIGHTS_ON_CMD   LIGHTS_STATUS          |          PUB_LIGHTS_MISMATCH
    %QX0.0                %IX100.0               |
 +--[/]-------------------[ ]--------------------+
    SCADA_LIGHTS_ON_CMD   LIGHTS_STATUS         
   (XOR: true when exactly one of the two is 1)

RUNG 4: compute flag PUB_FAN_MISMATCH
    %QX0.1          %IX100.1                    %QX0.3
 +--[ ]-------------[/]--------------+----------( )---+
    SCADA_FAN_CMD   FAN_STATUS       |          PUB_FAN_MISMATCH
    %QX0.1          %IX100.1         |
 +--[/]-------------[ ]--------------+
    SCADA_FAN_CMD   FAN_STATUS      
   (XOR: true when exactly one of the two is 1)

RUNG 5: publish LIGHTS_STATUS for SCADA
    %IX100.0          %QX0.4
 +--[ ]---------------( )---+
    LIGHTS_STATUS     PUB_LIGHTS_STATUS

RUNG 6: publish FAN_STATUS for SCADA
    %IX100.1       %QX0.5
 +--[ ]------------( )---+
    FAN_STATUS     PUB_FAN_STATUS

RUNG 7: publish HEARTBEAT for SCADA
 +----| |-----[ MOVE  HEARTBEAT (%IW100)  ->  PUB_HEARTBEAT (%QW0) ]------+
   (TRUE)

RUNG 8: publish RECONNECT_COUNT for SCADA
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW101)  ->  PUB_RECONNECT_COUNT (%QW1) ]------+
   (TRUE)

```

## In plain language

1. When SCADA sets `SCADA_LIGHTS_ON_CMD`, the program sets the ESP32's `LIGHTS_ON_CMD` coil (Lights on/off command).
2. When SCADA sets `SCADA_FAN_CMD`, the program sets the ESP32's `FAN_CMD` coil (Fan on/off command).
3. `PUB_LIGHTS_MISMATCH` = `SCADA_LIGHTS_ON_CMD XOR LIGHTS_STATUS`. Last command and actual lights state disagree (e.g. a local button moved it).
4. `PUB_FAN_MISMATCH` = `SCADA_FAN_CMD XOR FAN_STATUS`. Last command and actual fan state disagree.
5-8. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

The ESP32 fails both outputs to OFF on comms loss and keeps local buttons working. The PLC forwards commands and publishes command-vs-actual mismatch.
