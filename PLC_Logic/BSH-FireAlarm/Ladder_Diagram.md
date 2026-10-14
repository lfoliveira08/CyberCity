# BSH-FireAlarm -- Ladder diagram

> DRAFT Oct 11, 2026. Not yet built or tested in OpenPLC Editor. Drawn from `BSH_Systems/02_BSH-FireAlarm.md`.
> Addresses marked in `Register_Map.md` as "verify" must be confirmed on a real container before this is trusted.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: forward SCADA command to the ESP32 (Remote/hub-triggered alarm)
    %QX0.0                   %QX100.0
 +--[ ]----------------------( )---+
    SCADA_ALARM_ACTIVATE     ALARM_ACTIVATE

RUNG 2: compute flag PUB_REMOTE_ONLY
    %QX0.0                  %IX100.0                %IX100.1                %IX100.2                  %QX0.1
 +--[ ]---------------------[/]---------------------[/]---------------------[/]-----------------------( )---+
    SCADA_ALARM_ACTIVATE    MANUAL_PULL_TRIGGERED   FLAME_DETECTED          GAS_SMOKE_ALERT           PUB_REMOTE_ONLY

RUNG 3: publish MANUAL_PULL_TRIGGERED for SCADA
    %IX100.0                  %QX0.2
 +--[ ]-----------------------( )---+
    MANUAL_PULL_TRIGGERED     PUB_MANUAL_PULL_TRIGGERED

RUNG 4: publish FLAME_DETECTED for SCADA
    %IX100.1           %QX0.3
 +--[ ]----------------( )---+
    FLAME_DETECTED     PUB_FLAME_DETECTED

RUNG 5: publish GAS_SMOKE_ALERT for SCADA
    %IX100.2            %QX0.4
 +--[ ]-----------------( )---+
    GAS_SMOKE_ALERT     PUB_GAS_SMOKE_ALERT

RUNG 6: publish GAS_RAW_VALUE for SCADA
 +----| |-----[ MOVE  GAS_RAW_VALUE (%IW100)  ->  PUB_GAS_RAW_VALUE (%QW0) ]------+
   (TRUE)

RUNG 7: publish HEARTBEAT for SCADA
 +----| |-----[ MOVE  HEARTBEAT (%IW101)  ->  PUB_HEARTBEAT (%QW1) ]------+
   (TRUE)

RUNG 8: publish RECONNECT_COUNT for SCADA
 +----| |-----[ MOVE  RECONNECT_COUNT (%IW102)  ->  PUB_RECONNECT_COUNT (%QW2) ]------+
   (TRUE)

```

## In plain language

1. When SCADA sets `SCADA_ALARM_ACTIVATE`, the program sets the ESP32's `ALARM_ACTIVATE` coil (Remote/hub-triggered alarm).
2. `PUB_REMOTE_ONLY` = `SCADA_ALARM_ACTIVATE AND NOT (MANUAL_PULL_TRIGGERED OR FLAME_DETECTED OR GAS_SMOKE_ALERT)`. Alarm commanded remotely with no physical cause (Scenario 4: false fire alarm injection). Optional rung.
3-8. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

The ESP32 already sounds the alarm locally from its own sensors. The PLC only forwards the remote command and publishes status.
