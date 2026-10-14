# HC-SR04 Ultrasonic Distance Sensor

**Used in:** FH-Perimeter node (Founder's Hall mirror of BSH-Perimeter, which replaces FH-AirQuality in the project's system lineup — see `FH_Systems/05_FH-Perimeter.md`)

## Function
Measures distance to the nearest object in front of the sensor along the perimeter zone near Founder's Hall, giving a second, independent signal from the PIR sensor — "something is close" (a fixed range threshold) as well as the raw distance itself, so the dashboard/SCADA layer can see more than a bare yes/no.

## Key Specs
- HC-SR04 4-pin module: VCC, TRIG, ECHO, GND
- TRIG is a 3.3V-safe digital input (ESP32 can drive it directly)
- **ECHO outputs 5V — NOT 3.3V-safe on its own.** A direct connection to an ESP32 GPIO risks damaging the pin over repeated exposure. Use a voltage divider: 1kΩ resistor from ECHO to the GPIO, 2kΩ resistor from that same GPIO to GND. Output = 5V × 2000/(1000+2000) = 3.33V — same as BSH-Perimeter.
- Measuring range: roughly 2cm to 400cm; readings outside that range (or a missing echo) should be treated as "no valid reading," not as 0

## Wiring / Pins (FH-Perimeter ESP32)
| Signal | GPIO | Notes |
|---|---|---|
| TRIG | GPIO 14 | Direct connection, no divider needed |
| ECHO | GPIO 27 | **Through the 1kΩ/2kΩ voltage divider above — do not wire directly** |
| VCC | 5V | |
| GND | Common ground | Shared with the divider's ground leg |

## Modbus Register Tie-In
- Input Register `DISTANCE_CM` — raw measured distance, reported honestly (a sentinel value, not 0, is used for "no echo received" — see the firmware comments in `FH_Perimeter_Node.ino`)
- Discrete Input `OBJECT_IN_RANGE` — set locally once the distance drops below a chosen threshold; this is a simple range check, not an intrusion judgment (see the DESIGN NOTE in the firmware)

## Notes
`pulseIn()` is the standard way to read ECHO, but it blocks the CPU until the echo returns or a timeout expires. As with BSH-Perimeter, `FH_Perimeter_Node.ino` bounds this to a short, explicit timeout (tens of milliseconds, not seconds) and only issues it on a spaced-out sample interval, not every loop. Budget real bench time to confirm that holds up once this node is on real hardware.

This doc mirrors `BSH_Sensors/14_HC-SR04_Ultrasonic_Sensor.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-hc-sr04`

**How to use it:** VCC, GND, TRIG→GPIO 14, ECHO→GPIO 27. Click it while running and drag the distance slider (2–400 cm) to test the `OBJECT_RANGE_THRESHOLD_CM` logic.

**What the simulation hides:** **The 1kΩ/2kΩ voltage divider on ECHO is still mandatory on the bench.** Wokwi doesn't model the 5V ECHO signal, so the simulation wires it straight through and will never warn you.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
