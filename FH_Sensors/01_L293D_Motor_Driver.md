# L293D Motor Driver

> **NOT USED IN FOUNDER'S HALL as of Sep 26, 2026.** FH slot 1 changed from
> Elevator to an HVAC zone unit, and the elevator was this motor driver's only
> FH consumer. Kept, not deleted, for two reasons: its BSH mirror
> (`BSH_01_L293D_Motor_Driver.md`) is still live and in
> use on BSH-Elevator, and the specs/wiring here would otherwise have to be
> re-researched if FH ever regains an elevator. Same "annotate, don't delete"
> treatment the retired MQ-135 sheet got on Sep 14. See
> `FH_Systems/01_FH-HVAC.md`.

**Used in:** nothing, currently — the FH-Elevator node it served was retired on
Sep 26, 2026 (see the banner above). Its live counterpart is
`BSH_Sensors/01_L293D_Motor_Driver.md`, still in use on BSH-Elevator.

## Function
Drives the elevator's DC motor to move the cab up and down between the three simulated floors. Takes direction/enable commands from the ESP32 and switches motor current accordingly.

## Key Specs
- Dual H-Bridge driver IC
- Drives DC motors up to ~600mA per channel
- Motor supply: 4.5–36V (external supply required)
- Logic inputs are 5V-tolerant; most breakout modules work fine with ESP32's 3.3V logic — verify with your specific board

## Wiring / Pins (retired FH-Elevator ESP32 — reference only)
| Signal | GPIO |
|---|---|
| ENABLE (PWM / speed) | GPIO 18 |
| DIRA | GPIO 19 |
| DIRB | GPIO 5 |
| Motor GND | Common ground with ESP32 |

Motor power (Vmotor) comes from an **external supply**, not the ESP32's 3.3V/5V pins.

## Modbus Register Tie-In
Not exposed directly as a register — the elevator node's firmware translates hub commands into motor movement locally:
- Coils `CALL_FLOOR_1` / `CALL_FLOOR_2` / `CALL_FLOOR_3` trigger a move
- Discrete Input `ELEVATOR_MOVING` reports while the motor is running
- Input Register `CURRENT_FLOOR` reports the resulting floor once movement completes

## Notes
- Power the motor from an external 5V/6V supply — never from the ESP32 itself — to avoid brownouts (same rule that applied to your IRONCLAD servos).
- The L293D has internal flyback diodes, so no extra protection diodes are needed for a small DC motor.
- This doc mirrors `BSH_Sensors/01_L293D_Motor_Driver.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Stand-in — Wokwi has no L293D or DC motor part

**How to use it:** Two LEDs on GPIO 19 (DIRA) and GPIO 5 (DIRB) show direction, and a third LED on GPIO 18 (ENABLE) shows speed through PWM brightness. This teaches the H-bridge control logic (which pins go HIGH for up/down, what PWM does), not the real driver's wiring.

**What the simulation hides:** The real L293D needs its own motor supply (VCC2), shares ground with the ESP32, and can get hot or brown out the board. None of that exists in the simulation.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
