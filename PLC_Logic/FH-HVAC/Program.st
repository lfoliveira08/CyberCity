(* FH-HVAC -- OpenPLC Structured Text program
   DRAFT Oct 11, 2026 -- NOT yet compiled in OpenPLC Editor and NOT yet run on a container.
   Source of truth for registers: FH_Systems/01_FH-HVAC.md (folder wins over any copy).
   SCADA_* = bits/words SCADA writes into this PLC's own Modbus server (coil n = %QXn, holding n = %QWn).
   PUB_*   = values this program publishes for SCADA to read from the same server.
   Slave-device addresses (%IX100.x, %QX100.x, %IW100+, %QW100+) follow the OpenPLC v3 convention -- VERIFY on the Monitoring page. *)
PROGRAM prog0
  VAR
    SCADA_HVAC_ENABLE AT %QX0.0 : BOOL;
    SCADA_TARGET_TEMP_C AT %QW0 : UINT;
    PUB_COMPRESSOR_RUNNING AT %QX0.1 : BOOL;
    PUB_HVAC_FAULT AT %QX0.2 : BOOL;
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

  (* Forward SCADA commands to the ESP32 *)
  HVAC_ENABLE := SCADA_HVAC_ENABLE;
  TARGET_TEMP_C := SCADA_TARGET_TEMP_C;

  (* Publish node status for SCADA *)
  PUB_COMPRESSOR_RUNNING := COMPRESSOR_RUNNING;
  PUB_HVAC_FAULT := HVAC_FAULT;
  PUB_TEMP_C := TEMP_C;
  PUB_HUMIDITY_PCT := HUMIDITY_PCT;
  PUB_HEARTBEAT := HEARTBEAT;
  PUB_RECONNECT_COUNT := RECONNECT_COUNT;
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : prog0;
  END_RESOURCE
END_CONFIGURATION
