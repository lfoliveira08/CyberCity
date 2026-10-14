# Magnetic Reed Switch — BSH-Door

Reference doc: `BSH_Sensors/08_Reed_Switch.md`

## What it does here
Confirms the gate's **actual** physical open/closed state, completely
independent of what the servo was told to do. This is the sensor that
makes "commanded open, actually still closed" (or vice versa) a
detectable mismatch — the design point of this node.

## *** Read this before wiring — hardware caveat not in the general sensor doc ***

`BSH_Sensors/08_Reed_Switch.md` says to wire this as
`INPUT_PULLUP` on GPIO 34. **That won't work as written**: GPIO 34-39 on
the ESP32 are input-only ADC pins with no internal pull resistors in
silicon. `pinMode(PIN_REED, INPUT_PULLUP)` compiles fine and does nothing
— the input floats when the switch is open, giving unreliable/noisy
readings. `BSH_Door_Node.ino` already uses plain `INPUT` and expects a
**real external pull-up resistor (~10kΩ to 3.3V)** across the switch. Add
that resistor as part of wiring this component, not as a later fix.

## Key specs
- Normally-open magnetic reed switch
- Digital input — external pull-up required (see above), NOT
  `INPUT_PULLUP`
- Magnet mounted on the moving gate arm; switch body mounted on the fixed
  frame

## Part in the Phase A build (Oct 9, 2026) -- needs a check
The Phase A build uses a 3-pin reed **module** (S / V / G), not the bare
switch described below. It is **not yet confirmed** whether the module has its
own pull-up resistor. GPIO 34 has none internally, so check on the bench:
wire S to GPIO 34, V to 3.3V, G to ground, read the `Reed raw=` value with the
magnet near and away. A steady 0/1 that changes cleanly means the module
provides the pull-up. A floating or chattering value means add the external
10k resistor from GPIO 34 to 3.3V. Record the result in `Build_Log.md`.

## Wiring
| Signal | ESP32 pin | Notes |
|---|---|---|
| Reed switch (one leg) | GPIO 34 | Through a 10kΩ pull-up to 3.3V |
| Reed switch (other leg) | GND | |

## Firmware tie-in
`BSH_Door_Node.ino`: `PIN_REED = 34`, `pinMode(PIN_REED, INPUT)` (not
`INPUT_PULLUP`). `readDoorPositionOpen()` reads it raw and reports
`DOOR_POSITION` (DI1) — 1 = open, 0 = closed — completely independent of
`gateState`. Polarity is controlled by `REED_LOW_MEANS_CLOSED`; flip that
constant if your build reads backwards rather than rewiring.

## Bench-test checklist (Phase A, this component only)
- [ ] External 10kΩ pull-up resistor installed (to 3.3V) — this is the
      step most likely to be skipped since the general sensor doc doesn't
      mention it
- [ ] Switch + magnet mounted per your gate's mechanical design
- [ ] Flash the real firmware, watch `DOOR_POSITION` over Serial or a
      Modbus client
- [ ] Manually move the magnet away from/toward the switch and confirm the
      reading changes cleanly (no floating/chattering) — if it's noisy,
      the pull-up is either missing or too weak
- [ ] Confirm the 1=open/0=closed polarity matches reality; flip
      `REED_LOW_MEANS_CLOSED` in firmware if backwards

## Lessons Learned
*(fill in as you go — worth writing up clearly here, this GPIO 34 issue is
exactly the kind of thing worth flagging for whoever builds the FH mirror
of this node later)*

## Troubleshooting Tips
*(fill in as you go — e.g. "reading floats/chatters" -> missing or too-weak
external pull-up, not a code bug)*
