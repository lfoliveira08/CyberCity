# FH-PowerBox Node

> **Redesigned Oct 28, 2026, mirroring `BSH_Systems/04_BSH-PowerBox.md`.** This node no longer drives lights and a fan — it switches power to the other three FH systems (FH-HVAC, FH-FireAlarm, FH-Door). The old lights-and-fan design is archived: see `Archive/README.md`. Not yet bench-confirmed on FH's own hardware; mirrors BSH-PowerBox's bench results and decisions.

**IP Address:** 192.168.50.24 (unchanged)
**Field layer:** 3 relay channels (NO contact), one per downstream system, no local override buttons
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-FH-PowerBox` — its own Docker container at 192.168.50.124 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.24:502
**PLC program:** `PLC_Logic/FH-PowerBox/` (ladder diagram, register map, Structured Text program, build log)

Mirrors `BSH-PowerBox` exactly — same decisions, same GPIO map, same register layout. **FH has no Elevator**, so rail 1 is **FH-HVAC** instead of Elevator (the only naming difference from the BSH pattern).

## Role

Switches power to the other three FH systems — FH-HVAC, FH-FireAlarm and FH-Door — each its own relay channel, under hub (SCADA/PLC) command. No local manual control; a Serial command is available for bench testing only.

## Components used

- 3 × 1-channel 5V relay module (same type as BSH-PowerBox) — see `FH_Sensors/09_Relay_Module.md`
- No lights, no fan, no local override buttons (dropped Oct 28, 2026, mirroring BSH)

## GPIO pin map

| Pin | Function |
|---|---|
| GPIO 25 | FH-HVAC rail relay IN |
| GPIO 26 | FH-FireAlarm rail relay IN |
| GPIO 27 | FH-Door rail relay IN |

Same pins as BSH-PowerBox. 25, 26 and 27 are ordinary ESP32 pins — not strapping, SPI-flash or input-only (`References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md`, lines 56-101).

## Modbus register map

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | HVAC_RAIL_CMD | FH-HVAC rail on/off (master-written command); 1 = powered |
| Coil | Coil1 | FIRE_RAIL_CMD | FH-FireAlarm rail on/off (master-written command) |
| Coil | Coil2 | DOOR_RAIL_CMD | FH-Door rail on/off (master-written command) |
| Discrete Input | DI0 | HVAC_RAIL_STATUS | FH-HVAC rail's commanded state as the ESP32 drives it — **not** a measured voltage |
| Discrete Input | DI1 | FIRE_RAIL_STATUS | same, FH-FireAlarm |
| Discrete Input | DI2 | DOOR_RAIL_STATUS | same, FH-Door |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot |

No mismatch bits, same reasoning as BSH-PowerBox. **OpenPLC Input Register count for this node: 2 (unchanged); Coil and Discrete Input counts: 3 each (were 2).**

## Notes

All of BSH-PowerBox's design decisions carry over unchanged — see `BSH_Systems/04_BSH-PowerBox.md` Notes for the full reasoning on each:

- **Contact type: NO (normally open), fail-dark.** FH-PowerBox being off/unflashed/rebooting leaves FH-HVAC, FH-FireAlarm and FH-Door **without** power.
- **Boot and comms-loss (option A):** rails start OFF after power-up, stay off until the hub commands them on; forced off after 5 s without the WiFi link.
- **Power delivery — 5V-pin method:** each relay's NO feeds the downstream node's own 5V/VIN pin directly, node USB unplugged while powered this way. **Not yet wired or measured on FH's own hardware** — see `Firmware/FH-PowerBox/Schematic/Connections.md` once it exists.
- **FH-HVAC and FH-Door's own supplies:** whether FH-HVAC or the FH-Door servo have a separate supply the way BSH-Elevator's motor and BSH-Door's servo do is **not recorded** — check before assuming this node's relay cuts everything on that system.

**History — the old lights-and-fan design:** mirrored BSH-PowerBox's build order (DALI dropped Sep 28, 2026; two 1-channel modules Sep 30, 2026; changed-value hub-write rule Oct 21, 2026). That design is archived — see `Archive/README.md`.

## Wokwi Simulation (Phase A0)

Simulate this system in Wokwi before any real wiring (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above — the simulation is copied from `Firmware/BSH-PowerBox/Wokwi/` into `Firmware/FH-PowerBox/Wokwi/`, same convention as every other FH system mirroring BSH (`Firmware/WORKFLOW.md` Section 7), with rail names changed to HVAC/FireAlarm/Door.

Per-part detail: `FH_Sensors/09_Relay_Module.md`.
