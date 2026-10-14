# BSH-Perimeter Node

**IP Address:** 192.168.1.15
**Field layer:** Digital (PIR motion) + analog-timed (HC-SR04 ultrasonic) ↔ ESP32
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-BSH-Perimeter` — its own Docker container at 192.168.1.115 (macvlan, running on the hub host 192.168.1.10), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.1.15:502

> **Replaces BSH-AirQuality** as the fifth BSH system (same IP, ports, and
> OpenPLC slot — see Section 14, Lessons Learned & Decisions Log, of
> `Piranga_Workflow_v2.md` for the swap rationale). The MQ-135 sensor and
> its air-quality role are retired from the active build entirely — the
> Fire Alarm flame/smoke/gas upgrade that was expected to possibly reuse it
> went with an MQ-2 sensor instead (see `BSH_Systems/02_BSH-FireAlarm.md`,
> `BSH_Sensors/15_Flame_Sensor.md`, `BSH_Sensors/16_MQ-2_Gas_Smoke_Sensor.md`,
> and the Sep 2026 Lessons Learned entries in `Piranga_Workflow_v2.md`).

## Role
Monitors the campus perimeter/pathway zone for unauthorized presence,
using two independent, complementary sensors: a PIR sensor for broad
motion detection, and an ultrasonic ranger for a second, distance-based
signal covering the same zone. Neither sensor alone is treated as proof
of an intrusion — see "Notes" below.

## Components Used
- PIR Motion Sensor — see `BSH_Sensors/13_PIR_Motion_Sensor.md`
- HC-SR04 Ultrasonic Distance Sensor — see `BSH_Sensors/14_HC-SR04_Ultrasonic_Sensor.md`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 32 | PIR motion sensor OUT |
| GPIO 14 | Ultrasonic TRIG |
| GPIO 27 | Ultrasonic ECHO (**through a 1kΩ/2kΩ voltage divider — 5V to 3.3V, see the sensor doc**) |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | MOTION_DETECTED | Raw PIR output, reported honestly |
| Discrete Input | DI1 | OBJECT_IN_RANGE | Ultrasonic distance below a fixed threshold |
| Input Register | IR0 | DISTANCE_CM | Raw measured distance (sentinel value when no echo is received — never silently reported as 0) |
| Coil | Coil0 | ZONE_DISABLE | Master→slave, level. While set (and only while comms are live — see below), `MOTION_DETECTED`/`OBJECT_IN_RANGE` are forced false; `DISTANCE_CM` keeps reporting real readings regardless, since it's not a security judgment on its own |

## Notes
This node deliberately reports two **raw, independent** signals —
`MOTION_DETECTED` and `OBJECT_IN_RANGE` — rather than fusing them locally
into a single "intrusion" bit. Per Section 7 (Cascade Event Flow) of
`Piranga_Workflow_v2.md`, correlating multiple nodes' signals (including,
here, this node's own two sensors together) is the SCADA/OpenPLC layer's
job, not something a single ESP32 should decide alone — the same design
choice already used on BSH-Door for `IR_DETECTED`/`DOOR_POSITION`/`ACCESS_GRANTED`.

`ZONE_DISABLE` follows the same "remote command only honored while comms
are live" pattern as `ELEVATOR_DISABLE`, with one deliberate difference in
its **fail direction**: if the link to OpenPLC drops, `ZONE_DISABLE` is
**not** trusted and the zone is treated as enabled/reporting regardless of
its last known value. Failing toward "still watching" is the safer default
for a security sensor — the opposite of, say, a fire alarm's actuator
outputs, where failing toward "off" is safer. This asymmetry is worth
calling out explicitly in any presentation of this node's design.

`DISTANCE_CM`'s alert threshold (how close counts as `OBJECT_IN_RANGE`) is
a placeholder pending a real calibration pass against the actual printed
perimeter geometry — same "budget real bench time for this, not the night
before the demo" lesson as the MQ-135's old threshold.

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| PIR motion sensor (GPIO 32) | `wokwi-pir-motion-sensor` | Native |
| HC-SR04 (TRIG 14, ECHO 27) | `wokwi-hc-sr04` | Native — **no divider in sim; the bench still needs the 1kΩ/2kΩ divider** |

Folder: `Firmware/BSH-Perimeter/Wokwi/` — not built yet; created when BSH-Perimeter becomes the active system. What to try: Press "Simulate motion" on the PIR and drag the HC-SR04 distance below `OBJECT_RANGE_THRESHOLD_CM`. Confirms the two signals are reported separately, not fused.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
