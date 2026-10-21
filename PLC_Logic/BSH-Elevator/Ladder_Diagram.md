# BSH-Elevator -- Ladder diagram

> Drawn Oct 11, 2026 from `BSH_Systems/01_BSH-Elevator.md`; built in OpenPLC Editor by Luis and tested on the container Oct 20, 2026 (`Build_Log.md`). R1-R3 below are as built (a `TP` pulse timer), not as first drafted.

Naming: `SCADA_*` = SCADA writes it into this PLC's own Modbus server. `PUB_*` = this program publishes it for SCADA to read. Un-prefixed names are the ESP32's own register names (the slave-device block starting at `%IX100.0` / `%QX100.0` / `%IW100` / `%QW100`).

```
LEGEND   --[ ]--  contact: passes power when the bit is 1
         --[/]--  inverted contact: passes power when the bit is 0
         --( )--  coil: sets its bit to 1 when power reaches it
         [ MOVE ] copies a 16-bit value (enable input tied to TRUE)

RUNG 1: pulse the ESP32 call coil when SCADA presses Call to floor 1
    %QX0.0                  +------+                 %QX100.0
 +--[ ]---------------------| IN  Q|-----------------( )---+
    SCADA_CALL_FLOOR_1      |  TP0 |                  CALL_FLOOR_1
    CALL_PULSE_TIME --------| PT ET|--(unused)
    (TIME, T#300ms)         +------+

RUNG 2: same, Call to floor 2 (own timer instance TP1)
    SCADA_CALL_FLOOR_2 (%QX0.1) -> TP1 (PT = CALL_PULSE_TIME) -> CALL_FLOOR_2 (%QX100.1)

RUNG 3: same, Call to floor 3 (own timer instance TP2)
    SCADA_CALL_FLOOR_3 (%QX0.2) -> TP2 (PT = CALL_PULSE_TIME) -> CALL_FLOOR_3 (%QX100.2)

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

1. When SCADA sets `SCADA_CALL_FLOOR_1` (0 to 1), the `TP` timer raises the ESP32's `CALL_FLOOR_1` coil (Call to floor 1) for 300 ms. A held bit gives one pulse, not a stream of presses.
2. Same for `SCADA_CALL_FLOOR_2` and the ESP32's `CALL_FLOOR_2` coil (Call to floor 2).
3. Same for `SCADA_CALL_FLOOR_3` and the ESP32's `CALL_FLOOR_3` coil (Call to floor 3).
4. When SCADA sets `SCADA_ELEVATOR_DISABLE`, the program sets the ESP32's `ELEVATOR_DISABLE` coil (Lock out the elevator).
5-9. Every ESP32 discrete input and input register is copied to a `PUB_*` address, so SCADA reads it from this PLC's own Modbus server.

## Notes

Why a 300 ms pulse and not a plain copy (decided Oct 20, 2026): the ESP32 treats the call coils as momentary and clears them itself, and the OpenPLC master rewrites the coil every poll, so a held SCADA bit would send the car back to that floor every time it goes idle. A one-scan pulse could fall between two 100 ms polls and be lost (read in the master source, not measured), so the pulse is stretched to 300 ms. The ESP32 may see up to 2-3 rising edges inside the pulse; only the first one starts a trip, the others arrive while the car is busy and are dropped. `ELEVATOR_DISABLE` (rung 4) stays a plain level copy because the firmware treats it as a lockout level. Fault handling and the comms-loss behavior live in the ESP32 firmware.
