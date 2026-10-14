# BSH-BackupPower Node

**IP Address:** 192.168.1.15
**Field layer:** Digital (utility-loss switch, ATS relay) + analog (battery voltage divider, ACS712 current sensor) ↔ ESP32
**Backhaul protocol:** Modbus TCP (direct — no bridge)
**Dedicated PLC:** `OpenPLC-BSH-BackupPower` — its own Docker container at 192.168.1.115 (macvlan, running on the hub host 192.168.1.10), Web UI on port 8080, Modbus server on port 502 for the SCADA layer. It polls this ESP32 directly at 192.168.1.15:502

> **Replaces BSH-Perimeter** as the fifth BSH system (same IP, ports, and
> OpenPLC slot — see Section 14, Lessons Learned & Decisions Log, of
> `Piranga_Workflow_v2.md` for the swap rationale, Sep 28, 2026). The PIR
> motion sensor and HC-SR04 ultrasonic sensor are retired from the active
> build entirely — see `Archive/README.md`.

## Role
Monitors the campus's backup power system for a utility outage and
commands a test transfer to backup power, modeling the real-world
Automatic Transfer Switch (ATS) / UPS telemetry point that a real
facility's backup-power system exposes. Reports utility status, UPS
battery health, and load current draw. Unlike Perimeter/Motion, this node
has a genuine actuator (the ATS relay), and the failover it controls is a
classic real-world OT attack target — forcing a false failover test,
running down a UPS battery unnoticed, or blocking a generator start during
an actual outage.

## Components Used
- SPST Toggle Switch (utility-power-loss stand-in) — see `BSH_Sensors/17_Toggle_Switch.md`
- 1-Channel Relay Module (ATS transfer actuator) — see `BSH_Sensors/09_Relay_Module.md`
- Resistor Voltage Divider (UPS battery voltage sense) — see `BSH_Sensors/18_Voltage_Divider_Battery_Sense.md`
- ACS712 Current Sensor (load current sense) — see `BSH_Sensors/19_ACS712_Current_Sensor.md`

## GPIO Pin Map
| Pin | Function |
|---|---|
| GPIO 26 | Utility-power-loss toggle switch (`INPUT_PULLUP`, active LOW) |
| GPIO 27 | ATS transfer relay driver (output) |
| GPIO 34 (ADC1) | UPS battery voltage divider |
| GPIO 35 (ADC1) | ACS712 load current sensor analog out |

## Modbus Register Map
| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | UTILITY_POWER_LOST | Raw switch input, reported honestly |
| Discrete Input | DI1 | LOW_BATTERY | `UPS_BATTERY_PCT` below a fixed threshold |
| Input Register | IR0 | UPS_BATTERY_PCT | Scaled ADC reading — always reports the real value, regardless of `LOW_BATTERY` |
| Input Register | IR1 | LOAD_CURRENT_MA | Scaled ACS712 reading |
| Coil | Coil0 | ATS_TRANSFER_CMD | Master→slave. Drives the ATS relay to test/force a transfer to backup power. **Open-loop** — no feedback Discrete Input wired yet, same limitation as `BSH-PowerBox`'s `LIGHTS_CMD`/`FAN_CMD` |

## Notes
Like BSH-Door and the now-retired BSH-Perimeter, this node reports raw,
independent truth and leaves correlation to the SCADA/OpenPLC layer rather
than judging locally. For example: `ATS_TRANSFER_CMD` active while
`UTILITY_POWER_LOST` is false is a real-world red flag (someone forced a
transfer test with no actual outage) — it's the SCADA layer's job to flag
that inconsistency, not this node's.

`ATS_TRANSFER_CMD`'s fail direction on comm loss is the opposite of the
retired `ZONE_DISABLE`'s: it is **not** trusted, and defaults to "no
transfer commanded," following the same "remote command only honored
while comms are live, fails safe" pattern as `ELEVATOR_DISABLE`. An
uncommanded transfer test triggered by a dropped link during a real
outage would be the bad outcome here — the opposite asymmetry from a
security sensor like the old `ZONE_DISABLE`, which failed toward "still
watching."

`LOW_BATTERY`'s alert threshold is a placeholder pending a real
calibration pass against the actual battery and divider in hand — same
"budget real bench time for this, not the night before the demo" lesson
as the retired Perimeter node's `DISTANCE_CM` threshold and the Fire
Alarm's gas-alert baseline.

**Wiring caution:** many ACS712 breakout boards run their analog output
centered around half their supply voltage, and boards supplied at 5V can
swing close to or above 3.3V depending on current draw — measure the
actual output with a multimeter before wiring directly into GPIO 35, same
"measure it first" rule already used for the MQ-2 and HC-SR04.

## Wokwi Simulation (Phase A0)
Simulate this system in Wokwi before the Phase A bench build (rule: `Firmware/WORKFLOW.md` Section 7). Same GPIO map as the table above.

| Real part | Wokwi part | Type |
|---|---|---|
| Toggle switch (GPIO 26) | `wokwi-slide-switch` | Native |
| Relay module (GPIO 27) | `wokwi-relay-module` + `wokwi-led` | Native |
| Voltage divider / battery sense (GPIO 34) | `wokwi-potentiometer` | **Stand-in** (no battery/divider part) |
| ACS712 current sensor (GPIO 35) | `wokwi-potentiometer` | **Stand-in** (no ACS712 part in Wokwi) |

Folder: `Firmware/BSH-BackupPower/Wokwi/` — not built yet; created when BSH-BackupPower becomes the active system. What to try: flip the switch to simulate a utility outage, turn the "battery" potentiometer down past the `LOW_BATTERY` threshold, and command `ATS_TRANSFER_CMD` from a Modbus client to confirm the relay/LED responds. Confirms the four signals stay independent and raw, not locally fused.

Per-part detail (how to use it, what the simulation hides): the "In Wokwi (Phase A0)" section of each sheet in `BSH_Sensors/`.
