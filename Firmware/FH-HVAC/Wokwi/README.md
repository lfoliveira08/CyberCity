# FH-HVAC -- Wokwi Simulation (Phase A0)

Simulation of the whole FH-HVAC node: same GPIO pins and the same logic as
`../FH_HVAC_Node/FH_HVAC_Node.ino` (the only difference between the two sketches is
the `WOKWI_SIM` define on one line). All three parts are native Wokwi parts, so unlike
every other system there are no stand-ins. This one is built from scratch because FH
slot 1 has no BSH counterpart (`Firmware/WORKFLOW.md` Section 7).

> Status 2026-10-07: run in Wokwi (idle, cooling and enable behaviours seen). This
> simulation is Phase A logic only (no WiFi/Modbus); Wokwi uses a DHT22 because it
> has no DHT11 part. The real node sketch is `FH_HVAC_Node/FH_HVAC_Node.ino`.

## Files
| File | What it is |
|---|---|
| `diagram.json` | Parts and every wire |
| `sketch.ino` | Firmware, `WOKWI_SIM 1` |
| `libraries.txt` | `DHT sensor library`, `Adafruit Unified Sensor` |
| `README.md` | This guide |

## How to open it
1. wokwi.com -> *New Project* -> **ESP32**.
2. Replace the `sketch.ino` tab with this folder's `sketch.ino`, and `diagram.json` with this folder's.
3. Add the two libraries from `libraries.txt` with the Library Manager.
4. Press **Play**. Serial Monitor shows one status line every 2 seconds.

## What's in the circuit
| GPIO | Wokwi part | Real part |
|---|---|---|
| 4 | `wokwi-dht22` DATA | DHT22 (real bench adds a 10k pull-up unless the module has one) |
| 26 | `wokwi-relay-module` IN | 1-channel relay; cyan LED on NO/COM stands in for the fan |
| 16 / 17 / 18 | `wokwi-rgb-led` (common cathode) via 3x 220 ohm | RGB status LED |

Scaling: temperatures are integer tenths of a degree internally (23.4 C = 234) and
printed as degrees. Default setpoint 24.0 C, hysteresis +/-0.5 C.

## Scenarios
LED colours: **green** idle, **blue** cooling, **yellow** disabled, **red** fault.

| # | Do this | You should see |
|---|---|---|
| 1 | Start (DHT22 defaults to 24 C) | Green, fan LED off, `cooling=0` |
| 2 | Click the DHT22, set temperature to 26 C | Within 2 s: blue, relay clicks, fan LED on, `cooling=1` |
| 3 | Set 24.2 C (inside the band) | Stays as it was (hysteresis): still cooling if it was cooling |
| 4 | Set 23 C | Green, relay opens, fan LED off |
| 5 | With temp at 26 C and cooling, type `+` Enter until the setpoint reads 27.0 C | Cooling stops (next sample): the comparison follows the setpoint, not a constant. `-` brings it back |
| 6 | Heat to 28 C so it is cooling, then type `e` | Yellow, relay opens immediately-ish (next sample), `enable=0`, regardless of temperature. `e` again re-enables |
| 7 | Type `f` | Reads now fail. `fails=1,2`, then at 3 (6 s): red, relay open, `HVAC_FAULT latched` and the printout shows `T=---`, never `0.0`. Type `f` again: sensor "recovers" but the fault stays latched until Stop/Play |

## Real bench differences
- **Sensor model.** The bench part is a **DHT11** (whole degrees, 0-50 C); Wokwi only has the DHT22, so the simulation uses that. Logic is identical; the real sketch picks `DHT11` automatically.
- **Fan.** The bench fan is a plain brushed DC motor on its own supply with a flyback diode (see `../Relay_Module/Wiring.md`).
- **No fault in real life from this sim.** `f` is a simulation-only hook. On the bench,
  fault is proven by unplugging the DHT22 data line (Checklist Phase A step 5).
- **Relay polarity.** Wokwi's relay energizes on HIGH (sketch sets `RELAY_ACTIVE_LOW=false`
  here). The real board was bench-tested on 2026-10-07 and is also active-HIGH, so both sketches now use `false`.
  Still re-test if the relay board is ever swapped (lesson from BSH-PowerBox).
- **DHT22 hides:** the 2 s minimum interval, pull-up resistor, warm-up, self-heating, and
  failed reads. The sketch honours 2 s anyway.
- **Fan supply.** Here the "fan" LED is powered from the ESP32 3V3. On the bench the load
  is on its own supply with common ground.
- **Latched fault** clears only on reset: Stop/Play here, reset button on the bench.

## Troubleshooting
- Build error `DHT.h not found`: libraries not added (step 3).
- LED stays dark: resistor wired `1`/`2` the wrong way round to the GPIO, or RGB `COM` not on GND.
- Relay seems inverted: confirm `WOKWI_SIM` is `1` in `sketch.ino`.
