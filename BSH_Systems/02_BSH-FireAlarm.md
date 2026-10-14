# BSH-FireAlarm Node

**IP Address:** 192.168.50.12
**Field layer:** GPIO (buzzer, RGB LED, manual pull-station button, flame sensor, MQ-2 gas/smoke sensor)
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-BSH-FireAlarm` — its own Docker container at 192.168.50.112 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.12:502
**PLC program:** `PLC_Logic/BSH-FireAlarm/` (ladder diagram, register map, Structured Text program, build log)

## Role
Sounds and visually signals a fire/intrusion alarm for BSH. Can be triggered locally (pull station), by an onboard flame or gas/smoke sensor, or remotely (hub command / attack simulation).

**Upgrade note:** this system originally shipped as a bare pull-station buzzer/LED node. It was upgraded to proper flame/smoke/gas detection by adapting the `bugsNburgers/IOT-Fire-Alarm-System` reference project — the sensor selection and wiring were kept, but the WiFi->Blynk cloud transport was replaced with a Modbus TCP register extension, consistent with the rest of this project. See Section 14 (Lessons Learned & Decisions Log) of `Piranga_Workflow_v2.md` for the full rationale, including why MQ-2 was chosen over the previously-considered MQ-135.

## Components Used
- Active Buzzer, RGB LED, Push Button (pull station), Flame Sensor, MQ-2 Gas/Smoke Sensor — see `BSH_Sensors/`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 23 | Buzzer |
| GPIO 25 | RGB LED — Red |
| GPIO 26 | RGB LED — Green |
| GPIO 27 | RGB LED — Blue |
| GPIO 4 | Manual pull-station button |
| GPIO 18 | Flame sensor digital OUT (active LOW) |
| GPIO 34 | MQ-2 gas/smoke sensor analog OUT (ADC1 — verify voltage before wiring, see `BSH_Sensors/16_MQ-2_Gas_Smoke_Sensor.md`) |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | MANUAL_PULL_TRIGGERED | Local pull station activated |
| Discrete Input | DI1 | FLAME_DETECTED | Flame sensor tripped |
| Discrete Input | DI2 | GAS_SMOKE_ALERT | MQ-2 reading exceeded baseline + margin |
| Input Register | IR0 | GAS_RAW_VALUE | Raw MQ-2 ADC reading (0–4095), for dashboard display |
| Input Register | IR1 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR2 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |
| Coil | Coil0 | ALARM_ACTIVATE | Remote/hub-triggered alarm |

## Notes
**Connection monitoring (Oct 6, 2026):** SCADA reads OpenPLC, which keeps answering with stale values if this ESP32 drops off WiFi, so loss is detected from `HEARTBEAT` freezing. Heartbeat resumes with `RECONNECT_COUNT` +1 = WiFi drop; both back at 0 = reboot. OpenPLC input register count for this node is now 3.

Firmware ORs all local trigger sources (button, flame, gas/smoke) plus the remote coil to sound the buzzer/blink the LED — unlike Door, which deliberately avoid local correlation and let the SCADA layer judge intrusion, Fire Alarm's job is to sound locally regardless of whether the SCADA layer is reachable. The individual Discrete Inputs are still exposed separately so the dashboard/SCADA layer can distinguish a physically-triggered alarm (button, flame, or gas/smoke) from a remotely-injected one (coil with no matching physical signal) — the basis of Scenario 4's false-alarm-injection contrast, now with three physical signals to compare against instead of one.

The MQ-2's raw value being its own Modbus point also gives the project's "false sensor data injection" cyber-layer demo a second flavor: a compromised node reporting a fabricated `GAS_RAW_VALUE`/`GAS_SMOKE_ALERT` with no real gas present, distinguishable from a flame-triggered alarm.

*(This system was briefly designed around a BACnet/IP backhaul with a translation bridge — see `Alternate_Design_BACnet_MQTT_Option.md` if that direction is ever revisited.)*

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Active buzzer (GPIO 23) | `wokwi-buzzer` | Native — sim uses `tone()`, real firmware uses `digitalWrite` |
| RGB LED (GPIO 25/26/27) | `wokwi-rgb-led` (`common: cathode`) + 3× 220Ω | Native |
| Pull-station button (GPIO 4) | `wokwi-pushbutton` + 10kΩ pull-up | Native |
| Flame sensor (GPIO 18) | `wokwi-slide-switch` (left = flame) | **Stand-in** (no flame sensor part) |
| MQ-2 gas/smoke (GPIO 34) | `wokwi-gas-sensor` | Native |

Built and ready: `Firmware/BSH-FireAlarm/Wokwi/`. Trigger the button, flame switch, and gas level one at a time and together; confirm steady green → blinking red + buzzer, and that the Serial Monitor shows which input caused it.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
