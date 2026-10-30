# Relay Modules -- FH-PowerBox Power Rails

> **Redesigned Oct 28, 2026, mirroring `BSH_Sensors/09_Relay_Module.md`.** The old version (lights + fan) and the old `Fan_Relay/`/`Lights_Relay/` build folders are archived (`Archive/README.md`, Oct 28, 2026 rows).

**Used in:** FH-PowerBox node (3 power-rail channels — FH-HVAC, FH-FireAlarm, FH-Door)

## Function

Each relay switches power to one of the other three FH systems, under hub (or attacker) command. Mirrors BSH-PowerBox's redesign exactly, with FH-HVAC in place of Elevator (FH has no Elevator). See `FH_Systems/04_FH-PowerBox.md` Notes.

## Key Specs

- Three separate 1-channel 5V relay modules, one per rail (FH-HVAC, FH-FireAlarm, FH-Door)
- Typically active-LOW trigger (check **each** specific module — do not assume BSH-PowerBox's bench result carries over)
- **Not bench-confirmed on this node's own boards.** BSH-PowerBox's boards were active-HIGH; FH-PowerBox's are untested.
- ~70mA coil current per active channel (about 210 mA for all three energised at once)

## Wiring / Pins (FH-PowerBox ESP32)

| Channel | GPIO |
|---|---|
| FH-HVAC rail | GPIO 25 |
| FH-FireAlarm rail | GPIO 26 |
| FH-Door rail | GPIO 27 |

Same pins as BSH-PowerBox. Per-rail wiring not yet documented for this node — see `Firmware/FH-PowerBox/Schematic/Connections.md` (not yet updated for the power rails).

**Contact type — NO (normally open), mirrors BSH-PowerBox's Oct 27, 2026 decision.** COM feeds from a 5V source; NO feeds the downstream node's own 5V/VIN pin directly (the node's USB cable unplugged while powered this way — the "5V-pin method").

## Modbus Register Tie-In

- Coil `HVAC_RAIL_CMD` / `FIRE_RAIL_CMD` / `DOOR_RAIL_CMD` — master-written commands, 1 = powered
- Discrete Input `HVAC_RAIL_STATUS` / `FIRE_RAIL_STATUS` / `DOOR_RAIL_STATUS` — the rail's commanded state as the ESP32 drives it, not a measured voltage

## Notes

Same caution as BSH-PowerBox: on the BSH build, COM was fed from the PowerBox ESP32's own VIN pin rather than a separate supply rail, against the general "keep the switched load on its own rail" rule. Whether FH-PowerBox repeats that choice is not yet decided.

Firmware uses one flag, `RELAY_ACTIVE_LOW`, applied to all three rails. Test each board's own polarity before trusting the flag's value.

## In Wokwi (Phase A0)

**Wokwi part:** Native — `wokwi-relay-module` (one instance per rail)

**How to use it:** VCC, GND, IN→GPIO 25/26/27 for the three rails. A `wokwi-led` on each relay's NO/COM side stands in for "that node has power."

See `Firmware/WORKFLOW.md` Section 7 for the Phase A0 rule. The system's simulation lives in `Firmware/FH-PowerBox/Wokwi/` (mirrored from BSH-PowerBox, Oct 28, 2026).
