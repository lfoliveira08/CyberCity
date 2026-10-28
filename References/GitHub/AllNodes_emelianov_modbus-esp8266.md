# emelianov / modbus-esp8266 (`ModbusIP_ESP8266`)

- **Link:** https://github.com/emelianov/modbus-esp8266
- **System:** all 8 nodes (Phase B backhaul)
- **Status:** **Adopted** — this is the library every node's Modbus TCP slave layer is built on
- **License:** not yet recorded — check it before copying any of its code into ours (using it as an installed library is a separate question from copying source)
- **Version installed:** not yet recorded. Record it here the first time Phase B is brought up, per `References/INDEX.md`'s rule
- **Checked:** not independently verified. This note is written from the project's own records (`References/INDEX.md`, `Piranga_Workflow_v2.md` Section 9) — confirm the API against the version you actually install

## What it is

An Arduino-framework Modbus library for ESP8266/ESP32 that can act as a Modbus
TCP **slave** (server), which is exactly the role every node in this project
plays: each ESP32 is polled by its own dedicated OpenPLC instance acting as
master.

## What we use it for

One uniform pattern across all 8 nodes — the project deliberately has no
protocol diversity at the backhaul (see `Piranga_Workflow_v2.md` Section 5), so
this single library covers every node:

- Discrete Inputs for honest sensor state (`FLAME_DETECTED`, `IR_DETECTED`,
  `MOTION_DETECTED`, `COMPRESSOR_RUNNING`, …)
- Input Registers for raw analog values (`GAS_RAW_VALUE`, `DISTANCE_CM`,
  `CURRENT_FLOOR`, `TEMP_C`)
- Coils for master-written commands (`ALARM_ACTIVATE`, `ELEVATOR_DISABLE`,
  `ZONE_DISABLE`, `DOOR_OPEN_CMD`, `HVAC_ENABLE`)
- Holding Registers for master-set analog values — so far only
  `LIGHTS_DIM_LEVEL` and FH-HVAC's `TARGET_TEMP_C`

## Things to watch

- Its task function has to be called every loop iteration. That is the single
  reason this project bans blocking code: the Elevator prototype's `delay()`
  calls, the HC-SR04's `pulseIn()`, and the DHT22's bit-banged read are all
  judged against "does this starve the Modbus task" (see the Sep 10 and Sep 14
  entries in `Piranga_Workflow_v2.md` Section 14).
- Standard Modbus TCP has no authentication at all. That is not a library flaw —
  it is the teaching point behind Scenarios 4, 6, 7 and 11.
