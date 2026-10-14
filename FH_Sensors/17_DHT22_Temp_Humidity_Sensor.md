# DHT22 / AM2302 Temperature & Humidity Sensor

> **Bench part changed 2026-10-07:** the build uses a **DHT11** (3-pin module, whole-degree, 0-50 C, +/-2 C) because that is what was in hand. Wiring is identical, no external pull-up. Wokwi stays on the DHT22. Details: `Firmware/FH-HVAC/DHT11_Sensor/Wiring.md`.

**Used in:** FH-HVAC node (Founder's Hall slot 1 — see `FH_Systems/01_FH-HVAC.md`)

> **This is the one sensor sheet with no `BSH_Sensors/` mirror.** Every other
> component doc exists once per building because the component exists in both.
> No BSH system uses a DHT22, because FH-HVAC has no BSH counterpart (FH slot 1
> changed from Elevator to HVAC on Sep 26, 2026). If a BSH system ever adopts
> this part, copy this sheet to `BSH_Sensors/17_...` rather than cross-referencing
> across buildings, per the project's one-location-per-building convention.

## Function
Measures ambient temperature and relative humidity for the Founder's Hall HVAC
zone. Temperature is the value the node's control decision is made against
(compare `TEMP_C` to `TARGET_TEMP_C`, run the compressor/fan relay while the
zone is above setpoint). It is also the project's cleanest false-data-injection
target: a spoofed analog reading is far more realistic for building automation
than flipping a digital bit, and no mirrored system provides one (see
Scenario 11 in `Piranga_Workflow_v2.md`).

## Key Specs
- Single-wire digital protocol, proprietary to the DHT family — **not** I2C and
  **not** Dallas 1-Wire, despite looking like both. The library does the bit-banging.
- Supply 3.3–6V; runs directly off the ESP32's 3.3V rail
- Temperature roughly -40 to +80°C, typically ±0.5°C. Humidity 0–100% RH,
  typically ±2–5%
- **Maximum sample rate about 0.5 Hz** — one reading every 2 seconds. Polling
  faster returns stale or failed reads, it does not return fresher data
- Data line needs a pull-up resistor, 4.7kΩ–10kΩ. Many breakout-board versions
  (3-pin, as opposed to the bare 4-pin sensor) already have one fitted — check
  your specific module before adding a second one
- Reports both values from one read, so `HUMIDITY_PCT` costs nothing extra

## Wiring / Pins (FH-HVAC ESP32)
| Signal | GPIO |
|---|---|
| DATA | GPIO 4 (10kΩ pull-up to 3.3V) |
| VCC | 3.3V |
| GND | Common ground |

Provisional, along with the rest of FH-HVAC's pin map. One constraint is not
negotiable though: because the data line needs a pull-up, it **cannot** live on
GPIO 34–39 — those are input-only and have no internal pull resistors in
silicon, the same trap that caught the Door node's reed switch (see
`Piranga_Workflow_v2.md` Section 14, Sep 10). GPIO 4 is a normal bidirectional
pin, so it is a safe choice.

## Modbus Register Tie-In
- Input Register `TEMP_C` (IR0) — raw temperature reading
- Input Register `HUMIDITY_PCT` (IR1) — raw humidity reading
- Discrete Input `HVAC_FAULT` (DI1) — latched on a failed/timed-out read

Modbus registers are 16-bit integers and this sensor reports tenths. Decide
whole degrees vs. tenths (23.4°C → `234`) before the firmware is written, apply
the same scale to `TARGET_TEMP_C` (HR0), and write the choice into both this
sheet and the system doc — an unmatched scale between a reading and its setpoint
fails silently.

## Notes
**The read is blocking.** A DHT22 exchange bit-bangs for several milliseconds
(up to roughly 25ms on a bad read) with interrupts effectively held off. That is
the same class of problem as the HC-SR04's `pulseIn()` on the retired BSH-Perimeter, and it
gets the same treatment: call it once per sample interval on a `millis()` timer
(2s or slower, which the sensor requires anyway), never once per loop iteration.
Done that way it will not disturb Modbus polling; done wrong it makes the node
look like an unresponsive slave, exactly the failure the Elevator rewrite was
about.

**A failed read is information, not an error to swallow.** The libraries return
NaN when a read fails. That is precisely what should drive `HVAC_FAULT` — and it
must not be reported as `0°C`, the same "never silently report a sentinel as a
real value" rule already applied to `DISTANCE_CM` on the retired BSH-Perimeter.

Give it a few seconds after power-on before trusting the first reading, and keep
it away from the relay-switched fan's airflow and from the ESP32's own regulator,
or the zone temperature it reports will be the enclosure's, not the room's.

Library: Adafruit's "DHT sensor library" (needs "Adafruit Unified Sensor" too),
or `DHT_nonblocking` if the blocking read proves troublesome during bring-up.
Record the exact version installed in `References/INDEX.md`, per that file's rule.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-dht22`

**How to use it:** VCC, GND, SDA/DATA→GPIO 4. Click the part while the
simulation runs to set temperature and humidity directly, which is the easiest
way to exercise both the cooling threshold and the setpoint comparison.

**What the simulation hides:** the 2-second minimum sample interval, the pull-up
resistor, warm-up, self-heating, and failed reads — so the simulation will never
show you an `HVAC_FAULT`. Force that path on the bench by unplugging the data
line while the node runs.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. Unlike every other
system, FH-HVAC's simulation has to be built from scratch rather than copied
from BSH — `Firmware/FH-HVAC/Wokwi/`.
