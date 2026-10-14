# HC-SR04 Ultrasonic Distance Sensor

**Used in:** BSH-Perimeter node (replaces BSH-AirQuality in the project's
system lineup — see `BSH_Systems/05_BSH-Perimeter.md`)

## Function
Measures distance to the nearest object in front of the sensor along the
perimeter zone, giving a second, independent signal from the PIR sensor —
"something is close" (a fixed range threshold) as well as the raw distance
itself, so the dashboard/SCADA layer can see more than a bare yes/no.

## Key Specs
- HC-SR04 4-pin module: VCC, TRIG, ECHO, GND
- TRIG is a 3.3V-safe digital input (ESP32 can drive it directly)
- **ECHO outputs 5V — NOT 3.3V-safe on its own.** A direct connection to
  an ESP32 GPIO risks damaging the pin over repeated exposure. Use a
  voltage divider: 1kΩ resistor from ECHO to the GPIO, 2kΩ resistor from
  that same GPIO to GND. Output = 5V × 2000/(1000+2000) = 3.33V — the same
  math and the same precedent as the ultrasonic "Radar" reference
  implementation used elsewhere in this project's supporting materials.
- Measuring range: roughly 2cm to 400cm; readings outside that range (or a
  missing echo) should be treated as "no valid reading," not as 0

## Wiring / Pins (BSH-Perimeter ESP32)
| Signal | GPIO | Notes |
|---|---|---|
| TRIG | GPIO 14 | Direct connection, no divider needed |
| ECHO | GPIO 27 | **Through the 1kΩ/2kΩ voltage divider above — do not wire directly** |
| VCC | 5V | |
| GND | Common ground | Shared with the divider's ground leg |

## Modbus Register Tie-In
- Input Register `DISTANCE_CM` — raw measured distance, reported honestly
  (a sentinel value, not 0, is used for "no echo received" — see the
  firmware comments in `BSH_Perimeter_Node.ino`)
- Discrete Input `OBJECT_IN_RANGE` — set locally once the distance drops
  below a chosen threshold; this is a simple range check, not an
  intrusion judgment (see the DESIGN NOTE in the firmware)

## Notes
`pulseIn()` is the standard way to read ECHO, but it blocks the CPU until
the echo returns or a timeout expires. Unlike the multi-second `delay()`
calls that forced the Elevator node's rewrite (see Section 14 of
`Piranga_Workflow_v2.md`), a `pulseIn()` call here is given a short,
explicit timeout (tens of milliseconds, not seconds) and is only issued
on a spaced-out sample interval, not every loop — this keeps `mb.task()`
responsive enough for Modbus TCP polling in practice. Budget real bench
time to confirm that holds up once this node is on real hardware next to
the other nine; if it ever proves to be a problem, an interrupt-driven
echo measurement is the non-blocking alternative worth trying instead.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-hc-sr04`

**How to use it:** VCC, GND, TRIG→GPIO 14, ECHO→GPIO 27. Click it while running and drag the distance slider (2–400 cm) to test the `OBJECT_RANGE_THRESHOLD_CM` logic.

**What the simulation hides:** **The 1kΩ/2kΩ voltage divider on ECHO is still mandatory on the bench.** Wokwi doesn't model the 5V ECHO signal, so the simulation wires it straight through and will never warn you.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
