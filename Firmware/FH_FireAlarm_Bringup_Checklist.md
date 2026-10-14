# FH-FireAlarm — Bring-Up Checklist

**Purpose:** Founder's Hall mirror of BSH-FireAlarm — first FH system,
built only after all five BSH systems are complete (Phase 2 of
`Firmware/WORKFLOW.md`). Identical hardware, wiring, and logic to
BSH-FireAlarm; only the static IP differs. Comparing this node's behavior
directly against BSH-FireAlarm's is the basis for the project's
cross-building cascade scenario (Scenario 8).

Firmware for this node: `FH_FireAlarm_Node.ino` (same folder as this file,
once Phase B starts) — confirmed identical to `BSH_FireAlarm_Node.ino` in
every respect except its IP address. Until Phase B, use the same
standalone no-WiFi/no-Modbus approach as BSH-FireAlarm's current firmware
(`Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`) — for
this mirror node you can likely reuse that exact sketch unmodified for
Phase A, since the hardware and GPIO map are identical.

---

## 1. Parts Needed

Identical BOM to BSH-FireAlarm — see `Firmware/BSH_FireAlarm_PILOT_Bringup.md`
Section 1: ESP32 DevKit, active buzzer, common-cathode RGB LED + 3×220Ω
resistors, momentary push button (pull-station stand-in), flame sensor,
MQ-2 gas/smoke sensor, breadboard/jumpers, a USB cable with data wires, and
the dedicated project Wi-Fi AP (shared across all eight nodes, already
broadcasting by this point).

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Buzzer signal | GPIO 23 | Straight digital drive |
| Buzzer GND | GND | |
| RGB LED — Red | GPIO 25 | Through a 220Ω resistor |
| RGB LED — Green | GPIO 26 | Through a 220Ω resistor |
| RGB LED — Blue | GPIO 27 | Through a 220Ω resistor |
| Pull-station button | GPIO 4 | `INPUT_PULLUP`, active LOW |
| Flame sensor | GPIO 18 | Plain `INPUT`, active LOW |
| MQ-2 gas/smoke (analog) | GPIO 34 | ADC1, input-only — verify this stays under 3.3V for your specific module before wiring; add a divider if it doesn't |

Identical pin map to BSH-FireAlarm — see `FH_Sensors/15_Flame_Sensor.md`
and `16_MQ-2_Gas_Smoke_Sensor.md` for the general component reference.

---

## 3. Arduino IDE Setup

Same as BSH-FireAlarm: ESP32 board support, board = **ESP32 Dev Module**,
**ModbusIP_ESP8266** library (Phase B/C only — no library needed for
flame/gas, both use plain `digitalRead()`/`analogRead()`). Once Phase B
starts, open `FH_FireAlarm_Node.ino` and fill in real `ssid`/`password` —
this node has its own copy of those constants, separate from
BSH-FireAlarm's.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-FireAlarm/Wokwi/ (FH mirrors BSH — copy it to `Firmware/FH-FireAlarm/Wokwi/`)` on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: buzzer, RGB LED, pull-station button, flame sensor (slide-switch stand-in), gas sensor
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

1. Wire per Section 2 — identical to BSH-FireAlarm.
2. Flash the same standalone integration sketch pattern used for
   BSH-FireAlarm (button/flame/gas all OR into the buzzer/LED alarm logic,
   printing state once a second over Serial).
3. Confirm the gas/smoke sensor's boot-time clean-air baseline calibration
   completes (watch Serial for "MQ-2 baseline established") before judging
   `GAS_SMOKE_ALERT` behavior.
4. Press the button — buzzer sounds, LED blinks red. Release — LED returns
   to steady green. Trip the flame sensor and (carefully, if testing gas
   response) the gas sensor — same alarm behavior should trigger from
   either.
5. If this doesn't work, it's wiring/hardware, not network — fix here
   first.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.22`
   — **not** `.12` (BSH-FireAlarm's address). This is the one line that
   should differ between the two nodes' firmware.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read DI0 (`MANUAL_PULL_TRIGGERED`), DI1 (`FLAME_DETECTED`), DI2
     (`GAS_SMOKE_ALERT`), and IR0 (`GAS_RAW_VALUE`) — all should track
     their respective sensors live.
   - Write Coil 0 (`ALARM_ACTIVATE`) = 1 — buzzer/LED should respond
     exactly like a local trigger.
   - Disconnect Wi-Fi with Coil 0 still set — alarm should stop once the
     watchdog timeout elapses (a stale remote command can't be honored
     once comms are considered down); the local button/sensors keep
     working regardless.
4. With both BSH-FireAlarm and FH-FireAlarm up simultaneously, confirm
   they report independently — triggering one should not affect the
   other's reported state (this is the point of Scenario 8).


**Connection-monitor check (added Oct 6, 2026):** read Input Register 1 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 2 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/FH-FireAlarm/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-FireAlarm` — its own Docker container at 192.168.50.122 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.22, port 502). Set discrete input
   count = 3, input register count = 3 (IR0 `GAS_RAW_VALUE`, IR1 `HEARTBEAT`, IR2 `RECONNECT_COUNT`), coil count = 1.
2. Write the same minimal ladder as BSH-FireAlarm: pass a SCADA-writable
   internal coil through to `Coil0`, expose `DI0`-`DI2`/`IR0`-`IR2` for
   monitoring.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a status tile to the shared SCADA dashboard (Ignition), placed so it can be
   viewed side-by-side with BSH-FireAlarm's tile.

---

## 7. Success Criteria — This System Is Done When:

- All three trigger sources (button, flame, gas/smoke) work locally with
  zero network dependency, exactly matching BSH-FireAlarm's behavior
  (Phase A).
- Modbus confirms the correct IP (192.168.50.22) and independent operation
  from BSH-FireAlarm (Phase B).
- OpenPLC and the SCADA dashboard both show this node's live state, distinguishable
  side-by-side from BSH-FireAlarm's (Phase C).

At that point this pattern moves on to FH-HVAC (which replaced FH-Elevator in FH
slot 1 on Sep 26, 2026), per the project's
build order.
