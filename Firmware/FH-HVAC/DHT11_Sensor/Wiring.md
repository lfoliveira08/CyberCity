# DHT11 Temperature/Humidity Sensor -- FH-HVAC

Reference doc: `FH_Sensors/17_DHT22_Temp_Humidity_Sensor.md` (written for the DHT22; the
design part. **The bench part is a DHT11, 3-pin module, decided 2026-10-07** -- it is what was in hand.)

## What it does here
Provides `TEMP_C` (IR0, tenths of a degree) and `HUMIDITY_PCT` (IR1, whole percent).
Temperature is the value the cooling decision is made against.

## DHT11 vs the DHT22 the docs describe
| | DHT11 (bench) | DHT22 (Wokwi, design docs) |
|---|---|---|
| Temperature | 0-50 C, whole degrees, +/-2 C | -40-80 C, 0.1 C, +/-0.5 C |
| Humidity | 20-80 %, +/-5 % | 0-100 %, +/-2-5 % |
| Max sample rate | 1 Hz | 0.5 Hz |

The firmware keeps tenths scaling, so a DHT11 simply reports multiples of 10 (24.0, 25.0, never
24.3). With the +/-0.5 C hysteresis that means cooling starts a full degree above the setpoint and
stops a full degree below. Changing back to a DHT22 later is the one `DHT_MODEL` define in the sketch.
Firmware selects the model itself: `DHT11` on hardware, `DHT22` in Wokwi.

## Wiring (3-pin module)
| Module pin | ESP32 | Notes |
|---|---|---|
| VCC / + | 3.3V | Module accepts 3.3-5.5V; 3.3V keeps data levels safe for the ESP32 |
| DATA / OUT / S | GPIO 4 | **No external pull-up** -- the 3-pin module already has one fitted. Adding one is harmless but unnecessary |
| GND / - | GND | |

Pin order differs between module brands: read the silkscreen, do not assume. GPIO 4 is a normal
bidirectional pin. Never move DATA to GPIO 34-39 (input-only).

## Hazards / gotchas
- Single-wire protocol is DHT-proprietary, not I2C and not Dallas 1-Wire.
- The firmware samples every 2000 ms on a `millis()` timer (sensor allows 1 s; 2 s is deliberately conservative).
- A read blocks for several ms. Never call it every loop; never from a Modbus callback.
- A failed read returns NaN. The firmware treats NaN as a failure, never as 0 C. The firmware also
  rejects readings outside 0-50 C.
- Mount away from the motor/fan airflow and the ESP32's regulator, or it reports the enclosure's temperature.
- Wait ~2 s after power-on before trusting the first reading.
- Heating to move the reading: a DHT11 needs a whole degree of change, so warm it with a hand or
  hair dryer on low for several seconds.

## Standalone test checklist (BenchTest.ino)
- [ ] Serial prints plausible room values (about 18-30 C, 20-70 %), whole degrees, not 0 or nan
- [ ] Warming it with a hand raises the reading by at least 1 C within ~10 s
- [ ] Pulling the DATA wire prints `READ FAILED`, not a number
- [ ] Reinserting it recovers without a reset
