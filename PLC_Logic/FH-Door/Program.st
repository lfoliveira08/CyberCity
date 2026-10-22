(* FH-Door -- OpenPLC Structured Text program
   Generated Oct 21, 2026 from Luis's OpenPLC Editor build (project FH-Door-ESP32/):
   the variable block comes from pous/programs/main.ld and the logic from its 8 rungs
   (R1-R8, wiring checked against the saved project). Wrapped here with
   CONFIGURATION/RESOURCE so the v3 web UI (iec2c) can compile it -- the Editor's
   own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor; it is behaviorally
   identical to the Oct 11 hand-draft (replaced by this file; no backup kept).
   Registers source of truth: FH_Systems/03_FH-Door.md (folder wins over any copy).
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave-block addresses (%IX100.x, %QX100.x, %IW100+) follow the convention verified
   on BSH-FireAlarm and BSH-Door; confirm them on this container's Slave Devices page. *)
PROGRAM main
  VAR
    SCADA_DOOR_OPEN_CMD AT %QX0.0 : BOOL;
    SCADA_DOOR_CLOSE_CMD AT %QX0.1 : BOOL;
    PUB_REMOTE_OPEN_NO_CARD AT %QX0.2 : BOOL;
    PUB_IR_DETECTED AT %QX0.3 : BOOL;
    PUB_DOOR_POSITION AT %QX0.4 : BOOL;
    PUB_ACCESS_GRANTED AT %QX0.5 : BOOL;
    PUB_HEARTBEAT AT %QW0 : UINT;
    PUB_RECONNECT_COUNT AT %QW1 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    DOOR_OPEN_CMD AT %QX100.0 : BOOL;
    DOOR_CLOSE_CMD AT %QX100.1 : BOOL;
    IR_DETECTED AT %IX100.0 : BOOL;
    DOOR_POSITION AT %IX100.1 : BOOL;
    ACCESS_GRANTED AT %IX100.2 : BOOL;
    HEARTBEAT AT %IW100 : UINT;
    RECONNECT_COUNT AT %IW101 : UINT;
  END_VAR

  (* R1 ForwardOpenCommand -- forward the SCADA open command to the ESP32 coil *)
  DOOR_OPEN_CMD := SCADA_DOOR_OPEN_CMD;

  (* R2 ForwardCloseCommand -- forward the SCADA close command to the ESP32 coil *)
  DOOR_CLOSE_CMD := SCADA_DOOR_CLOSE_CMD;

  (* R3 ComputeRemoteOpenNoCard -- open commanded with no accepted card *)
  PUB_REMOTE_OPEN_NO_CARD := SCADA_DOOR_OPEN_CMD AND NOT(ACCESS_GRANTED);

  (* R4-R6 publish the ESP32 discrete inputs for SCADA *)
  PUB_IR_DETECTED := IR_DETECTED;
  PUB_DOOR_POSITION := DOOR_POSITION;
  PUB_ACCESS_GRANTED := ACCESS_GRANTED;

  (* R7-R8 publish the ESP32 counters for SCADA (MOVE copies the 16-bit value) *)
  PUB_HEARTBEAT := MOVE(HEARTBEAT);
  PUB_RECONNECT_COUNT := MOVE(RECONNECT_COUNT);
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : main;
  END_RESOURCE
END_CONFIGURATION
