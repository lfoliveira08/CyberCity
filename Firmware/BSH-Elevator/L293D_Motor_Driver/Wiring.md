# L293D Motor Driver — BSH-Elevator

Reference doc: `BSH_Sensors/01_L293D_Motor_Driver.md`

## What it does here
Drives the elevator cab's DC motor up/down between the 3 simulated floors.

## Key specs
- Dual H-Bridge driver IC, drives DC motors up to ~600mA per channel
- Motor supply: 4.5-36V — **external supply required**, never from the
  ESP32's own 3.3V/5V pins (same brownout risk as the Door node's servo)
- Logic inputs are 5V-tolerant; most breakout modules work fine with the
  ESP32's 3.3V logic — verify with your specific board
- Has internal flyback diodes — no extra protection diodes needed for a
  small DC motor

## Motor in hand: DG01D-A130, 48:1 gearbox
Confirmed 2-wire plain brushed DC motor (not the DG01D-E encoder variant,
which has extra signal wires) — this is exactly what the L293D design
already assumes: 2 wires straight to OUT1/OUT2, nothing else to wire.

Per the manufacturer datasheet (SparkFun-hosted copy — see
`References/INDEX.md`):
- Suggested voltage: **4.5V DC**
- No-load output speed: **90 ±10 RPM** (after the 48:1 gearbox)
- No-load current: **190mA typ, 250mA max**
- Max torque: **800 gf·cm min**
- Stall current: not published — small brushed gearmotors like this can
  draw well above their no-load current when stalled/loaded hard, so
  don't assume it stays under the L293D's ~600mA/channel limit just
  because no-load current is low. Worth a quick check with a multimeter
  once the motor's under real cab load.

**Voltage note:** this motor's suggested voltage is 4.5V, lower than the
5-6V range mentioned above for a generic small DC motor. Running it at
6V will spin it faster/harder than rated — fine for short demo duty
cycles on most of these hobby gearmotors, but a 4.5-5V supply (e.g. 3xAA
batteries, or a regulated 5V source) is closer to spec if you want to
stay conservative. Either way, `MOTOR_SPEED` (PWM duty) still controls
the *effective* average voltage the motor sees — see "Speed / Timing
Calibration" below.

The rated 90 RPM no-load figure is also a sanity check for that
calibration: once you know the winch/pulley diameter converting rotation
to cab travel, `expected linear speed = RPM × (π × diameter) / 60` gives
a rough cross-check against whatever `MOTOR_SPEED`/`FLOOR_TRAVEL_MS`
you land on empirically — real gearbox friction and cab load will pull
the actual number below this no-load figure, so treat it as an upper
bound, not the answer.

## Wiring
| L293D pin | ESP32 pin |
|---|---|
| ENABLE (PWM/speed) | GPIO 18 |
| DIRA | GPIO 19 |
| DIRB | GPIO 5 |
| Motor GND | Common ground with ESP32 |

Motor power (Vmotor) from an **external** 5V/6V supply, not the ESP32.

## Firmware tie-in
`BSH_Elevator_Node.ino`: `PIN_MOTOR_ENABLE/DIRA/DIRB = 18/19/5`. Speed set
via `ledcAttach()`/`ledcWrite()` at `MOTOR_SPEED = 200` (0-255 PWM). Not
exposed as a direct register — the elevator's state machine translates
`CALL_FLOOR_1/2/3` coils (or button presses) into `motorRunToward()` calls,
then reports `ELEVATOR_MOVING` (DI0) and `CURRENT_FLOOR` (IR0) once the
non-blocking travel timer completes.

## Optional bench debug LEDs (separate pins, not tapped off these lines)
See `../Wiring_Diagram.svg` for the full picture. Rather than tapping the
LEDs directly off GPIO 18/19/5 (which would have worked electrically --
the L293D's logic input draws negligible current -- but means physically
splicing two things onto one wire on the breadboard), these use their own
free pins instead:

| Debug LED | Pin | Mirrors |
|---|---|---|
| ENABLE/speed (yellow) | GPIO 23 | GPIO 18 |
| DIRA/up (green) | GPIO 25 | GPIO 19 |
| DIRB/down (red) | GPIO 27 | GPIO 5 |

Trade-off: cleaner wiring (no tap/splice), but the combined Phase A
integration sketch (not written yet -- see `Firmware/WORKFLOW.md`
Section 7) needs a few extra lines to mirror each real signal onto its
debug pin, e.g. `ledcWrite(23, sameDutyAs18)` alongside
`ledcWrite(18, ...)`, and `digitalWrite(25, ...)` alongside
`digitalWrite(19, ...)`. Zero cost if tapped directly off 18/19/5 instead
-- this is a deliberate choice to keep the breadboard simpler at the cost
of a few extra firmware lines.

## Bench-test checklist (Phase A, this component only)
- [ ] Motor driver wired per the table above, motor on its own external
      supply
- [ ] Power up the ESP32 with the real firmware — at boot the motor should
      be stationary (ENABLE at 0 duty)
- [ ] Trigger a floor call (once buttons are wired) — motor should spin in
      the correct direction (check DIRA/DIRB aren't swapped if it goes
      backwards) and stop cleanly when the travel timer completes
- [ ] Watch the LCD ("Going up.../Going down...", "Arrived!") to confirm
      the state machine and the actual motor motion agree

## Speed / Timing Calibration — target-time method

`MOTOR_SPEED` (PWM duty on ENABLE, GPIO 18) and `FLOOR_TRAVEL_MS` (how long
the motor runs per floor, in the firmware's state machine) are both
currently placeholders (`200` and `6000`) carried over from the original
`Elevator_Motor.ino` prototype — not measured from this hardware. Once the
motor and cab mechanism are wired and moving, calibrate them for real.

**Measured floor-to-floor cab travel distance: 119.705 mm.**

Procedure — pick a target travel time first, then tune the PWM to hit it
(rather than calculating PWM from a theoretical speed, since duty cycle
isn't linearly proportional to actual cab speed):

1. Decide the travel time you want per floor for the demo (e.g. 2-3
   seconds — long enough to read on the LCD, short enough not to drag).
2. Temporarily set `MOTOR_SPEED` to a starting value (e.g. the current
   `200`) and run the motor for that target duration with the cab
   mechanism attached.
3. Measure how far the cab actually moved with a ruler/tape measure.
   - Moved less than 119.705 mm in that time -> raise `MOTOR_SPEED`.
   - Moved more than 119.705 mm -> lower `MOTOR_SPEED`.
   - If lowering it stalls the motor instead of slowing it down smoothly,
     you've hit the stall-torque floor under the cab's load — pick a
     longer target time instead of forcing the PWM lower.
4. Repeat steps 2-3, adjusting `MOTOR_SPEED`, until the cab covers
   119.705 mm in (approximately) the target time.
5. Lock in that `MOTOR_SPEED` and set `FLOOR_TRAVEL_MS` to the target time
   you tuned for — the measured value, not a distance/speed calculation on
   paper, since real friction/load/gearing losses don't show up in a pure
   calculation.
6. Log the final `MOTOR_SPEED` / `FLOOR_TRAVEL_MS` pair, and how many
   tuning passes it took, in this component's `Build_Log.md` under Lessons
   Learned — later systems with their own timing questions (e.g. Door's
   servo) can reuse the same method.

This is still open-loop timing, not position feedback — see
`Firmware/BSH-Elevator/Wokwi/README.md`'s "Real bench differences" for why
that's an intentional simplicity trade-off here, and
`References/GitHub/Elevator_KaziRifatAlMuin.md` for what a closed-loop
alternative (current sensing, ultrasonic) would look like if ever revisited.

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "motor doesn't move" -> check the external
supply is actually connected, not just the logic pins; "motor runs
backwards" -> swap DIRA/DIRB wiring or the two constants in firmware)*
