# BSH-Elevator Node

**IP Address:** 192.168.50.11
**Field layer:** GPIO/I2C (motor driver, floor buttons, LCD) — no dedicated field-bus protocol
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-BSH-Elevator` — its own Docker container at 192.168.50.111 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.11:502
**PLC program:** `PLC_Logic/BSH-Elevator/` (ladder diagram, register map, Structured Text program, build log)

## Role
Simulates a 3-floor elevator inside the BSH building. Runs locally off button presses and/or remote "call floor" commands, reporting its state through its own dedicated mini-PLC.

## Components Used
- L293D Motor Driver, Push Button ×3, I2C LCD Display — see `BSH_Sensors/`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 18 | Motor ENABLE (PWM) |
| GPIO 19 | Motor DIRA |
| GPIO 5 | Motor DIRB |
| GPIO 4 | Floor button 1 |
| GPIO 16 | Floor button 2 |
| GPIO 17 | Floor button 3 |
| GPIO 21 | LCD SDA |
| GPIO 22 | LCD SCL |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | ELEVATOR_MOVING | Motor currently running |
| Discrete Input | DI1 | ELEVATOR_FAULT | Fault/stall condition |
| Coil | Coil0 | CALL_FLOOR_1 | Remote call to floor 1 |
| Coil | Coil1 | CALL_FLOOR_2 | Remote call to floor 2 |
| Coil | Coil2 | CALL_FLOOR_3 | Remote call to floor 3 |
| Coil | Coil3 | ELEVATOR_DISABLE | Lock out the elevator (fault-injection target) |
| Input Register | IR0 | CURRENT_FLOOR | Reported current floor (1–3) |
| Input Register | IR1 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR2 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

## Notes
**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 3.**

`ELEVATOR_DISABLE` remains a good "compromised node" demo target — an attacker or the hub can lock the elevator remotely while the dashboard shows it disabled, distinguishable from a genuine `ELEVATOR_FAULT`.

*(This system was briefly designed around a BACnet/IP backhaul with a translation bridge — see `Alternate_Design_BACnet_MQTT_Option.md` if that direction is ever revisited.)*

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Push buttons ×3 (GPIO 4/16/17) | `wokwi-pushbutton` ×3 | Native |
| I2C 16×2 LCD (GPIO 21/22) | `wokwi-lcd1602`, `"pins": "i2c"` | Native |
| L293D + DC motor (GPIO 18/19/5) | 3× `wokwi-led` — ENABLE brightness = speed, DIRA/DIRB = direction | **Stand-in** (no L293D/DC motor part) |

Folder: `Firmware/BSH-Elevator/Wokwi/` — not built yet; created when BSH-Elevator becomes the active system. What to try: Press a floor button and watch the state machine run (door-close delay → moving → arrived) on the LCD while the direction LEDs light. Confirms the non-blocking `millis()` logic never freezes.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
