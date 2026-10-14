# FH-HVAC Node

> **Bench part changed 2026-10-07:** the build uses a **DHT11** (3-pin module, whole-degree, 0-50 C, +/-2 C) because that is what was in hand. Wiring is identical, no external pull-up. Wokwi stays on the DHT22. Details: `Firmware/FH-HVAC/DHT11_Sensor/Wiring.md`.

**IP Address:** 192.168.50.21
**Field layer:** Digital (temperature/humidity sensor, one-wire) + relay-driven output ↔ ESP32
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-FH-HVAC` — its own Docker container at 192.168.50.121 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.21:502
**PLC program:** `PLC_Logic/FH-HVAC/` (ladder diagram, register map, Structured Text program, build log)

> **ADOPTED Sep 26, 2026 — this is FH slot 1.** It replaces FH-Elevator,
> keeping that node's IP (192.168.50.21), container IP (192.168.50.121), ports
> and OpenPLC slot; only the instance name changed, to `OpenPLC-FH-HVAC`.
> Same swap pattern as BSH-AirQuality → BSH-Perimeter (Sep 14).
>
> This is an FH-only substitution — BSH keeps `BSH-Elevator` unchanged in
> its own slot 1, so **BSH and FH are deliberately no longer mirrored 1:1**.
> Three systems (Fire Alarm, Door, Power Box) still mirror exactly.
> The retired `01_FH-Elevator.md` is in `Archive/`. Folded into
> `Piranga_Workflow_v2.md` (Sections 1, 3, 4, 5, 6, 8, 9, 11, 13, 14) and
> `Firmware/WORKFLOW.md` (Phase 2 order, status tracker, Section 7).

## Role
Simulates a zone HVAC/AC unit for Founder's Hall: reads ambient
temperature (and humidity, if the sensor used supports it) and reports a
compressor/fan run state, with the setpoint and enable/disable both
settable remotely from OpenPLC. Doubles as the project's demonstration
node for the "false sensor data injection" cyber-attack scenario in the
project brief — spoofing `TEMP_C` is a direct, realistic stand-in for a
real building-automation sensor-spoofing attack, which none of the other
four systems currently model as cleanly.

## Components Used
- DHT22 (AM2302) temperature/humidity sensor — see
  `FH_Sensors/17_DHT22_Temp_Humidity_Sensor.md` (written Sep 26, 2026). It is
  the one sensor sheet with no `BSH_Sensors/` mirror, because no BSH system
  uses it.
- Relay Module (compressor/fan simulation) — see `FH_Sensors/09_Relay_Module.md`
- RGB LED (idle / cooling / fault status) — see `FH_Sensors/05_RGB_LED.md`
- Small DC fan (optional, wired through the relay) — gives the demo
  something visibly moving when the unit is "running," the same role the
  buzzer/RGB play for Fire Alarm

## GPIO Pin Map (bench-confirmed 2026-10-07)
These pins were wired and run on the bench in Phase A and B (DHT11 GPIO 4, relay GPIO 26,
RGB 16/17/18). The note below is the original design reasoning, kept for the record. Two conventions already hold it to account: the DHT22 data
line needs an external 10kΩ pull-up, so it must stay off GPIO 34-39 (input-only,
no internal pull resistors in silicon — the lesson from the Door's reed switch),
and nothing on this node reads analog, so the ADC1/ADC2-with-WiFi caveat does
not apply here.

| Pin | Function |
|---|---|
| GPIO 4 | Temp/humidity sensor DATA (one-wire) |
| GPIO 26 | Relay control (compressor/fan) |
| GPIO 16 / 17 / 18 | RGB LED (R/G/B) |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Input Register | IR0 | TEMP_C | Raw temperature reading |
| Input Register | IR1 | HUMIDITY_PCT | Raw humidity reading (omit if the sensor used doesn't report one) |
| Discrete Input | DI0 | COMPRESSOR_RUNNING | Relay/fan output state, reported honestly |
| Discrete Input | DI1 | HVAC_FAULT | Sensor timeout or out-of-range reading |
| Holding Register | HR0 | TARGET_TEMP_C | Master-set setpoint — first use of a Holding Register in the project; every other master→slave value so far has been a Coil, but a setpoint is a written analog value, not a bit |
| Coil | Coil0 | HVAC_ENABLE | Master→slave. While clear, the unit is forced off regardless of `TARGET_TEMP_C`/`TEMP_C` — this is the register a false-data-injection or unauthorized-override demo would target |
| Input Register | IR2 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR3 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

## Notes
**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 4.**

Like the retired BSH-Perimeter's `MOTION_DETECTED`/`OBJECT_IN_RANGE`, this node
reports `TEMP_C` and `COMPRESSOR_RUNNING` as two independent raw signals
rather than deciding "is this an attack" locally — the SCADA/OpenPLC layer
correlates them (e.g., flags a fault if `COMPRESSOR_RUNNING` stays true
while `TEMP_C` isn't moving toward `TARGET_TEMP_C`).

**Decided 2026-10-07: tenths of a degree C for both TEMP_C and TARGET_TEMP_C (23.4 C -> 234).**
The original open question, kept for the record:
either send whole degrees (losing resolution) or send tenths (23.4°C → `234`),
in which case `TARGET_TEMP_C` must use the same scale or the comparison silently
breaks. This is the project's first analog setpoint, so the convention set here
is the one every later analog register should follow.

`HVAC_ENABLE`'s fail direction on comms loss still needs a deliberate
decision when this gets built. Fail-off (like `ELEVATOR_DISABLE`) avoids
a compressor being force-run unsupervised; fail-open (like `ZONE_DISABLE`)
keeps the zone climate-controlled if the network drops. Worth deciding
on purpose rather than by default, and documenting the choice the same
way the retired BSH-Perimeter's fail-open `ZONE_DISABLE` choice was called
out explicitly (no currently-active node fails in that direction).

This system has no BSH counterpart and won't get one — see the callout
at the top of this document.

## Wokwi Simulation (Phase A0)
Rule: `Firmware/WORKFLOW.md` Section 7. **This system cannot reuse a BSH
simulation** — it has no BSH counterpart, so `Firmware/FH-HVAC/Wokwi/` has to be
built from scratch when FH-HVAC becomes the active system. The upside: all three
parts are native Wokwi parts, so unlike every other system in the project this
one needs no stand-ins.

| Real part | Wokwi part | Type |
|---|---|---|
| DHT22 (GPIO 4) | `wokwi-dht22` | Native — click it to set temperature/humidity |
| Relay module (GPIO 26) | `wokwi-relay-module` + `wokwi-led` as the "fan" | Native |
| RGB LED (GPIO 16/17/18) | `wokwi-rgb-led` (`common: cathode`) + 3× 220Ω | Native |

What to try: raise the simulated temperature above `TARGET_TEMP_C` and confirm the
relay closes and the LED goes to its cooling colour; clear `HVAC_ENABLE` and confirm
the relay stays open regardless of temperature.
