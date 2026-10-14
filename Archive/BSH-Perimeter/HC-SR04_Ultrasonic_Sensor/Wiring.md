# HC-SR04 Ultrasonic Distance Sensor — BSH-Perimeter

Reference doc: `BSH_Sensors/14_HC-SR04_Ultrasonic_Sensor.md`

## What it does here
Measures distance to the nearest object in the perimeter zone — the
second of this node's two independent sensors. Reported as both a raw
distance (`DISTANCE_CM`) and a simple threshold check (`OBJECT_IN_RANGE`);
neither is treated as an intrusion judgment on its own (see the DESIGN
NOTE in `BSH_Perimeter_Node.ino`).

## *** Read this before wiring — ECHO is not 3.3V-safe ***
The HC-SR04's ECHO pin outputs 5V. Wiring it directly to an ESP32 GPIO
risks damaging the pin over repeated exposure. Build a voltage divider:
**1kΩ resistor from ECHO to the GPIO, 2kΩ resistor from that same GPIO to
GND.** This drops the signal to 5V × 2000/(1000+2000) = 3.33V, safely
within the ESP32's logic-level range. TRIG has no such requirement — it's
an input to the sensor, safe to drive directly from the ESP32 at 3.3V.

## Key specs
- 4-pin module: VCC, TRIG, ECHO, GND
- Measuring range: ~2cm to ~400cm
- TRIG: 3.3V-safe digital output from the ESP32
- ECHO: 5V digital output from the sensor — **needs the divider above**

## Wiring
| Signal | ESP32 pin | Notes |
|---|---|---|
| TRIG | GPIO 14 | Direct connection |
| ECHO | GPIO 27 | **Through the 1kΩ/2kΩ divider — do not wire directly** |
| VCC | 5V | |
| GND | Common ground | Shared with the divider's ground leg |

## Firmware tie-in
`BSH_Perimeter_Node.ino`: `PIN_TRIG = 14`, `PIN_ECHO = 27`. `readDistanceCm()`
pulses TRIG, then reads ECHO with a bounded `pulseIn()` (25ms timeout — see
the BOUNDED-BLOCKING NOTE at the top of that file). A timed-out or
out-of-range reading returns `NO_ECHO_SENTINEL_CM` (9999), never a bare 0
— 0 would look like "something is touching the sensor," which is not what
a missing echo means. Sampled once every 150ms, not every loop.

## Bench-test checklist (Phase A, this component only)
- [ ] **Voltage divider built and verified with a multimeter** (measure
      the actual voltage at the GPIO side before connecting the ESP32 —
      confirm ~3.3V, not 5V) — this is the step most likely to be skipped
- [ ] Wired per the table above
- [ ] Flash the real firmware, watch `DISTANCE_CM` over Serial or a Modbus
      client
- [ ] Hold an object at a few known distances and confirm the reported
      value is reasonably close
- [ ] Move an object out of range (or point the sensor at nothing solid)
      and confirm `DISTANCE_CM` reports `9999`, not `0` or a garbage value
- [ ] Confirm `OBJECT_IN_RANGE` flips at roughly `OBJECT_RANGE_THRESHOLD_CM`
      (50cm placeholder) — recalibrate that constant once the real
      perimeter geometry is printed and assembled
- [ ] Watch for any Modbus timeout/slave-not-responding symptoms once this
      is polled by OpenPLC — the bounded `pulseIn()` call is a new pattern
      for this project, not yet proven on real hardware

## Lessons Learned
*(fill in as you go — especially anything about the pulseIn() timing in
practice, since that's flagged as an open question in the firmware)*

## Troubleshooting Tips
*(fill in as you go — e.g. "readings jump around wildly" -> check divider
wiring and that nothing is vibrating the sensor mount; "always reads 9999"
-> check TRIG/ECHO aren't swapped, or that ECHO's divider isn't wired
backwards)*
