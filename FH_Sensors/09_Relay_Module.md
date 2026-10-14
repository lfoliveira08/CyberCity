# Relay Modules -- Lights + Fan

**Used in:** FH-PowerBox node (lights + fan) — Founder's Hall mirror of BSH-PowerBox

## Function
Switches the building's illumination lighting and a small fan on/off under hub (or attacker) command, or a local override button — models a smart building's power/environmental control layer. Replaced DALI dimming for the lighting channel (Sep 28, 2026) — see `FH_Systems/04_FH-PowerBox.md` Notes.

**Two separate 1-channel modules (Sep 30, 2026):** lights and fan were
originally planned to share one 2-channel relay module. Switched to two
separate 1-channel relay modules instead -- one board per channel -- see
the Sep 30, 2026 entry in `Piranga_Workflow_v2.md` Section 14. No
functional difference to the ESP32, but each board now has its own
`VCC`/`GND` wired to the ESP32, and the two relays are not guaranteed to
share a polarity just because they used to share a board.

## Key Specs
- Two separate 1-channel 5V relay modules, one per channel (lights, fan)
- Typically active-LOW trigger (check **each** specific module -- don't
  assume the lights relay matches the fan relay's result)
- **Bench-confirmed on this node 2026-10-07 (Phase B): active-HIGH, both flags `false`.** Original note: BSH-PowerBox's own two boards
  were bench-confirmed active-HIGH (`RELAY_LIGHTS_ACTIVE_LOW = false`,
  `RELAY_FAN_ACTIVE_LOW = false`) on Oct 5, 2026 -- see
  `BSH_Sensors/09_Relay_Module.md`. Worth trying active-HIGH first here,
  but this node's boards are different physical units and need their own
  independent Phase A test; `FH_PowerBox_Node.ino` keeps both flags at
  their untested `true` default until then.
- ~70mA coil current per active channel

## Wiring / Pins (FH-PowerBox ESP32)
| Channel | GPIO |
|---|---|
| Lights | GPIO 13 |
| Fan | GPIO 15 |

Each relay module has its own `VCC`/`GND` pair wired to the ESP32 (they
are no longer a shared pair from one board).

## Modbus Register Tie-In
- Coil `LIGHTS_ON_CMD` / Coil `FAN_CMD` — master-written commands
- Discrete Input `LIGHTS_STATUS` / Discrete Input `FAN_STATUS` — the relay's actual physical state, reported back honestly regardless of whether the hub or a local override button (see `FH_Sensors/02_Push_Button.md`) most recently changed it

**Feedback added Sep 29, 2026:** mirrors the BSH-PowerBox change — this used to be open-loop with no feedback Discrete Inputs wired. Adding the local override buttons was the occasion to close that gap.

## Notes
Keep whatever the relay switches (lights, fan motor) on its own power rail, physically separated from the ESP32's own 5V/3.3V supply — same separation principle used for the IRONCLAD track-power relay.

Firmware uses two independent flags, `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW`, matching the two separate physical boards. Bench-verify each one on its own module before flashing the real firmware -- don't carry over BSH-PowerBox's Oct 5, 2026 bench results, this is a different physical board.

This doc mirrors `BSH_Sensors/09_Relay_Module.md` exactly — same component, same wiring, only the building differs.

## In Wokwi (Phase A0)
**Wokwi part:** Native — `wokwi-relay-module` (one instance per channel)

**How to use it:** VCC, GND, IN→GPIO 13 for lights and IN→GPIO 15 for fan. Put a `wokwi-led` on the NO/COM side of each, so you can see when the contact closes.

**What the simulation hides:** Real modules vary in polarity, and Wokwi's `wokwi-relay-module` turned out to energize on HIGH — the opposite of the common active-LOW assumption this doc's Key Specs line above still reflects for real hardware. Phase A0 testing on BSH-PowerBox found it the hard way (see `Firmware/BSH-PowerBox/Wokwi/README.md`'s "Relay polarity" section): don't assume the simulation's polarity matches your real module, in either direction — check it fresh on the bench.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's ready-made simulation lives in `Firmware/<System>/Wokwi/`.
