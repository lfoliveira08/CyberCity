# BSH-BackupPower -- Wokwi Simulation (Phase A0)

A simulation of the whole BSH-BackupPower (ATS Monitor) system: same GPIO pins
as `BSH_Systems/05_BSH-BackupPower.md`, no WiFi, no Modbus. Use it to learn what
each of the four signals does and how they stay independent, before the real
bench build.

Rule: `Firmware/WORKFLOW.md` Section 7. Template: `../../BSH-FireAlarm/Wokwi/`.

**Status:** Phase A0 **done** (Oct 5, 2026). Every scenario below behaved as
described, including the relay and green demo-load LED responding to `1`/`0`.
Logged in each component's `Build_Log.md`.

## Files

| File | What it is |
|---|---|
| `diagram.json` | Parts and every wire |
| `sketch.ino` | Standalone test logic. Same pin map as the future node firmware |
| `README.md` | This guide |

No `libraries.txt` -- the sketch uses only the ESP32 core.

## How to open it

1. wokwi.com -> *New Project* -> **ESP32**.
2. Replace the `sketch.ino` tab with this folder's `sketch.ino`.
3. Replace the `diagram.json` tab with this folder's `diagram.json`.
4. Press **Play**. The Serial Monitor opens underneath.

## What's in the circuit

| GPIO | Part in Wokwi | Real part | Notes |
|---|---|---|---|
| 26 | Slide switch | SPST toggle switch | One terminal to GPIO 26, other to GND, `INPUT_PULLUP`. Slide **LEFT = utility power lost** (pin LOW), **RIGHT = normal** |
| 27 | Relay module -> green LED + 220R ("Demo load") | Relay module -> demo load | Relay and LED share the ESP32's 5V in the sim for simplicity |
| 34 | **Potentiometer (stand-in)** | Battery voltage divider | Wiper to GPIO 34 (ADC1) |
| 35 | **Potentiometer (stand-in)** | ACS712 OUT | Wiper to GPIO 35 (ADC1). Mid-point = zero current |

## Driving ATS_TRANSFER_CMD (simulation only)

On the real node this is Modbus Coil0, written by the master. There is no
network here, so type into the Serial Monitor and press Enter:

| Type | Effect |
|---|---|
| `1` | Coil0 = 1, transfer commanded, relay on, load LED lit |
| `0` | Coil0 = 0, relay off |
| `t` | Toggle |

## Scenarios

**Both potentiometers start at their minimum in Wokwi**, so at boot you will see
the battery at 0% (`LOW_BATTERY=YES`) and the current clamped at 5000 mA (the
current pot's zero point is mid-scale). That is correct. Click each pot and
set the **battery** pot to about 70% and the **current** pot to the middle for
the normal state:

```
DI0 UTILITY_POWER_LOST=no  DI1 LOW_BATTERY=no  IR0 UPS_BATTERY_PCT=69  IR1 LOAD_CURRENT_MA=0  Coil0 ATS_TRANSFER_CMD=0 (relay off)
```

| # | Do this | You should see |
|---|---|---|
| 1 | Start, then set the battery pot to ~70% and the current pot to mid-scale | Load LED off, utility `no`, `LOW_BATTERY=no`, current ~0 mA. (At boot: battery 0%, current 5000 mA, see above) |
| 2 | Slide the switch LEFT | `UTILITY_POWER_LOST=YES`. Relay and LED do **not** change: the node never transfers on its own |
| 3 | Slide it back RIGHT | `no` again within ~50 ms |
| 4 | Type `1` + Enter | Relay clicks on, LED lit, `Coil0=1` |
| 5 | Type `0` + Enter | Relay off, LED dark |
| 6 | Turn the **battery** pot down | `UPS_BATTERY_PCT` falls. `LOW_BATTERY` flips to `YES` the moment it drops below 20 |
| 7 | Keep turning it to 0 | `UPS_BATTERY_PCT` keeps reporting the real value, even while `LOW_BATTERY=YES` |
| 8 | Turn the **current** pot either way from mid | `LOAD_CURRENT_MA` rises from 0 (absolute value, clamped at 5000) |
| 9 | Command `1` with the switch RIGHT (no outage) | Relay on while `UTILITY_POWER_LOST=no`. The node says nothing about it; this is the "forced failover test with no real outage" pattern SCADA must flag later |

Things to notice:
- **Four independent signals.** Nothing is fused locally. Every combination is
  reportable, including the suspicious ones.
- **Raw plus derived.** `LOW_BATTERY` is a threshold on `UPS_BATTERY_PCT`, and
  the raw percentage keeps reporting regardless.
- **Non-blocking.** No `delay()` in the loop, ready for `mb.task()` in Phase B.

## Placeholders in the sketch

| Constant | Value | Why it is a placeholder |
|---|---|---|
| `LOW_BATTERY_PCT` | 20 | Needs real calibration against the actual pack and divider |
| `UPS_BATTERY_PCT` scaling | raw * 100 / 4095 | Real mapping depends on R1/R2 and the pack's empty/full voltage |
| `CURRENT_ZERO_RAW` | 2048 | Real zero-current baseline must be measured on the bench |
| `MA_PER_COUNT` | 4.36 | Assumes a 5A ACS712 (185 mV/A) with no divider; check the actual variant |

## Real bench differences

- **Relay polarity:** Wokwi's relay energizes on HIGH (`RELAY_ACTIVE_LOW=false`).
  Real modules vary and most cheap ones are active-LOW. Check your module
  fresh (see `BSH_Sensors/09_Relay_Module.md`). On the bench, the relay
  module's own IN pull-up may also matter if the pin is a strapping pin
  (GPIO 27 is not).
- **Battery divider:** the pot teaches only the threshold logic. Real R1/R2
  must be sized so the full-charge voltage stays under 3.3V, and checked with a
  multimeter on the battery and the ADC tap together. There is no real
  discharge curve here.
- **ACS712 output swing:** a 5V-supplied board centers near 2.5V and can swing
  near or above 3.3V under load. **Measure OUT before wiring to GPIO 35**, and
  add a divider if needed. Wokwi never warns about voltage. Never put it in
  series with mains.
- **ACS712 noise and offset:** the real zero-current reading drifts and is
  noisy; the sim's is perfectly stable. Expect to average samples.
- **Relay power:** the sim draws coil and LED from the same 5V pin. On the
  bench, the demo load should be on its own supply, separated from the ESP32.
- **Comm-loss fail-safe:** `ATS_TRANSFER_CMD` defaulting to "not commanded"
  when comms drop is a Phase B watchdog behavior and cannot be shown here.
- **Switch bounce:** the sim switch is clean; a real toggle bounces. The 50 ms
  debounce is there for it.

## Troubleshooting

- **Relay/LED is inverted:** flip `RELAY_ACTIVE_LOW` in `sketch.ino`.
- **Battery reads 0 or 100 and won't move:** make sure the pot's `SIG` goes to
  GPIO 34 and VCC to 3V3 (not 5V).
- **Typing in the Serial Monitor does nothing:** press Enter after the
  character, and make sure the simulation is running.
