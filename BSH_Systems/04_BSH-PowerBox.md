# BSH-PowerBox Node

**IP Address:** 192.168.50.14
**Field layer:** Plain relay (lights + fan) + local override push buttons, 2-channel relay module
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-BSH-PowerBox` — its own Docker container at 192.168.50.114 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.14:502
**PLC program:** `PLC_Logic/BSH-PowerBox/` (ladder diagram, register map, Structured Text program, build log)

## Role
Controls building illumination and a fan, both plain on/off, under hub command or a local physical override switch per channel.

## Components Used
- 2-Channel Relay Module (lights + fan) — see `BSH_Sensors/09_Relay_Module.md`
- Push Button ×2 (local lights/fan override switches) — see `BSH_Sensors/02_Push_Button.md`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 13 | Lights relay |
| GPIO 15 | Fan relay |
| GPIO 4 | Lights local override button |
| GPIO 16 | Fan local override button |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | LIGHTS_ON_CMD | Lights on/off (master-written command) |
| Coil | Coil1 | FAN_CMD | Fan on/off (master-written command) |
| Discrete Input | DI0 | LIGHTS_STATUS | Lights' actual physical state (honest feedback — may disagree with the last command if a local button moved it since) |
| Discrete Input | DI1 | FAN_STATUS | Fan's actual physical state (same idea) |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

## Notes
**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 2.**

**Local override buttons (Sep 29, 2026):** added a real physical push-button switch per channel — like an actual room light switch — alongside the existing remote Modbus command. See `BSH_Sensors/02_Push_Button.md` and the Sep 29, 2026 entry in `Piranga_Workflow_v2.md` Section 14 for the full rationale and the authority rule (whichever source, hub or button, acted most recently wins; the hub only regains authority on a *fresh* coil write, not merely by holding its old value; the buttons work even with no network at all). This also closes a gap `09_Relay_Module.md` used to flag — commands were previously open-loop with no feedback; `LIGHTS_STATUS`/`FAN_STATUS` (DI0/DI1) now report the real physical state honestly, so SCADA can see when a local button press has made reality disagree with the last command it sent.

**Design change (Sep 28, 2026):** this node originally used DALI for individually dimmable lighting — see the retired `BSH_Sensors/12_DALI_LED_Driver.md` and `References/GitHub/PowerBox_silahcen001_DALI-ESP32.md`. DALI was dropped in favor of a plain on/off relay for lights, the same pattern already used for the fan. This removes the project's only unverified field-bus library and its untested transceiver hardware; the trade-off is that Power Box no longer demonstrates individually addressable dimming or the "partial dim = something's off" cascade cue. See the Sep 28, 2026 entries in `Piranga_Workflow_v2.md` Section 14.

Power Box is now a plain Modbus-relay node — the same pattern already proven on Fire Alarm and Elevator — rather than the project's second-highest-risk node.

**GPIO correction (Sep 28, 2026):** the lights relay was originally assigned GPIO 12, carried over from the old DALI TX pin without re-checking it. GPIO 12 is an ESP32 strapping pin (MTDI, samples flash voltage at reset) — a relay module's onboard IN pull-up drags it HIGH during that boot-time window, which was suspected of causing inverted/unreliable behavior in the Phase A0 Wokwi test. Moved to GPIO 13, a plain GPIO with no boot-strapping role. **Update (Sep 29, 2026):** this move was worth keeping (GPIO 12 is a real hazard for any pulled-up peripheral), but it turned out not to be what actually caused that inversion — see the Wokwi Wokwi/README.md "Relay polarity" section and the Sep 29, 2026 entry in `Piranga_Workflow_v2.md` Section 14: the real cause was Wokwi's simulated relay module energizing on HIGH rather than LOW, on both channels, not a GPIO-specific issue at all. See `00_START_HERE.md` Section 7 for the still-valid general strapping-pin caveat.

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Lights relay (GPIO 13) | `wokwi-relay-module` + `wokwi-led` as the "light" | Native |
| Fan relay (GPIO 15) | `wokwi-relay-module` + `wokwi-led` as the "fan" | Native |
| Lights override button (GPIO 4) | `wokwi-pushbutton` | Native |
| Fan override button (GPIO 16) | `wokwi-pushbutton` | Native |

Folder: `Firmware/BSH-PowerBox/Wokwi/` — built Sep 28, 2026, when BSH-PowerBox became the active system; buttons added Sep 29, 2026. What to try: cycle through lights and fan on/off from a Serial command or the button parts. Both relay channels are native Wokwi parts, so unlike the old DALI stand-in, this simulation can prove the whole node's logic, not just teach it. Also see the Wokwi folder's `README.md` for a real finding worth knowing before the bench build: Wokwi's simulated relay module turned out to energize on HIGH, not LOW — a simulator-only quirk, not something to carry over to the real firmware without your own bench verification.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
