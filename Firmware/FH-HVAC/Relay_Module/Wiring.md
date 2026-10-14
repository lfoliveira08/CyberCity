# Relay Module (compressor/fan simulation) -- FH-HVAC

Reference doc: `FH_Sensors/09_Relay_Module.md`

## What it does here
One 1-channel relay switches the "compressor/fan" load while the zone is above setpoint.
Its state is reported honestly as `COMPRESSOR_RUNNING` (DI0 in Phase B).

## Wiring
| Signal | ESP32 | Notes |
|---|---|---|
| Relay IN | GPIO 26 | Not a strapping pin |
| Relay VCC | 5V (VIN) | Most modules want 5V for the coil (~70 mA) |
| Relay GND | GND | Common ground |
| COM / NO | the load | Fan on its **own supply**, grounds common with the ESP32. Never power a motor from the ESP32 3V3 |

The IN pin has an onboard pull-up on many modules. GPIO 26 is safe for that; GPIO 12 is not
(strapping pin -- see BSH-PowerBox Lights_Relay Build_Log).

## Fan (decided 2026-10-07: the build includes a real DC fan)
The fan is the relay's load. It is switched fully on or off, no speed or direction control
(no L293D on this node).

```
 fan supply (+) ---- COM  [relay]  NO ---- fan (+)
 fan (-) ---------------------------------- fan supply (-) ---- ESP32 GND (common ground)
```
- Fan supply (see "Power source" below for the current choice): its own source, matched to the fan's rating (e.g. 5V or 12V fan with a matching
  adapter/battery pack). **Not** the ESP32's 3V3, and not its 5V pin unless the fan draws under
  ~100 mA. A fan's startup surge can reset the ESP32.
- Ground of the fan supply tied to the ESP32 GND.
- Brushed DC motor/fan: put a diode across the fan terminals, cathode to the (+) side
  (e.g. 1N4007 or 1N4148), to absorb the spike when the relay opens. A computer-style 3-pin
  fan with built-in electronics does not need it.
- Check the relay contact rating covers the fan's current (typical module: 10 A, far above any small fan).
- Keep the DHT11 out of the fan's airflow and away from the ESP32's regulator.

Fan used: plain brushed DC motor with a fan blade (2-wire) -- **flyback diode required**.
Rated voltage ______  current ______ (fill in from the motor's label).  Supply: ESP32 board 5V pin (see "Power source").
Flyback diode: **1N4007** (confirmed from the part marking 2026-10-07; 1000 V / 1 A, plenty for this motor). Stripe (cathode) toward the motor lead on the relay NO side.

### Power source (decided 2026-10-07): the ESP32 board's 5V pin
The motor is a small toy motor and no separate supply is on hand, so it runs from the ESP32
board's own 5V (VIN/VCC) pin through the relay: 5V pin -> COM, NO -> motor, motor other lead -> GND.
This is acceptable for a light fan blade but is the riskier option:
- Power the board from a good USB port or charger (1 A or more), not a weak hub.
- The **flyback diode is still required**.
- If the board resets or the serial output restarts whenever the fan switches on, the motor
  is pulling too much: move it to its own supply (2-3 AA cells, grounds joined) and update this note.
- Do not use the board's DC jack as the motor supply.

## Polarity -- MUST be tested, not assumed
`FH_HVAC_Node.ino` has `RELAY_ACTIVE_LOW`, now `false` (was an untested `true`). Other relay boards in this
project turned out active-HIGH (BSH/FH-PowerBox), but each physical board is its own test.
Record the result here: **polarity of this board: active-HIGH (relay energizes on HIGH), bench-confirmed 2026-10-07** with the full node sketch (it behaved reversed under the active-LOW default).

## Standalone test checklist (BenchTest.ino)
- [ ] Relay clicks on and off every 2 s, onboard LED follows
- [ ] Polarity written above and `RELAY_ACTIVE_LOW` set to match
- [ ] On boot the relay is OFF before the first command (no glitch on reset)
- [ ] Fan runs from the board's 5V pin; ESP32 does not brown out or reset when it switches (if it does, add a battery pack)
- [ ] Diode fitted across the fan if it is a plain brushed motor
