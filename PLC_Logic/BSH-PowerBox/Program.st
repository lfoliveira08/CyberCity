(* BSH-PowerBox -- OpenPLC Structured Text program (power rails)
   Generated Oct 28, 2026 from Luis's OpenPLC Editor build (project BSH-PowerBox-ESP32/, saved
   18:25 that day): the variable block comes from pous/programs/main.ld and the logic from its
   8 rungs (R1-R8, wiring checked by reading the saved project file: normal contacts and coils,
   MOVE blocks with EN and ENO on the power rails). Wrapped here with CONFIGURATION/RESOURCE so
   the v3 web UI (iec2c) can compile it -- the Editor's own files have no CONFIGURATION block.
   This is the deployable form of the ladder built in the Editor. It replaces the lights-and-fan
   program (archived in Archive/BSH-PowerBox_PLC_lights-and-fan/). Not compiled by the agent.
   Registers: Register_Map.md in this folder (the sketch header in Firmware/BSH-PowerBox wins).
   SCADA_* = bits SCADA writes in this PLC's own Modbus server.
   PUB_*   = values this program publishes for SCADA to read.
   Slave device on the container: BSH-PowerBox-ESP32, Generic Modbus TCP, 192.168.50.14:502,
   slave ID 0, discrete inputs start 0 size 3, coils start 0 size 3, input registers start 0
   size 2, holding registers read and write size 0. Slave block %IX100.0-2, %QX100.0-2,
   %IW100-101 (confirm on the container's Slave Devices page). *)
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

  (* R1 ForwardElevRailCommand -- copy the SCADA Elevator rail command to the ESP32 coil (level copy) *)
  ELEV_RAIL_CMD := SCADA_ELEV_RAIL_CMD;

  (* R2 ForwardFireRailCommand -- same for FireAlarm *)
  FIRE_RAIL_CMD := SCADA_FIRE_RAIL_CMD;

  (* R3 ForwardDoorRailCommand -- same for Door *)
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
