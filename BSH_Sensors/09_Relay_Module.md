# Relay Modules -- BSH-PowerBox Power Rails

> **Redesigned Oct 28, 2026.** This sheet used to describe the lights + fan relays; the old version, and the old `Fan_Relay/`/`Lights_Relay/` build folders, are archived (`Archive/README.md`, Oct 28, 2026 rows).
>
> **Elevator rail unused as of Nov 17, 2026** — the relay itself is still in place (GPIO 25) but switches nothing now; the Elevator brownout-looped under motor load through it and moved to permanent USB power. See `Firmware/BSH-PowerBox/Schematic/Connections.md`.

**Used in:** BSH-PowerBox node (3 power-rail channels — Elevator, FireAlarm, Door)

## Function

Each relay switches power to one of the other three BSH systems, under hub (or attacker) command — models a power-distribution layer where cutting one rail takes a whole downstream system offline, a heavier form of manipulation than flipping a light. Replaces the old lights + fan relays (dropped Oct 28, 2026) — see `BSH_Systems/04_BSH-PowerBox.md` Notes.

## Key Specs

- Three separate 1-channel 5V relay modules, one per rail (Elevator, FireAlarm, Door). Songle SRD-05VDC-SL-C type reported on the bench.
- Typically active-LOW trigger (check **each** specific module — don't assume one rail's result matches another's)
- **Not individually bench-confirmed per rail on this build.** One relay board was tested with a multimeter (continuity across COM/NO) during the prototype phase and found consistent with the chosen contact type; the other two were not separately verified. See `Logs/2026-10-28.md`.
- ~70mA coil current per active channel (about 210 mA for all three energised at once)

## Wiring / Pins (BSH-PowerBox ESP32)

| Channel | GPIO |
|---|---|
| Elevator rail | GPIO 25 |
| FireAlarm rail | GPIO 26 |
| Door rail | GPIO 27 |

Each relay module has its own `VCC`/`GND` pair wired to the ESP32's 5V/GND rail (assumed shared across all three; not confirmed as electrically separate pairs — see `Firmware/BSH-PowerBox/Schematic/Connections.md`).

**Contact type — NO (normally open), decided Oct 27, 2026.** Unlike a relay used for simple on/off switching where either contact works, here the choice matters: with NO, the relay must be actively energised for the rail to be powered, so a dead or unflashed PowerBox leaves the downstream node **without** power ("fail-dark"). COM is fed from the PowerBox ESP32's own VIN pin; NO feeds the downstream node's own 5V/VIN pin directly (not its USB cable, which must be unplugged while powered this way — see `BSH_Systems/04_BSH-PowerBox.md` Notes, "5V-pin method").

## Modbus Register Tie-In

- Coil `ELEV_RAIL_CMD` / `FIRE_RAIL_CMD` / `DOOR_RAIL_CMD` — master-written commands, 1 = powered
- Discrete Input `ELEV_RAIL_STATUS` / `FIRE_RAIL_STATUS` / `DOOR_RAIL_STATUS` — the rail's commanded state as the ESP32 drives it. **Not a measured voltage** — there is no local button or voltage-sense input on this design, so these can never disagree with the last command the way the old lights/fan status bits could.

## Notes

Keep whatever a relay switches on its own power rail, physically separated from the ESP32's own supply — the old project rule, carried forward, but **not followed on the current build**: COM is fed from this board's own VIN pin, so all three downstream nodes draw through this ESP32's own 5V path and whatever source is behind it. Current through that path has not been measured.

Firmware uses one flag, `RELAY_ACTIVE_LOW`, applied to all three rails (the old design's two independent per-channel flags, `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW`, are gone with the old channels). Test each board's own polarity before trusting the flag's value — don't assume one relay's result for another, same rule as always.

## In Wokwi (Phase A0)

**Wokwi part:** Native — `wokwi-relay-module` (one instance per rail)

**How to use it:** VCC, GND, IN→GPIO 25/26/27 for the three rails. A `wokwi-led` on each relay's NO/COM side stands in for "that node has power."

**What the simulation hides:** real current draw, the DevKit's VIN-path limit, and whether the relay pins glitch at ESP32 reset. Wokwi's `wokwi-relay-module` energises on HIGH — check each real module fresh on the bench, don't assume either direction.

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's simulation lives in `Firmware/BSH-PowerBox/Wokwi/`.
