(* BSH-FireAlarm -- OpenPLC Structured Text program
   DRAFT Oct 11, 2026 -- NOT yet compiled in OpenPLC Editor and NOT yet run on a container.
   Source of truth for registers: BSH_Systems/02_BSH-FireAlarm.md (folder wins over any copy).
   SCADA_* = bits/words SCADA writes into this PLC's own Modbus server (coil n = %QXn, holding n = %QWn).
   PUB_*   = values this program publishes for SCADA to read from the same server.
   Slave-device addresses (%IX100.x, %QX100.x, %IW100+, %QW100+) follow the OpenPLC v3 convention -- VERIFY on the Monitoring page. *)
PROGRAM prog0
  VAR
    SCADA_ALARM_ACTIVATE AT %QX0.0 : BOOL;
    PUB_REMOTE_ONLY AT %QX0.1 : BOOL;
    PUB_MANUAL_PULL_TRIGGERED AT %QX0.2 : BOOL;
    PUB_FLAME_DETECTED AT %QX0.3 : BOOL;
    PUB_GAS_SMOKE_ALERT AT %QX0.4 : BOOL;
    PUB_GAS_RAW_VALUE AT %QW0 : UINT;
    PUB_HEARTBEAT AT %QW1 : UINT;
    PUB_RECONNECT_COUNT AT %QW2 : UINT;
    (* ---- ESP32 slave-device block (OpenPLC master side) ---- *)
    ALARM_ACTIVATE AT %QX100.0 : BOOL;
    MANUAL_PULL_TRIGGERED AT %IX100.0 : BOOL;
    FLAME_DETECTED AT %IX100.1 : BOOL;
    GAS_SMOKE_ALERT AT %IX100.2 : BOOL;
    GAS_RAW_VALUE AT %IW100 : UINT;
    HEARTBEAT AT %IW101 : UINT;
    RECONNECT_COUNT AT %IW102 : UINT;
  END_VAR

  (* Forward SCADA commands to the ESP32 *)
  ALARM_ACTIVATE := SCADA_ALARM_ACTIVATE;

  (* Flags computed in the PLC *)
  PUB_REMOTE_ONLY := SCADA_ALARM_ACTIVATE AND NOT (MANUAL_PULL_TRIGGERED OR FLAME_DETECTED OR GAS_SMOKE_ALERT);

  (* Publish node status for SCADA *)
  PUB_MANUAL_PULL_TRIGGERED := MANUAL_PULL_TRIGGERED;
  PUB_FLAME_DETECTED := FLAME_DETECTED;
  PUB_GAS_SMOKE_ALERT := GAS_SMOKE_ALERT;
  PUB_GAS_RAW_VALUE := GAS_RAW_VALUE;
  PUB_HEARTBEAT := HEARTBEAT;
  PUB_RECONNECT_COUNT := RECONNECT_COUNT;
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : prog0;
  END_RESOURCE
END_CONFIGURATION
