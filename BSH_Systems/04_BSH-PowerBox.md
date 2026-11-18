# BSH-PowerBox Node

> **Redesigned Oct 28, 2026.** This node no longer drives lights and a fan — it switches power to the other three BSH systems (Elevator, FireAlarm, Door). The old lights-and-fan design is archived: see `Archive/README.md` (sketch, PLC files, `Fan_Relay/`, `Lights_Relay/`). Adoption record: `Prototype_PowerRails/ADOPTION_CHECKLIST.md`; decisions and bench results: `Logs/2026-10-28.md`.
>
> **Elevator rail SUPERSEDED Nov 17, 2026 — this node switches only FireAlarm and Door power now, in practice.** The Elevator rail relay (GPIO 25, `ELEV_RAIL_CMD`/`ELEV_RAIL_STATUS`) is still physically present and in the PLC/register map, but it's wired to nothing: the Elevator brownout-looped under its own motor load on this rail and was moved to a permanent dedicated USB 5V supply instead. See the note in `Schematic/Connections.md`, `STATUS.md`, and `Logs/2026-11-17.md`. The register map, PLC program and tags below are **left unchanged** (the coil is harmless unused infrastructure) — only the physical wiring and the cascade/attack scripts changed.

**IP Address:** 192.168.50.14 (unchanged; the ESP32 at this address is now the former spare board on a breadboard, not the original lights-and-fan board, which is set aside)
**Field layer:** 3 relay channels (NO contact), one per downstream system, no local override buttons
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-BSH-PowerBox` — its own Docker container at 192.168.50.114 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.14:502
**PLC program:** `PLC_Logic/BSH-PowerBox/` (ladder diagram, register map, Structured Text program, build log)

## Role

Switches power to the other three BSH systems — Elevator, FireAlarm and Door — each its own relay channel, under hub (SCADA/PLC) command. No local manual control; a Serial command is available for bench testing only, with no Serial connection in the finished build.

**Why:** demonstrates that a "boring" infrastructure node (power distribution) is itself a viable attack target — cutting a rail is a different, heavier kind of manipulation than flipping lights, since it takes a whole downstream system offline. Idea raised by Luis Oct 27, 2026; adopted Oct 28, 2026 after bench testing.

## Components used

- 3 × 1-channel 5V relay module (Songle SRD-05VDC-SL-C type, or equivalent) — see `BSH_Sensors/09_Relay_Module.md`
- No lights, no fan, no local override buttons (dropped Oct 28, 2026)

## GPIO pin map

| Pin | Function |
|---|---|
| GPIO 25 | Elevator rail relay IN |
| GPIO 26 | FireAlarm rail relay IN |
| GPIO 27 | Door rail relay IN |

25, 26 and 27 are ordinary ESP32 pins — not strapping, SPI-flash or input-only (`References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md`, lines 56-101). Boot-time relay behaviour on these pins has not been tested.

## Modbus register map

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | ELEV_RAIL_CMD | Elevator rail on/off (master-written command); 1 = powered |
| Coil | Coil1 | FIRE_RAIL_CMD | FireAlarm rail on/off (master-written command) |
| Coil | Coil2 | DOOR_RAIL_CMD | Door rail on/off (master-written command) |
| Discrete Input | DI0 | ELEV_RAIL_STATUS | Elevator rail's commanded state as the ESP32 drives it — **not** a measured voltage |
| Discrete Input | DI1 | FIRE_RAIL_STATUS | same, FireAlarm |
| Discrete Input | DI2 | DOOR_RAIL_STATUS | same, Door |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot |

No mismatch bits: with no local button and no voltage sense, a rail's command and status can never disagree, so a mismatch could never be true. **OpenPLC Input Register count for this node: 2 (unchanged); Coil and Discrete Input counts: 3 each (were 2).**

## Notes

**Contact type — decided Oct 27, 2026: NO (normally open), fail-dark.** The relay coil off means no power; the coil must be actively energised for a rail to be powered. A PowerBox that is off, unflashed, or rebooting leaves the other three systems **without** power. This is the opposite of the old PowerBox's own fail-safe (lights/fan defaulted OFF but PowerBox staying up kept the building's own power on): here PowerBox being down takes three systems down with it. It also means `Piranga_Workflow_v2.md` Section 7's claim — that each ESP32+OpenPLC pair keeps running independently if the hub goes offline — no longer holds for the PowerBox: it is now a single point of failure for the other three BSH systems.

**Boot and comms-loss behaviour — decided Oct 28, 2026 (option A):** all three rails start **OFF** after a power-up, and stay off until the hub (SCADA, or a Modbus write before Ignition exists) commands them on. If nothing sends that command, the three systems stay dark. If the WiFi **link** is down for more than 5 seconds, all rails are forced off (same watchdog pattern as the old sketch); this detects WiFi link loss only, not a stopped PLC container or a silent master. After the link returns, rails stay off until the next fresh hub write (the ESP32 forgets the last hub value on reconnect, as before).

**Power delivery — the 5V-pin method (decided Oct 28, 2026):** each relay's COM feeds from a 5V source and its NO feeds the downstream node's own 5V/VIN pin directly (not through the node's USB cable). The node's USB cable must be **unplugged** while powered this way — connecting both at once is unverified and avoided. On the current bench build, COM is fed from this PowerBox ESP32's own VIN pin, so all three nodes draw through this board's own 5V path and the USB source behind it; that path's current limit is not known and nothing has been measured. In a hub- or strip-fed final build, COM would come from that shared 5V supply instead. See `Schematic/Connections.md` for the as-built wiring and the open electrical questions.

**The Elevator motor and Door servo are not switched by this node.** They need an external 5V supply, separate from their ESP32 (`BSH_Sensors/01_L293D_Motor_Driver.md`, `BSH_Sensors/06_SG90_Servo.md`), reported as power banks on the bench. Cutting a rail here cuts only that node's own ESP32 — the motor/servo supply is untouched. Whether that is the intended scope of "the system is off" was raised but not resolved; see `Prototype_PowerRails/Wiring_DRAFT.md`.

**Contradiction flagged, not resolved (Nov 17, 2026):** the line above says the Elevator motor has its own separate supply, but Luis stated directly on Nov 16/17 that the motor **shares the Elevator ESP32's own 5V rail**. These two statements disagree and neither has been re-measured to settle it. This discrepancy doesn't block anything now that the Elevator rail is superseded (above), but it should be checked against the real bench wiring before relying on either claim elsewhere.

**History — the old lights-and-fan design (Sep 28 – Oct 21, 2026):** this node originally drove a DALI-dimmed light channel and a plain-relay fan channel (DALI dropped Sep 28, 2026), then two separate 1-channel relay modules with local override buttons (Sep 29–30, 2026) and the "changed-value" hub-write rule (Oct 21, 2026, carried forward unchanged into this design — see Authority rule below). That design is archived; see `Archive/README.md` for the retired sketch, PLC files and component folders. The GPIO-12-strapping-pin lesson from that build (see the project's general strapping-pin caveat, `00_START_HERE.md` Section 7) still applies to any future GPIO choice on this node.

**Authority rule (unchanged from Oct 21, 2026):** OpenPLC rewrites the command coils on every poll (about 100 ms). The sketch counts a coil write as a fresh hub command only when its value differs from the last value the hub wrote (forgotten after a WiFi rejoin), so the hub cannot re-force the same value without first writing the opposite one. There is no local button here to contest authority with, so this mainly matters for the comms-loss/reconnect case above.

**Connection monitoring (unchanged since Oct 6, 2026):** `HEARTBEAT` and `RECONNECT_COUNT` work the same as on every node — see `CLAUDE.md` §2.

## Wokwi Simulation (Phase A0)

Simulate this system in Wokwi before any real wiring (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Elevator rail relay (GPIO 25) | `wokwi-relay-module` + `wokwi-led` as the node | Native |
| FireAlarm rail relay (GPIO 26) | same | Native |
| Door rail relay (GPIO 27) | same | Native |

Folder: `Firmware/BSH-PowerBox/Wokwi/` — rebuilt Oct 28, 2026 from the prototype sim (`Prototype_PowerRails/Wokwi/`), lights removed. Controls: Serial `1`/`2`/`3` toggle a rail, `a` cuts all, `r` restores all. Relay polarity in Wokwi energises on HIGH; real boards must each be tested on their own bench.

Per-part detail: `BSH_Sensors/09_Relay_Module.md`.
