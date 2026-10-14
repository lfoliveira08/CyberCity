# PIR Motion Sensor (HC-SR501 or equivalent)

**Used in:** BSH-Perimeter node (replaces BSH-AirQuality in the project's
system lineup — see `BSH_Systems/05_BSH-Perimeter.md`)

## Function
Detects motion anywhere in the sensor's field of view along the campus
perimeter/pathway zone — the first, cheapest layer of intrusion detection
for this node. Reports raw, immediate motion state; it does not by itself
decide "this is an intrusion" (see the DESIGN NOTE in
`Firmware/BSH_Perimeter_Node.ino`).

## Key Specs
- Passive IR motion sensor (HC-SR501-style 3-pin module)
- 5V powered, digital output (module regulates its own PIR bias internally
  — the OUT pin is a clean 3.3V/5V-logic digital HIGH/LOW, not raw analog,
  so no voltage divider is needed here, unlike the ultrasonic sensor)
- Most common modules have two onboard trimmer potentiometers (sensitivity
  and retrigger/hold time) and a jumper (single-trigger vs. repeatable
  trigger) — set the jumper to **repeatable trigger (H)** so continued
  motion keeps `MOTION_DETECTED` held true rather than pulsing once
- Needs a brief settle time after power-on (typically 30-60s) before
  readings stabilize — the module's internal calibration, separate from
  any warm-up logic in firmware

## Wiring / Pins (BSH-Perimeter ESP32)
| Signal | GPIO |
|---|---|
| OUT | GPIO 32 |
| VCC | 5V |
| GND | Common ground |

## Modbus Register Tie-In
Feeds Discrete Input `MOTION_DETECTED` directly — reported raw and
honestly, every loop iteration, with no local smoothing or judgment logic
(unlike the ultrasonic sensor's distance reading, a PIR output doesn't
benefit from averaging; it's already a clean digital trigger).

## Notes
Mount away from HVAC vents, direct sun through windows, and moving
foliage if this is ever run outdoors/near a window — all classic PIR
false-trigger sources. Tune the sensitivity trimmer during bench-test,
not the night before the demo, same lesson learned from the MQ-135's
calibration note on the (now-retired) Air Quality node.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-pir-motion-sensor`

**How to use it:** VCC, GND, OUT→GPIO 32. Click the sensor while the simulation runs and press "Simulate motion"; the OUT pin goes HIGH for the configured delay time.

**What the simulation hides:** Real PIRs need 30–60 s of warm-up, have sensitivity/delay potentiometers and a repeat-trigger jumper, and can false-trigger on heat. The simulation is instant and clean.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
