(* BSH-PowerBox -- OpenPLC Structured Text program
   DRAFT Oct 11, 2026 -- NOT yet compiled in OpenPLC Editor and NOT yet run on a container.
   Source of truth for registers: BSH_Systems/04_BSH-PowerBox.md (folder wins over any copy).
   SCADA_* = bits/words SCADA writes into this PLC's own Modbus server (coil n = %QXn, holding n = %QWn).
   PUB_*   = values this program publishes for SCADA to read from the same server.
   Slave-device addresses (%IX100.x, %QX100.x, %IW100+, %QW100+) follow the OpenPLC v3 convention -- VERIFY on the Monitoring page. *)
PROGRAM prog0
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

  (* Forward SCADA commands to the ESP32 *)
  LIGHTS_ON_CMD := SCADA_LIGHTS_ON_CMD;
  FAN_CMD := SCADA_FAN_CMD;

  (* Flags computed in the PLC *)
  PUB_LIGHTS_MISMATCH := SCADA_LIGHTS_ON_CMD XOR LIGHTS_STATUS;
  PUB_FAN_MISMATCH := SCADA_FAN_CMD XOR FAN_STATUS;

  (* Publish node status for SCADA *)
  PUB_LIGHTS_STATUS := LIGHTS_STATUS;
  PUB_FAN_STATUS := FAN_STATUS;
  PUB_HEARTBEAT := HEARTBEAT;
  PUB_RECONNECT_COUNT := RECONNECT_COUNT;
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : prog0;
  END_RESOURCE
END_CONFIGURATION
