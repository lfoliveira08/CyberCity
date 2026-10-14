# Alternate Design Option — BACnet/MQTT Backhaul with Protocol Bridges

**Status:** Not currently used. The project locked in an all-Modbus, zero-bridge backhaul for all 10 systems on Sep 8, 2026 (see `Piranga_Workflow_v2.md`, Section 14). This document preserves the alternate design in full, in case the team wants to revisit it later — nothing here needs to change if you do, since it slots in alongside the current field-layer choices (Wiegand at Door, DALI at Power Box) without touching them.

## Why This Option Exists

Real building integration commonly uses BACnet/IP for fire alarm and elevator status reporting, and MQTT for modern IoT-style air quality sensors — both are arguably more realistic backhaul choices than Modbus for those specific system types. The reason the project didn't keep them: **OpenPLC's master/polling engine only speaks Modbus TCP or Modbus RTU** — it cannot poll a BACnet device or subscribe to an MQTT topic on its own. Using BACnet or MQTT as the backhaul therefore requires a small translation bridge sitting between the ESP32 and its OpenPLC instance, rather than a direct connection.

This isn't unrealistic — real BACnet/MQTT-to-BMS gateways work exactly this way — but it's an extra piece of software to build, test, and debug per protocol, which is why the simpler direct-Modbus design was chosen for now.

## What Would Change

Only the backhaul for three systems. Door and Power Box stay exactly as currently designed (they were already direct Modbus).

| System | Backhaul (alternate) | Bridge |
|---|---|---|
| Elevator | BACnet/IP, Device Instance 3011 (BSH) / 3021 (FH) | BACnet↔Modbus (Node-RED) |
| Fire Alarm | BACnet/IP, Device Instance 3012 (BSH) / 3022 (FH) | BACnet↔Modbus (Node-RED) |
| Air Quality | MQTT (broker on hub, 192.168.1.10:1883) | MQTT↔Modbus (Node-RED) |

## How the Bridge Pattern Works

Each bridge is a bidirectional Node-RED flow:
- It polls (BACnet) or subscribes to (MQTT) the ESP32's native protocol.
- It writes the resulting values into that system's own dedicated OpenPLC instance over Modbus (OpenPLC's internal Modbus **server**, not its master function).
- For commands going the other direction (e.g., a SCADA-issued `ALARM_ACTIVATE`), the bridge reads the write from OpenPLC's Modbus memory and issues the equivalent BACnet `WriteProperty` or MQTT publish back out to the ESP32.

This keeps the SCADA layer's view of all 10 systems uniform (it always reads/writes Modbus, regardless of what any individual system's OpenPLC-to-ESP32 link actually happens to be) — the bridge is what does the work of hiding the protocol difference from everything above it.

## BACnet Object Map — Elevator

| Object | Instance | Name | Meaning |
|---|---|---|---|
| Binary Input | BI-0 | ELEVATOR_MOVING | Motor currently running |
| Binary Input | BI-1 | ELEVATOR_FAULT | Fault/stall condition |
| Binary Output | BO-0 | CALL_FLOOR_1 | Remote call to floor 1 |
| Binary Output | BO-1 | CALL_FLOOR_2 | Remote call to floor 2 |
| Binary Output | BO-2 | CALL_FLOOR_3 | Remote call to floor 3 |
| Binary Output | BO-3 | ELEVATOR_DISABLE | Lock out the elevator |
| Analog Input | AI-0 | CURRENT_FLOOR | Reported current floor (1–3) |

## BACnet Object Map — Fire Alarm

| Object | Instance | Name | Meaning |
|---|---|---|---|
| Binary Input | BI-0 | MANUAL_PULL_TRIGGERED | Local pull station activated |
| Binary Output | BO-0 | ALARM_ACTIVATE | Remote/hub-triggered alarm |

## MQTT Topics — Air Quality

| Topic | Data Type | Description |
|---|---|---|
| `piranga/bsh/airquality/raw` | Numeric | Raw analog reading (BSH) |
| `piranga/bsh/airquality/alert` | Boolean | Reading above threshold (BSH) |
| `piranga/fh/airquality/raw` | Numeric | Raw analog reading (FH) |
| `piranga/fh/airquality/alert` | Boolean | Reading above threshold (FH) |

## Feasibility Notes (from prior research)

- **BACnet on ESP32:** Real implementations exist (`bacnet-stack-esp32`, Chipkin's `ESP32-BACnetServerExample`), but a couple of community forum threads report rough edges in some ports. Budget real debugging time if this is revisited — this was the main driver of extra project risk in this design.
- **MQTT on ESP32:** Fully native, zero risk — this was already proven working in the original v1 design for Founder's Hall.
- Reviving this option means building and testing 2 Node-RED bridge flows (BACnet↔Modbus, MQTT↔Modbus) that don't currently exist anywhere in the project.

## A Smaller Middle-Ground Option, If Revisited

If the goal is just to add *some* backhaul protocol variety without going all the way back to BACnet/MQTT bridges: OpenPLC can act as a **DNP3 outstation** (it can be polled by an external DNP3 master, though it can't poll a DNP3 field device itself). This means the SCADA-to-OpenPLC link — not the ESP32-to-OpenPLC link — could use DNP3 for one or more instances instead of Modbus, which is a real and common SCADA protocol in critical infrastructure, without touching the ESP32 firmware or requiring any bridge at all. Worth considering as a lower-effort way to reintroduce protocol variety at a different layer than originally planned. Note: OpenPLC's DNP3 outstation support is not available on Windows.
