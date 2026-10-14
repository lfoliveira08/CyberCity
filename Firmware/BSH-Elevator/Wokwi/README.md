# BSH-Elevator — Wokwi Simulation (Phase A0)

A working simulation of the whole BSH-Elevator system: same GPIO pins, same
state-machine logic the real node firmware will use
(`../BSH_Elevator_Node/BSH_Elevator_Node.ino`, which now exists and was
bench-confirmed on 2026-09-25 — see `../SYSTEM_REPORT.md`). There is no
Founder's Hall twin any more: FH slot 1 became an HVAC zone unit on
Sep 26, 2026, so this simulation is BSH-only. Use it to learn how the non-blocking door-close → moving → arrived
sequence works, how the L293D's ENABLE/DIRA/DIRB lines drive movement, and
how a stall gets caught — all without touching real hardware.

Rule: `Firmware/WORKFLOW.md` Section 7. Modeled on the project's first
simulation, `Firmware/BSH-FireAlarm/Wokwi/`.

## Files

| File | What it is |
|---|---|
| `diagram.json` | The parts and every wire |
| `sketch.ino` | The firmware — same state machine as the (future) real node, minus WiFi/Modbus |
| `README.md` | This guide |
| `libraries.txt` | The one library this sketch needs: `LiquidCrystal_I2C` |

## How to open it

1. Go to **wokwi.com** → *Start from scratch* (or *New Project*) → **ESP32**.
2. In the `sketch.ino` tab, delete everything and paste this folder's `sketch.ino`.
3. In the `diagram.json` tab, delete everything and paste this folder's `diagram.json`.
4. Add the library: use Wokwi's Library Manager (books icon) to add
   `LiquidCrystal_I2C`, or paste this folder's `libraries.txt` into a
   `libraries.txt` tab if your project has one — either way, Wokwi needs
   that library present before the sketch will build.
5. Press the green **▶ Play** button. The Serial Monitor opens underneath.
6. Optional: sign in and **Save** so the project gets its own link the team can reuse.

## What's in the circuit

| GPIO | Part in Wokwi | Real part | Notes |
|---|---|---|---|
| 4 / 16 / 17 | 3× push button | 3× floor-call button | `INPUT_PULLUP` in firmware — no external resistor, same as the bench |
| 21 / 22 | I2C LCD (`wokwi-lcd1602`, `"pins": "i2c"`) | 16×2 I2C LCD | Same `LiquidCrystal_I2C` library and `0x27` address as the bench |
| 19 | Green LED | L293D DIRA | **Stand-in** (Wokwi has no L293D/DC motor part). Lit while the cab moves up |
| 5 | Red LED | L293D DIRB | **Stand-in.** Lit while the cab moves down |
| 18 | Yellow LED | L293D ENABLE (PWM) | **Stand-in.** Brightness = `MOTOR_SPEED` — fixed at 200/255 in this design (no variable-speed logic), so it's either off or at that one brightness |

## Scenarios to try

Start with nothing pressed: the LCD reads "Elevator Ready / Floor 1", all
three LEDs off, and the Serial Monitor prints the startup banner.

| # | Do this | You should see |
|---|---|---|
| 1 | Press **Floor 2** | LCD: "Floor 2 requested / Doors closing..." for 3s, then "Going up... / To Floor 2" with the **green** LED (DIRA) and **yellow** LED (ENABLE) both on, then ~6s later "Arrived! / Floor 2" for 2s, then back to "Elevator Ready / Floor 2" |
| 2 | Press **Floor 2** again while already on Floor 2 | LCD: "Already on / Floor 2" — no motor movement |
| 3 | Press **Floor 1** | Same sequence, but the **red** LED (DIRB) lights instead of green — direction depends on whether the target floor is above or below the current one |
| 4 | Press a button mid-travel (e.g. Floor 3 while still moving to Floor 2) | Ignored — the state machine only accepts a new request from `ELEV_IDLE`, per "no queueing" |
| 5 | While the cab is "moving" (after the door-close delay, before arrival), type **`s`** + Enter into the Serial Monitor | ~2s later: LEDs go dark, LCD shows "FAULT / Check elevator", Serial prints `ELEVATOR_FAULT latched...`. Press any floor button afterward — LCD shows "FAULT latched / Reset to clear" and nothing moves. Only a Wokwi restart (stop ■ / play ▶ again) clears it, same as a board reset on the bench |

## Real bench differences

The simulation can't show these, so they still have to be checked on the
real bench:

- **The L293D itself:** the simulation only shows the *logic* — which pins
  go HIGH for which direction, and PWM brightness for speed. The real
  L293D needs its own external motor supply (never the ESP32's own rail),
  shares ground with the ESP32, and can get warm under load — none of that
  exists here.
- **Fault trigger:** on the bench, `ELEVATOR_FAULT` latches from a *real*
  stall (a jammed cab, a disconnected motor) caught by the same watchdog
  timer already in this sketch. Wokwi has no physical motor to jam, so this
  sketch adds one simulation-only test hook — the `s` Serial command,
  behind `#if WOKWI_SIM` — that isn't needed (and does nothing useful) on
  real hardware.
- **`ELEVATOR_DISABLE` isn't simulated here.** It's a remote-lockout Modbus
  coil (Phase B) with no local equivalent, so it doesn't exist until Modbus
  does. Don't confuse the `s` stall test with it: a stall is a genuine
  local fault; disable will be a deliberate remote action once Phase B
  lands. Keeping the two visibly distinguishable is the whole point of
  having both — see `BSH_Systems/01_BSH-Elevator.md`'s Notes section.
- **I2C address:** same `0x27` default as the bench — real backpacks
  sometimes ship at `0x3F`, and a bad contrast pot can make a working
  display look blank. Neither problem can happen in the simulation.
- **Debounce:** Wokwi simulates real contact bounce on its buttons, same as
  the bench; the 50ms debounce should hide it here just like on real
  hardware.
- **Travel timing is fixed, not measured.** Both this sketch and the real
  design move the cab for a fixed duration (`FLOOR_TRAVEL_MS` per floor)
  rather than sensing cab position — there's no distance/position sensor
  in this node's BOM. If that ever changes (e.g. a limit switch or
  ultrasonic sensor to detect the cab reaching a floor), it's a hardware
  addition, not something this simulation already anticipates.

## Troubleshooting

- **LCD stays blank:** double-check `"pins": "i2c"` is set on `lcd1` in
  `diagram.json` — without it, Wokwi expects the full 16-pin parallel LCD
  wiring instead of I2C.
- **A floor button does nothing:** confirm its GPIO in `diagram.json`
  matches the pin table above, and that it's wired to GND (not 3V3) — same
  failure mode as the Fire Alarm's pull-station button.
- **LEDs never light:** check the GPIO reaches the resistor, and the
  resistor's other leg reaches the LED's anode (`A`, not `C`) — a swapped
  `A`/`C` on a `wokwi-led` leaves it dark with no error message.
- **Build fails / `LiquidCrystal_I2C.h` not found:** the library wasn't
  added — see step 4 in "How to open it."
- **Fault won't latch when you type `s`:** it only works while `state ==
  ELEV_MOVING` (after the door-closing delay, mid-travel) — try again right
  after a "Going up/down..." message appears, and give it ~2 seconds.
