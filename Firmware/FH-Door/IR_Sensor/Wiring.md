# IR Sensor (Approach Detection) — FH-Door

> **Founder's Hall mirror.** Identical component and identical wiring to
> BSH-Door -- this file was copied from
> `Firmware/BSH-Door/IR_Sensor/Wiring.md` on Oct 9, 2026 and re-pointed at FH's
> own docs and IP (192.168.50.23). **Nothing here has been verified on FH
> hardware yet.** If a correction is found while building either building,
> fix both copies.

Reference doc: `FH_Sensors/07_IR_Sensor.md`

## What it does here
Detects approach/presence at the gate. Reported honestly as its own
Modbus point (`IR_DETECTED`) — it does not by itself open the gate or
trigger any local alarm; only the SCADA layer correlates it against
`ACCESS_GRANTED` for an intrusion judgment (see this system's README).

## Key specs
- IR obstacle/line-tracking sensor module, digital output
- Onboard potentiometer for sensitivity — tune this to your intended
  approach distance
- 3.3V powered

## Wiring
| Sensor pin | ESP32 pin |
|---|---|
| Signal (digital out) | GPIO 14 |
| VCC | 3.3V |
| GND | Common ground |

## Firmware tie-in
`FH_Door_Node.ino` (Phase A, Oct 9, 2026): `PIN_IR_SENSOR = 14`;
`readIrDetected()` applies the `IR_ACTIVE_LOW` flag (default `true`, meaning
LOW = object detected, as `FH_Sensors/07_IR_Sensor.md` describes). **The
flag is unverified on the real module.** The sketch prints `IR raw=` and
`IRDetected=` twice a second: hold a hand in front of the sensor and set the
flag so `IRDetected=1` appears with the hand there. The Phase B sketch must
publish `readIrDetected()` to `IR_DETECTED` (DI0), not a raw `== HIGH` test.

The sketch's comment names GPIO 4 as a fallback if a board has no GPIO 14.
That would be a pin-map change and has to be logged here and in
`FH_Systems/03_FH-Door.md` first. GPIO 14 is confirmed available on the FH
board (2026-10-07), so FH uses it.

## Bench-test checklist (Phase A, this component only)
- [ ] Sensor wired per the table above
- [ ] Polarity confirmed: read `IR raw=` with and without a hand in front,
      then set `IR_ACTIVE_LOW` so `IRDetected=1` means an object is present
- [ ] Flash the real firmware, open Serial Monitor or a Modbus client
      (once at Phase B) and watch `IR_DETECTED`
- [ ] Wave a hand/object at increasing distance from the sensor — adjust
      the onboard potentiometer until it only triggers at the intended
      approach distance, not from across the room

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go — e.g. "triggers constantly" -> sensitivity pot needs
adjusting, not a wiring fault)*
