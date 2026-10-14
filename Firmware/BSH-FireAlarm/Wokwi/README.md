# BSH-FireAlarm — Wokwi Simulation (Phase A0)

A working simulation of the whole BSH-FireAlarm system: same GPIO pins, same
logic as the real bench sketch (`../BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`).
Use it to learn how each sensor behaves, how everything is wired, and how the
alarm logic reacts, all without touching real hardware.

Rule: `Firmware/WORKFLOW.md` Section 7. This was the first system to get a
simulation, and it serves as the template for the other nine.

## Files

| File | What it is |
|---|---|
| `diagram.json` | The parts and every wire. This is the wiring diagram, and Wokwi draws it for you |
| `sketch.ino` | The firmware. Identical logic to the bench sketch; the only change is the buzzer (see below) |
| `README.md` | This guide |

No `libraries.txt` needed, because the sketch uses only the ESP32 core.

## How to open it

1. Go to **wokwi.com** → *Start from scratch* (or *New Project*) → **ESP32**.
2. In the `sketch.ino` tab, delete everything and paste this folder's `sketch.ino`.
3. In the `diagram.json` tab, delete everything and paste this folder's `diagram.json`.
4. Press the green **▶ Play** button. The Serial Monitor opens underneath.
5. Optional: sign in and **Save** so the project gets its own link the team can reuse.

## What's in the circuit

| GPIO | Part in Wokwi | Real part | Notes |
|---|---|---|---|
| 23 | Buzzer | Active buzzer | Sim uses `tone()`, see below |
| 25 / 26 / 27 | RGB LED (common cathode) + 3× 220Ω | RGB LED + 3× 220Ω | Same wiring as the bench |
| 4 | Red push button "Pull station" + 10kΩ pull-up to 3V3 | Keyestudio push-button module | The 10kΩ plays the role of the resistor built into the real module |
| 18 | **Slide switch (stand-in)** | Flame sensor D0 | Wokwi has no flame sensor. Slide **LEFT = flame** (pin goes LOW), **RIGHT = clear** |
| 34 | Gas sensor (MQ-2 model) | MQ-2 / MQ-9 AO | DO pin left unused, same as the bench |

## Scenarios to try

Start with nothing triggered: the LED should be **steady green**, the buzzer
silent, and the Serial Monitor should print a line like this once a second:

```
button=released  flame=clear  gasRaw=812 (baseline=810, alert>1210 ok)  ==> alarmActive=false (green)
```

| # | Do this | You should see |
|---|---|---|
| 1 | Nothing (normal operation) | Green LED, silent, `alarmActive=false` |
| 2 | **Click and hold** the red *Pull station* button (Ctrl-click / Cmd-click keeps it pressed) | Red LED blinks, buzzer sounds, `button=PRESSED` |
| 3 | Release it | Back to green within ~50 ms (the debounce time) |
| 4 | Slide the flame switch **LEFT** | Red blink + buzzer, `flame=DETECTED` |
| 5 | Slide it back **RIGHT** | Back to green |
| 6 | Click the **gas sensor** while running and raise the gas level | `gasRaw` climbs. When it passes `alert>…`, the alarm fires and prints `ALERT` |
| 7 | Lower the gas level again | Back to green (no latching; same as the real sketch) |
| 8 | Trigger two at once (e.g. hold the button + flame LEFT) | Still one alarm. The Serial line shows *which* inputs are active, which is what SCADA will later use to tell a real alarm from an injected one (Scenario 4) |

Things to notice while you play:
- **OR logic:** any one input raises the alarm. The real firmware later adds a
  fourth source, the remote Modbus coil `ALARM_ACTIVATE` (Phase B).
- **Non-blocking code:** the LED keeps blinking and the button keeps
  responding at the same time, because the sketch uses `millis()` instead of
  `delay()`.
- **Baseline calibration:** the gas threshold isn't a fixed number. It's
  whatever the sensor read at boot plus a margin of 400. Change the gas
  sensor's `ppm` in `diagram.json`, restart, and watch the baseline move.

## Real bench differences

The simulation can't show these, so they still have to be checked on the
real bench:

- **Buzzer:** Wokwi's buzzer acts like a *passive* piezo and stays silent on
  a plain HIGH, so this sketch uses `tone()` behind `#define WOKWI_SIM 1`. The
  real part is an *active* buzzer, and the bench sketch just uses
  `digitalWrite(HIGH)`.
- **Gas sensor AO voltage:** a real MQ board powered at 5V can push AO close
  to 5V, and GPIO 34 only tolerates 3.3V. Measure AO with a multimeter
  before wiring it (see `../MQ-2_Gas_Smoke_Sensor/Wiring.md`). Wokwi never
  warns about voltage.
- **Warm-up:** real MQ sensors need 20–30 s or more of heater warm-up before
  the baseline means anything. Wokwi's reads correctly right away.
- **Sensor model:** Wokwi simulates an MQ-2, but the BSH bench currently
  uses an **MQ-9**, so real `gasRaw` and baseline numbers will differ.
- **Flame sensor:** it's a slide switch here. The real module's sensitivity
  potentiometer, and false triggers from sunlight or IR remotes, only show
  up with the real part.
- **Button pull-up:** the sim adds a 10kΩ resistor because a bare Wokwi
  button would float. Check whether your real module already has one (the
  Keyestudio module does).

## Troubleshooting

- **No sound:** check your browser/tab isn't muted and that `WOKWI_SIM` is `1`.
  The buzzer's `volume` is set to 0.2 in `diagram.json`; raise it if needed.
- **Alarm never fires from gas:** if the boot message says the baseline is
  too high, lower `"ppm"` for `gas1` in `diagram.json` and restart.
- **Alarm is on at boot:** the flame switch is probably LEFT. Slide it
  RIGHT. (`"value": "1"` in `diagram.json` starts it on the right.)
- **Button seems to flicker:** that's Wokwi simulating contact bounce on
  purpose. The 50 ms debounce should hide it; if it doesn't, that's a real
  bug worth logging.
