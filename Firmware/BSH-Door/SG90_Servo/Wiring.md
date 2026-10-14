# SG90 Micro Servo — BSH-Door

Reference doc: `BSH_Sensors/06_SG90_Servo.md`

## What it does here
Physically opens/closes the gate arm — same role as on the IRONCLAD
crossing gate this project's patterns draw from.

## Key specs
- SG90 micro servo, ~500mA stall current
- **Requires an external 5V supply** — do not power from the ESP32's
  3.3V pin, powering it from the ESP32 directly can brown out the whole
  board when the servo stalls or starts moving
- Standard `Servo.h` is **not** ESP32-compatible — this firmware uses
  `ESP32Servo.h` instead

## Wiring
| Servo wire | Connection |
|---|---|
| Signal (orange) | GPIO 13 |
| VCC (red) | External 5V supply |
| GND (brown) | Shared ground with ESP32 |

## Firmware tie-in
`BSH_Door_Node.ino`: `PIN_SERVO = 13`, `Servo gateServo;` from
`ESP32Servo.h`, `gateServo.attach(PIN_SERVO)`. Movement is a non-blocking
ramp (`SERVO_STEP_DEG = 2` every `SERVO_STEP_INTERVAL_MS = 20`) between
`GATE_CLOSED_ANGLE = 0` and `GATE_OPEN_ANGLE = 90`, driven by the gate
state machine (`GATE_CLOSED/OPENING/OPEN/CLOSING`).

## Bench-test checklist (Phase A, this component only)
- [ ] Servo wired per the table above, **external 5V supply connected**
      (confirm before powering the ESP32 — don't test with the servo
      powered from the ESP32 "just to check")
- [ ] Flash the real firmware (or a throwaway test sketch that sweeps
      0->90->0) — servo should move smoothly, not jerk or reset the ESP32
      (a reset while moving is the classic sign of powering it from the
      wrong rail)
- [ ] Confirm the gate arm doesn't bind mechanically at either end of
      travel — adjust `GATE_OPEN_ANGLE` if 90° over- or under-shoots your
      physical arm

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "ESP32 resets when servo moves" -> it's on the
wrong power rail, not a code bug)*
