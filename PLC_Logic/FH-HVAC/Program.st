(* FH-HVAC -- OpenPLC Structured Text program
   Generated Oct 21, 2026 from Luis's OpenPLC Editor build (project FH-HVAC-ESP32/):
   the variable block comes from pous/programs/main.ld and the logic from its 8 rungs
   (R1-R8, wiring checked against the saved project). Wrapped here with
   CONFIGURATION/RESOURCE so the v3 web UI (iec2c) can compile it -- the Editor's
   own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor; it replaces the
   Oct 11 hand-draft (same logic: plain level copy for the enable command, MOVE for the setpoint).
   Registers source of truth: FH_Systems/01_FH-HVAC.md (folder wins over any copy).
   SCADA_* = values SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave-block addresses (%IX100.x, %QX100.x, %IW100+, %QW100) follow the convention verified
   on the BSH nodes; confirm them on this container's Slave Devices page.
   TEMP_C and TARGET_TEMP_C are in tenths of a degree C (240 = 24.0 C). *)
PROGRAM main
  VAR
    SCADA_HVAC_ENABLE AT %QX0.0 : BOOL;
    PUB_COMPRESSOR_RUNNING AT %QX0.1 : BOOL;
    PUB_HVAC_FAULT AT %QX0.2 : BOOL;
    SCADA_TARGET_TEMP_C AT %QW0 : UINT;
    PUB_TEMP_C AT %QW1 : UINT;
    PUB_HUMIDITY_PCT AT %QW2 : UINT;
    PUB_HEARTBEAT AT %QW3 : UINT;
    PUB_RECONNECT_COUNT AT %QW4 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    HVAC_ENABLE AT %QX100.0 : BOOL;
    TARGET_TEMP_C AT %QW100 : UINT;
    COMPRESSOR_RUNNING AT %IX100.0 : BOOL;
    HVAC_FAULT AT %IX100.1 : BOOL;
    TEMP_C AT %IW100 : UINT;
    HUMIDITY_PCT AT %IW101 : UINT;
    HEARTBEAT AT %IW102 : UINT;
    RECONNECT_COUNT AT %IW103 : UINT;
  END_VAR

  (* R1 ForwardEnable -- copy the SCADA enable command to the ESP32 coil (level copy) *)
  HVAC_ENABLE := SCADA_HVAC_ENABLE;

  (* R2 ForwardSetpoint -- copy the SCADA setpoint to the ESP32 holding register (MOVE) *)
  TARGET_TEMP_C := MOVE(SCADA_TARGET_TEMP_C);

  (* R3-R4 publish the ESP32 discrete inputs for SCADA *)
  PUB_COMPRESSOR_RUNNING := COMPRESSOR_RUNNING;
  PUB_HVAC_FAULT := HVAC_FAULT;

  (* R5-R8 publish the ESP32 input registers for SCADA (MOVE copies the 16-bit value) *)
  PUB_TEMP_C := MOVE(TEMP_C);
  PUB_HUMIDITY_PCT := MOVE(HUMIDITY_PCT);
  PUB_HEARTBEAT := MOVE(HEARTBEAT);
  PUB_RECONNECT_COUNT := MOVE(RECONNECT_COUNT);
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : main;
  END_RESOURCE
END_CONFIGURATION
