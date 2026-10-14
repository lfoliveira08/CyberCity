# PIR Motion Sensor — BSH-Perimeter

Reference doc: `BSH_Sensors/13_PIR_Motion_Sensor.md`

## What it does here
Broad motion detection across the perimeter/pathway zone — the first of
this node's two independent sensors. Reported raw and honestly; whether
motion "means" an intrusion is a judgment left to the SCADA layer (see the
DESIGN NOTE in `BSH_Perimeter_Node.ino`).

## Key specs
- 3-pin HC-SR501-style module: OUT, VCC, GND
- 5V powered, but OUT is a clean digital logic level — no voltage divider
  needed (unlike the ultrasonic sensor in this same node)
- Onboard sensitivity + hold-time trimmers, and a trigger-mode jumper —
  set the jumper to **repeatable trigger (H)**

## Wiring
| Signal | ESP32 pin | Notes |
|---|---|---|
| OUT | GPIO 32 | Direct digital input |
| VCC | 5V | |
| GND | Common ground | |

## Firmware tie-in
`BSH_Perimeter_Node.ino`: `PIN_PIR = 32`, `pinMode(PIN_PIR, INPUT)`. Read
raw every loop iteration and published directly to `MOTION_DETECTED`
(DI0) — forced false only while `ZONE_DISABLE` is active and comms are
live (see the system doc's "Notes" on the fail-direction of that coil).

## Bench-test checklist (Phase A, this component only)
- [ ] Trigger-mode jumper set to repeatable (H)
- [ ] Sensitivity/hold-time trimmers set to a reasonable starting point
- [ ] Allow the module's ~30-60s post-power-on settle time before judging
      results
- [ ] Flash the real firmware, watch `MOTION_DETECTED` over Serial or a
      Modbus client
- [ ] Wave a hand in front of the sensor and confirm it triggers cleanly
      with the jumper on repeatable mode (should stay true while motion
      continues, not just pulse once)
- [ ] Check for false triggers from anything nearby that moves on its own
      (fans, sunlit windows) before mounting permanently

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "triggers randomly with nothing moving" ->
check for HVAC vents, direct sunlight, or other classic PIR false-trigger
sources near the mounting spot)*
