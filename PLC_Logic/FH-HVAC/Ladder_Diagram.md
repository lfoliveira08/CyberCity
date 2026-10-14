# FH-HVAC -- Ladder diagram

> DRAFT Oct 11, 2026. Not yet built or tested in OpenPLC Editor. Drawn from `FH_Systems/01_FH-HVAC.md`.
> Addresses marked in `Register_Map.md` as "verify" must be confirmed on a real container before this is trusted.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: forward SCADA command to the ESP32 (Forces unit off while clear)
    %QX0.0                %QX100.0
 +--[ ]-------------------( )---+
    SCADA_HVAC_ENABLE     HVAC_ENABLE

RUNG 2: forward SCADA setpoint to the ESP32 holding register (Master-set setpoint)
  %QW0  ->  %QW100
 +----| |-----[ MOVE  SCADA_TARGET_TEMP_C  ->  TARGET_TEMP_C ]------+
   (TRUE)

RUNG 3: publish COMPRESSOR_RUNNING for SCADA
    %IX100.0               %QX0.1
 +--[ ]--------------------( )---+
    COMPRESSOR_RUNNING     PUB_COMPRESSOR_RUNNING

RUNG 4: publish HVAC_FAULT for SCADA
    %IX100.1       %QX0.2
 +--[ ]------------( )---+
    HVAC_FAULT     PUB_HVAC_FAULT

RUNG 5: publish TEMP_C for SCADA
 +----| |-----[ MOVE  TEMP_C (%IW100)  ->  PUB_TEMP_C (%QW1) ]------+
   (TRUE)

RUNG 6: publish HUMIDITY_PCT for SCADA
 +----| |-----[ MOVE  HUMIDITY_PCT (%IW101)  ->  PUB_HUMIDITY_PCT (%QW2) ]------+
   (TRUE)

RUNG 7: publish HEARTBEAT for SCADA
 +----| |-----[ MOVE  HEARTBEAT (%IW102)  ->  PUB_HEARTBEAT (%QW3) ]------+
   (TRUE)

RUNG 8: publish RECONNECT_COUNT for SCADA
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW103)  ->  PUB_RECONNECT_COUNT (%QW4) ]------+
   (TRUE)

```

## In plain language

1. When SCADA sets `SCADA_HVAC_ENABLE`, the program sets the ESP32's `HVAC_ENABLE` coil (Forces unit off while clear).
2. SCADA's `SCADA_TARGET_TEMP_C` value is copied to the ESP32's `TARGET_TEMP_C` holding register (Master-set setpoint).
3-8. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

DECISION PENDING: which way HVAC_ENABLE fails on comms loss (fail-off vs fail-open) is not decided in the system doc yet. This draft is pass-through only; add the comms-loss rung once that is decided.
