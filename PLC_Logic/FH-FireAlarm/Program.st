(* FH-FireAlarm -- OpenPLC Structured Text program
   Generated Oct 21, 2026 from Luis's OpenPLC Editor build (project FH-FireAlarm-ESP32/):
   the variable block comes from pous/programs/main.ld and the logic from its 8 rungs
   (R1-R8, wiring checked against the saved project). Wrapped here with
   CONFIGURATION/RESOURCE so the v3 web UI (iec2c) can compile it -- the Editor's
   own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor; it replaces the
   Oct 11 hand-draft (same logic as BSH-FireAlarm: plain level copy for the command).
   Registers source of truth: FH_Systems/02_FH-FireAlarm.md (folder wins over any copy).
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave-block addresses (%IX100.x, %QX100.x, %IW100+) follow the convention verified
   on the BSH nodes; confirm them on this container's Slave Devices page. *)
PROGRAM main
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

  (* R1 ForwardAlarmCommand -- copy the SCADA alarm command to the ESP32 coil (level copy) *)
  ALARM_ACTIVATE := SCADA_ALARM_ACTIVATE;

  (* R2 RemoteOnlyFlag -- alarm commanded by SCADA with no physical cause *)
  PUB_REMOTE_ONLY := SCADA_ALARM_ACTIVATE AND NOT(MANUAL_PULL_TRIGGERED) AND NOT(FLAME_DETECTED) AND NOT(GAS_SMOKE_ALERT);

  (* R3-R5 publish the ESP32 discrete inputs for SCADA *)
  PUB_MANUAL_PULL_TRIGGERED := MANUAL_PULL_TRIGGERED;
  PUB_FLAME_DETECTED := FLAME_DETECTED;
  PUB_GAS_SMOKE_ALERT := GAS_SMOKE_ALERT;

  (* R6-R8 publish the ESP32 input registers for SCADA (MOVE copies the 16-bit value) *)
  PUB_GAS_RAW_VALUE := MOVE(GAS_RAW_VALUE);
  PUB_HEARTBEAT := MOVE(HEARTBEAT);
  PUB_RECONNECT_COUNT := MOVE(RECONNECT_COUNT);
END_PROGRAM

CONFIGURATION Config0
  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms, PRIORITY := 0);
    PROGRAM instance0 WITH task0 : main;
  END_RESOURCE
END_CONFIGURATION
