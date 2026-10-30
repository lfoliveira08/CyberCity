> Copied from BSH-PowerBox on Oct 6, 2026 (FH mirrors BSH pin-for-pin). Re-verify everything on FH's own boards -- especially relay polarity.

# Fan Relay — FH-PowerBox

> **FH correction (Oct 7, 2026):** the "Oct 5, 2026" polarity dates below were carried over from BSH-PowerBox. FH's own board was bench-confirmed active-HIGH (`false`) on Oct 7, 2026 during Phase B.

Reference doc: `BSH_Sensors/09_Relay_Module.md`

## What it does here
Switches the fan on/off under hub command (`FAN_CMD` coil) — the simpler
of this node's two outputs, build and test this one first.

## Key specs
- Its own dedicated 1-channel 5V relay module — as of Sep 30, 2026,
  lights and fan are two separate physical boards, not two channels of
  one module (see `../Lights_Relay/Wiring.md` for the lights channel's
  own relay; DALI was retired Sep 28, 2026, see
  `BSH_Systems/04_FH-PowerBox.md` Notes)
- **Polarity confirmed Oct 5, 2026: `RELAY_FAN_ACTIVE_LOW = false`
  (active-HIGH)** on this specific 1-channel board, verified via the
  Phase A combined standalone integration sketch (see `Build_Log.md`). A
  Sep 30, 2026 bench test had separately found the *old*, now-retired
  2-channel module's fan channel active-HIGH too — that earlier result
  did not carry over on its own, but the fresh Oct 5 test on this new
  board happened to land on the same value.
- ~70mA coil current when active
- Relay VCC from 5V, GND common with ESP32 (this board's own VCC/GND
  pair, not shared with the lights relay); keep whatever the relay
  switches (the fan itself) on its own power rail, physically separate
  from the ESP32's own supply

## Wiring
| Relay module pin | ESP32 pin |
|---|---|
| IN (signal) | GPIO 15 |
| VCC | 5V |
| GND | Common ground |

Fan motor (DC motor + fan blade) wired through the relay's NO/COM contacts
on its own power rail — not through the ESP32. `COM` -> 5V rail, `NO` ->
motor `+`, motor `-` -> GND rail directly.

## Firmware tie-in
`FH_PowerBox_Node.ino`: `PIN_FAN_RELAY = 15`, driven by `setRelay(pin, on,
activeLow)`, called with `RELAY_FAN_ACTIVE_LOW` (its own independent flag
as of Sep 30, 2026 -- no longer shared with the lights channel). Set to
`false` as of Oct 5, 2026, confirmed bench-correct on this board.
Commanded by Modbus Coil `FAN_CMD` (Coil1) -- Phase B verified
Oct 7, 2026.

## Bench-test checklist (Phase A, this component only)
- [x] Relay wired per the table above (its own VCC/GND, own board), fan
      on its own separate supply
- [x] Power up the ESP32 (used the Phase A Step 4 combined integration
      sketch) — confirmed the relay clicks and the motor actually spins,
      not just the relay's own LED
- [x] Confirmed active-LOW/active-HIGH fresh on **this** board —
      active-HIGH (`RELAY_FAN_ACTIVE_LOW = false`), Oct 5, 2026 — don't
      reuse the retired 2-channel module's result even though it landed
      the same way

## Lessons Learned
- Don't assume "typically active-LOW" holds for a specific module — both
  the original 2-channel module's fan channel and this new 1-channel
  board turned out active-HIGH. Two boards landing on the same value is a
  coincidence worth noting, not a rule to extend to FH-PowerBox's boards
  without testing those too.
- A flashing mistake (wrong sketch on the board) can look exactly like a
  dead relay and dead buttons, including zero Serial output — check the
  flashed code before chasing a wiring theory.

## Troubleshooting Tips
- Relay clicks but fan doesn't spin -> check the fan's own power rail, not
  the ESP32/relay wiring.
- Fan spins opposite of what the Serial log says -> flip
  `RELAY_FAN_ACTIVE_LOW` and re-flash; this is expected module-to-module
  variation, not a wiring mistake. (Confirmed correct value for this
  board, Oct 5, 2026: `false`.)
- Zero Serial output for `f` or the fan button: confirm the flashed
  sketch is actually the current one before suspecting hardware.
