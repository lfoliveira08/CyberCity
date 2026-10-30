(* FH-PowerBox -- OpenPLC Structured Text program
   Generated Oct 21, 2026 from Luis's OpenPLC Editor build (project FH-PowerBox-ESP32/):
   the variable block comes from pous/programs/main.ld and the logic from its 8 rungs
   (R1-R8, wiring checked against the saved project). Wrapped here with
   CONFIGURATION/RESOURCE so the v3 web UI (iec2c) can compile it -- the Editor's
   own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor; it replaces the
   Oct 11 hand-draft (same logic: plain level copies, option A, mirror of BSH-PowerBox).
   Registers source of truth: FH_Systems/04_FH-PowerBox.md (folder wins over any copy).
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave-block addresses (%IX100.x, %QX100.x, %IW100+) follow the convention verified
   on the earlier nodes; confirm them on this container's Slave Devices page. *)
PROGRAM main
  VAR
    SCADA_LIGHTS_ON_CMD AT %QX0.0 : BOOL;
    SCADA_FAN_CMD AT %QX0.1 : BOOL;
    PUB_LIGHTS_MISMATCH AT %QX0.2 : BOOL;
    PUB_FAN_MISMATCH AT %QX0.3 : BOOL;
    PUB_LIGHTS_STATUS AT %QX0.4 : BOOL;
    PUB_FAN_STATUS AT %QX0.5 : BOOL;
    PUB_HEARTBEAT AT %QW0 : UINT;
    PUB_RECONNECT_COUNT AT %QW1 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    LIGHTS_ON_CMD AT %QX100.0 : BOOL;
    FAN_CMD AT %QX100.1 : BOOL;
    LIGHTS_STATUS AT %IX100.0 : BOOL;
    FAN_STATUS AT %IX100.1 : BOOL;
    HEARTBEAT AT %IW100 : UINT;
    RECONNECT_COUNT AT %IW101 : UINT;
  END_VAR

  (* R1 ForwardLightsCommand -- copy the SCADA lights command to the ESP32 coil (level copy) *)
  LIGHTS_ON_CMD := SCADA_LIGHTS_ON_CMD;

  (* R2 ForwardFanCommand -- copy the SCADA fan command to the ESP32 coil (level copy) *)
  FAN_CMD := SCADA_FAN_CMD;

  (* R3 ComputeLightsMismatch -- XOR: commanded state and actual state disagree *)
  PUB_LIGHTS_MISMATCH := (SCADA_LIGHTS_ON_CMD AND NOT(LIGHTS_STATUS)) OR (NOT(SCADA_LIGHTS_ON_CMD) AND LIGHTS_STATUS);

  (* R4 ComputeFanMismatch -- XOR: commanded state and actual state disagree *)
  PUB_FAN_MISMATCH := (SCADA_FAN_CMD AND NOT(FAN_STATUS)) OR (NOT(SCADA_FAN_CMD) AND FAN_STATUS);

  (* R5-R6 publish the ESP32 discrete inputs for SCADA *)
  PUB_LIGHTS_STATUS := LIGHTS_STATUS;
  PUB_FAN_STATUS := FAN_STATUS;

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
