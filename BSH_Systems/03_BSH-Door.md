# BSH-Door Node (Gate)

**IP Address:** 192.168.50.13
**Field layer:** SPI (RFID reader) + plain GPIO (IR sensor, reed switch)
**Backhaul protocol:** Modbus TCP (direct — no bridge needed)
**Dedicated PLC:** `OpenPLC-BSH-Door` — its own Docker container at 192.168.50.113 (macvlan, running in the OpenPLC Docker VM 192.168.50.10 on the Proxmox server), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.50.13:502
**PLC program:** `PLC_Logic/BSH-Door/` (ladder diagram, register map, Structured Text program, build log)

> **Field layer updated Sep 2026:** the card reader was swapped from
> Wiegand to an MFRC522/RC522 RFID reader (SPI) — same role, same
> `ACCESS_GRANTED` register semantics. See Section 14, Lessons Learned &
> Decisions Log, of `Piranga_Workflow_v2.md` for the swap rationale.

## Role
Controls the automatic door/gate, styled after the IRONCLAD crossing gate, now with a genuine access-control front end: an RFID card reader grants entry, while the IR sensor and reed switch independently watch for unexpected approach and confirm actual position.

## Components Used
- SG90 Servo, RGB LED, IR Sensor, Reed Switch, **RFID Reader (MFRC522/RC522)** — see `BSH_Sensors/`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 13 | Servo signal (gate arm) |
| GPIO 25 | RGB LED — Red |
| GPIO 26 | RGB LED — Green |
| GPIO 27 | RGB LED — Blue |
| GPIO 14 | IR sensor (approach detect) |
| GPIO 34 | Reed switch (position feedback) |
| GPIO 18 | RFID reader — SCK (VSPI default) |
| GPIO 19 | RFID reader — MISO (VSPI default) |
| GPIO 23 | RFID reader — MOSI (VSPI default) |
| GPIO 32 | RFID reader — SDA/SS (chip select) |
| GPIO 33 | RFID reader — RST |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | IR_DETECTED | Approach/presence detected |
| Discrete Input | DI1 | DOOR_POSITION | Actual open(1)/closed(0) state |
| Discrete Input | DI2 | ACCESS_GRANTED | Valid card read accepted locally |
| Coil | Coil0 | DOOR_OPEN_CMD | Command to open (remote override) |
| Coil | Coil1 | DOOR_CLOSE_CMD | Command to close |
| Input Register | IR0 | HEARTBEAT | Counts up once per second (wraps at 65535). Frozen = node lost (added Oct 6, 2026) |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot; 0 after a reboot (added Oct 6, 2026) |

## Notes
**Connection monitoring (Oct 6, 2026):** every node exposes `HEARTBEAT` (counts up once a second, wraps at 65535) and `RECONNECT_COUNT` (WiFi rejoins since boot, 0 after a reboot) so SCADA can tell when this ESP32 drops off WiFi -- OpenPLC keeps answering with stale values, so only a counter the ESP32 itself increments proves it is alive. See `Piranga_Workflow_v2.md` Section 14, Oct 6, 2026. **OpenPLC Input Register count for this node: 2.**

`ACCESS_GRANTED` (from a legitimate RFID card read) versus `DOOR_OPEN_CMD` issued with no matching card event is your clean authorized-vs-injected-access contrast — a card-based open should always have `ACCESS_GRANTED` fire first; a remote/attacker-issued `DOOR_OPEN_CMD` without it is a detectable anomaly, same idea as the reed-switch mismatch check.

The RFID reader is a 3.3V-only part — power it from the ESP32's 3.3V pin, never 5V (see `BSH_Sensors/11_RFID_RC522_Reader.md`).

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| SG90 servo (GPIO 13) | `wokwi-servo` | Native |
| RGB LED (GPIO 25/26/27) | `wokwi-rgb-led` (`common: cathode`) + 3× 220Ω | Native |
| IR obstacle sensor (GPIO 14) | `wokwi-slide-switch` (left = object) | **Stand-in** |
| Reed switch (GPIO 34) + 10kΩ pull-up | `wokwi-slide-switch` + 10kΩ pull-up | **Stand-in** |
| RC522 RFID (SPI 18/19/23, SS 32, RST 33) | `board-mfrc522` | Native — 6 preset cards |

Folder: `Firmware/BSH-Door/Wokwi/` — built and run Oct 9, 2026 (Luis: looks fine). The reed stand-in models the real 3-pin reed module (S/V/G) with a slide switch and a 10kΩ pull-up. What to try: Tap an authorized card (e.g. Blue `01:02:03:04`) → gate opens, LED changes. Tap an unlisted card (e.g. Red `AA:BB:CC:DD`) → denied. Use a second preset card as the "cloned card" for Scenario 3.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
