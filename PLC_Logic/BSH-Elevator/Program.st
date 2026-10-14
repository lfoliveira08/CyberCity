(* BSH-Elevator -- OpenPLC Structured Text program
   DRAFT Oct 11, 2026 -- NOT yet compiled in OpenPLC Editor and NOT yet run on a container.
   Source of truth for registers: BSH_Systems/01_BSH-Elevator.md (folder wins over any copy).
   SCADA_* = bits/words SCADA writes into this PLC's own Modbus server (coil n = %QXn, holding n = %QWn).
   PUB_*   = values this program publishes for SCADA to read from the same server.
   Slave-device addresses (%IX100.x, %QX100.x, %IW100+, %QW100+) follow the OpenPLC v3 convention -- VERIFY on the Monitoring page. *)
PROGRAM prog0
  VAR
    SCADA_CALL_FLOOR_1 AT %QX0.0 : BOOL;
    SCADA_CALL_FLOOR_2 AT %QX0.1 : BOOL;
    SCADA_CALL_FLOOR_3 AT %QX0.2 : BOOL;
    SCADA_ELEVATOR_DISABLE AT %QX0.3 : BOOL;
    PUB_ELEVATOR_MOVING AT %QX0.4 : BOOL;
    PUB_ELEVATOR_FAULT AT %QX0.5 : BOOL;
    PUB_CURRENT_FLOOR AT %QW0 : UINT;
    PUB_HEARTBEAT AT %QW1 : UINT;
    PUB_RECONNECT_COUNT AT %QW2 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    CALL_FLOOR_1 AT %QX100.0 : BOOL;
    CALL_FLOOR_2 AT %QX100.1 : BOOL;
    CALL_FLOOR_3 AT %QX100.2 : BOOL;
    ELEVATOR_DISABLE AT %QX100.3 : BOOL;
    ELEVATOR_MOVING AT %IX100.0 : BOOL;
    ELEVATOR_FAULT AT %IX100.1 : BOOL;
    CURRENT_FLOOR AT %IW100 : UINT;
    HEARTBEAT AT %IW101 : UINT;
    RECONNECT_COUNT AT %IW102 : UINT;
  END_VAR

  (* Forward SCADA commands to the ESP32 *)
  CALL_FLOOR_1 := SCADA_CALL_FLOOR_1;
  CALL_FLOOR_2 := SCADA_CALL_FLOOR_2;
  CALL_FLOOR_3 := SCADA_CALL_FLOOR_3;
  ELEVATOR_DISABLE := SCADA_ELEVATOR_DISABLE;

  (* Publish node status for SCADA *)
  PUB_ELEVATOR_MOVING := ELEVATOR_MOVING;
  PUB_ELEVATOR_FAULT := ELEVATOR_FAULT;
  PUB_CURRENT_FLOOR := CURRENT_FLOOR;
  PUB_HEARTBEAT := HEARTBEAT;
  PUB_RECONNECT_COUNT := RECONNECT_COUNT;
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : prog0;
  END_RESOURCE
END_CONFIGURATION
