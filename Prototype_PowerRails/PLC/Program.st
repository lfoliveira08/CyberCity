(* PowerRails -- PROTOTYPE OpenPLC Structured Text program (DRAFT, Oct 28, 2026)
   NOT the real BSH-PowerBox program and NOT built by Luis in the OpenPLC Editor.
   Written from Prototype_PowerRails/Register_Map_DRAFT.md and Ladder_DRAFT.md
   (8 plain rungs, same shape as PLC_Logic/BSH-PowerBox/Program.st). Never compiled.
   UPLOAD WARNING: uploading this to a real container replaces the program it runs.
   ON HOLD (Luis, Oct 28, 2026): nothing goes into the real design, including the real
   BSH-PowerBox container (.114), until Luis confirms. If he later confirms a test there, afterwards
   restore the real program from PLC_Logic/BSH-PowerBox/Program.st and the slave device
   from PLC_Logic/BSH-PowerBox/Register_Map.md section 2. Never use any other real container.
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave device in OpenPLC: IP of the PROTOTYPE ESP32, port 502, slave ID 0,
   discrete inputs start 0 size 3, coils start 0 size 3, input registers start 0 size 2,
   holding registers read and write size 0. Slave block: %IX100.0-2, %QX100.0-2, %IW100-101. *)
PROGRAM main
  VAR
    SCADA_ELEV_RAIL_CMD AT %QX0.0 : BOOL;
    SCADA_FIRE_RAIL_CMD AT %QX0.1 : BOOL;
    SCADA_DOOR_RAIL_CMD AT %QX0.2 : BOOL;
    PUB_ELEV_RAIL_STATUS AT %QX0.3 : BOOL;
    PUB_FIRE_RAIL_STATUS AT %QX0.4 : BOOL;
    PUB_DOOR_RAIL_STATUS AT %QX0.5 : BOOL;
    PUB_HEARTBEAT AT %QW0 : UINT;
    PUB_RECONNECT_COUNT AT %QW1 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    ELEV_RAIL_CMD AT %QX100.0 : BOOL;
    FIRE_RAIL_CMD AT %QX100.1 : BOOL;
    DOOR_RAIL_CMD AT %QX100.2 : BOOL;
    ELEV_RAIL_STATUS AT %IX100.0 : BOOL;
    FIRE_RAIL_STATUS AT %IX100.1 : BOOL;
    DOOR_RAIL_STATUS AT %IX100.2 : BOOL;
    HEARTBEAT AT %IW100 : UINT;
    RECONNECT_COUNT AT %IW101 : UINT;
  END_VAR

  (* R1-R3 forward the SCADA rail commands to the ESP32 coils (level copy) *)
  ELEV_RAIL_CMD := SCADA_ELEV_RAIL_CMD;
  FIRE_RAIL_CMD := SCADA_FIRE_RAIL_CMD;
  DOOR_RAIL_CMD := SCADA_DOOR_RAIL_CMD;

  (* R4-R6 publish the ESP32 rail status inputs for SCADA *)
  PUB_ELEV_RAIL_STATUS := ELEV_RAIL_STATUS;
  PUB_FIRE_RAIL_STATUS := FIRE_RAIL_STATUS;
  PUB_DOOR_RAIL_STATUS := DOOR_RAIL_STATUS;

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
