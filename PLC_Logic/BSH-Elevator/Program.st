(* BSH-Elevator -- OpenPLC Structured Text program
   Generated Oct 20, 2026 from Luis's OpenPLC Editor build (project BSH-Elevator-ESP32/):
   the variable block comes from pous/programs/main.ld and the logic from its 9 rungs
   (R1-R9, wiring checked against the saved project). Wrapped here with
   CONFIGURATION/RESOURCE so the v3 web UI (iec2c) can compile it -- the Editor's
   own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor; it replaces the
   Oct 11 hand-draft. Unlike that draft, the three call-floor rungs use a TP pulse
   timer (design decision of Oct 20, 2026, see Build_Log.md).
   Registers source of truth: BSH_Systems/01_BSH-Elevator.md (folder wins over any copy).
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave-block addresses (%IX100.x, %QX100.x, %IW100+) follow the convention verified
   on BSH-FireAlarm and BSH-Door; confirm them on this container's Slave Devices page. *)
PROGRAM main
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

  (* matiec rule: a VAR block with located variables (AT %...) holds only located
     variables, so the unlocated ones sit in their own VAR block. *)
  VAR
    (* ---- pulse stretch for the call-floor commands ---- *)
    CALL_PULSE_TIME : TIME := T#300ms;
    TP0 : TP;
    TP1 : TP;
    TP2 : TP;
  END_VAR

  (* R1 PulseCallFloor1 -- one press of SCADA_CALL_FLOOR_1 gives one 300 ms pulse to the ESP32 coil *)
  TP0(IN := SCADA_CALL_FLOOR_1, PT := CALL_PULSE_TIME);
  CALL_FLOOR_1 := TP0.Q;

  (* R2 PulseCallFloor2 *)
  TP1(IN := SCADA_CALL_FLOOR_2, PT := CALL_PULSE_TIME);
  CALL_FLOOR_2 := TP1.Q;

  (* R3 PulseCallFloor3 (the saved project's rung comment says R2_PulseCallFloor3, a typo) *)
  TP2(IN := SCADA_CALL_FLOOR_3, PT := CALL_PULSE_TIME);
  CALL_FLOOR_3 := TP2.Q;

  (* R4 ForwardElevatorDisable -- level copy: the ESP32 treats DISABLE as a lockout level *)
  ELEVATOR_DISABLE := SCADA_ELEVATOR_DISABLE;

  (* R5-R6 publish the ESP32 discrete inputs for SCADA *)
  PUB_ELEVATOR_MOVING := ELEVATOR_MOVING;
  PUB_ELEVATOR_FAULT := ELEVATOR_FAULT;

  (* R7-R9 publish the ESP32 input registers for SCADA (MOVE copies the 16-bit value) *)
  PUB_CURRENT_FLOOR := MOVE(CURRENT_FLOOR);
  PUB_HEARTBEAT := MOVE(HEARTBEAT);
  PUB_RECONNECT_COUNT := MOVE(RECONNECT_COUNT);
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : main;
  END_RESOURCE
END_CONFIGURATION
