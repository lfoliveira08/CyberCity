(* FH-Door -- OpenPLC Structured Text program
   DRAFT Oct 11, 2026 -- NOT yet compiled in OpenPLC Editor and NOT yet run on a container.
   Source of truth for registers: FH_Systems/03_FH-Door.md (folder wins over any copy).
   SCADA_* = bits/words SCADA writes into this PLC's own Modbus server (coil n = %QXn, holding n = %QWn).
   PUB_*   = values this program publishes for SCADA to read from the same server.
   Slave-device addresses (%IX100.x, %QX100.x, %IW100+, %QW100+) follow the OpenPLC v3 convention -- VERIFY on the Monitoring page. *)
PROGRAM prog0
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

  (* Forward SCADA commands to the ESP32 *)
  DOOR_OPEN_CMD := SCADA_DOOR_OPEN_CMD;
  DOOR_CLOSE_CMD := SCADA_DOOR_CLOSE_CMD;

  (* Flags computed in the PLC *)
  PUB_REMOTE_OPEN_NO_CARD := SCADA_DOOR_OPEN_CMD AND NOT ACCESS_GRANTED;

  (* Publish node status for SCADA *)
  PUB_IR_DETECTED := IR_DETECTED;
  PUB_DOOR_POSITION := DOOR_POSITION;
  PUB_ACCESS_GRANTED := ACCESS_GRANTED;
  PUB_HEARTBEAT := HEARTBEAT;
  PUB_RECONNECT_COUNT := RECONNECT_COUNT;
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : prog0;
  END_RESOURCE
END_CONFIGURATION
