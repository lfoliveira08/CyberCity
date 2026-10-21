# BSH-BackupPower — Bring-Up Checklist

**Purpose:** fifth (last) BSH system in the build order (after Fire Alarm,
Elevator, Door, and Power Box — see `Firmware/WORKFLOW.md`). Replaces the
retired BSH-Perimeter node in this slot (see `Piranga_Workflow_v2.md`
Section 14, Sep 28, 2026). Lower firmware-library risk than Door/PowerBox
(no new third-party library — just `ModbusIP_ESP8266` and built-in
`analogRead`), but two analog thresholds need real bench calibration
(`LOW_BATTERY`'s trip point, and the ACS712's zero-current baseline) —
same "budget real bench time, not the night before the demo" caution
already flagged for the retired Perimeter node's `DISTANCE_CM` threshold
and the Fire Alarm's gas baseline.

Firmware for this node: `BSH_BackupPower_Node.ino` (same folder as this
file, once Phase B starts). Not written yet. Until then, per
`Firmware/WORKFLOW.md` Section 5, Phase A uses a standalone no-WiFi/no-Modbus
integration sketch, same pattern as
`Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 1 | SPST toggle switch | Utility-power-loss stand-in — see `BSH_Sensors/17_Toggle_Switch.md` |
| 1 | 1-channel relay module | ATS transfer actuator — see `BSH_Sensors/09_Relay_Module.md` |
| 1 | Small demo load (LED or small fan) | Driven through the relay, so a transfer is visible |
| 2 | Resistors (divider) | Value TBD — size to the actual battery pack, see `BSH_Sensors/18_Voltage_Divider_Battery_Sense.md` |
| 1 | Small battery pack (2xAA, 9V, etc.) | Whatever's on hand — the divider ratio depends on it |
| 1 | ACS712 current sensor module | See `BSH_Sensors/19_ACS712_Current_Sensor.md` and its 5V-swing wiring caution |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Utility-loss toggle switch | GPIO 26 | `INPUT_PULLUP`, active LOW |
| ATS transfer relay | GPIO 27 | Check active-LOW/active-HIGH against your specific module, same as Power Box's fan relay |
| Battery voltage divider tap | GPIO 34 | ADC1 — compute R1/R2 for your actual battery pack before wiring |
| ACS712 OUT | GPIO 35 | ADC1 — **measure actual OUT swing with a multimeter first**; add a divider if it exceeds 3.3V |

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries (Phase B/C): **ModbusIP_ESP8266** only — no new third-party
   library for this node, same as Power Box now (DALI retired Sep 28, 2026),
   unlike Door (MFRC522).
4. Once Phase B starts: open `BSH_BackupPower_Node.ino` and fill in your
   real `ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-BackupPower/Wokwi/` on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: toggle switch, relay/LED, two potentiometers standing in for the battery divider and the ACS712
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — the divider
math and the ACS712's real voltage swing still get checked on the bench in
Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Build order for this node (digital first, analog second — the two analog
inputs are this node's real unknowns):

1. **Toggle switch alone** — confirm `UTILITY_POWER_LOST` flips cleanly
   over Serial.
2. **Relay alone** — plain digital output, confirm the demo load turns
   on/off cleanly and check active-LOW/active-HIGH polarity.
3. **Voltage divider alone** — with a multimeter on the battery and on
   the ADC tap simultaneously, confirm the ADC reading tracks a known
   fraction of the real battery voltage before trusting any
   percentage-scaling math in firmware.
4. **ACS712 alone** — confirm a stable baseline reading with no load
   current flowing, then confirm the reading moves when the demo load
   draws current. Measure OUT with a multimeter first per the wiring
   caution.
5. **Combined standalone integration sketch** — all four signals reported
   over Serial together, no WiFi/Modbus. Model it on
   `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.
6. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials in `BSH_BackupPower_Node.ino` and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.1.15`.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read DI0 (`UTILITY_POWER_LOST`) — tracks the switch live.
   - Read DI1 (`LOW_BATTERY`) and IR0 (`UPS_BATTERY_PCT`) — confirm DI1
     flips exactly when IR0 crosses the threshold, and that IR0 keeps
     reporting real readings regardless of DI1's state.
   - Read IR1 (`LOAD_CURRENT_MA`) — changes when the demo load is toggled.
   - Write Coil0 (`ATS_TRANSFER_CMD`) = 1 — the relay/demo load responds.
   - Disconnect Wi-Fi (or otherwise simulate comm loss) with
     `ATS_TRANSFER_CMD` active — confirm it defaults to **not commanded**
     once the watchdog timeout elapses. This is the opposite fail
     direction from the retired Perimeter node's `ZONE_DISABLE` (which
     failed toward "still watching") — here, an uncommanded transfer
     during a dropped link would be the bad outcome, so it fails safe
     toward "no transfer."
4. Note in this node's `Build_Log.md` the real R1/R2 values used and the
   actual `LOW_BATTERY` threshold chosen, so the calibration isn't lost.

---

## 6. Phase C — OpenPLC + SCADA Integration

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-BSH-BackupPower` — its own Docker container at
   192.168.1.115, Web UI on port 8080, Modbus server on port 502; it polls
   this node's ESP32 at 192.168.1.15, port 502). Set discrete input
   count = 2, coil count = 1, input register count = 2.
2. Write the minimal ladder: pass `ATS_TRANSFER_CMD` through to Coil0, and
   surface DI0/DI1/IR0/IR1 for the dashboard.
3. Confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add utility-status, battery-%, load-current, and an ATS-transfer button
   to the shared Ignition dashboard.

---

## 7. Success Criteria — This System Is Done When:

- All four signals (utility status, derived low-battery flag, raw battery
  %, raw load current) report correctly and independently with zero
  network dependency (Phase A).
- A plain Modbus write to `ATS_TRANSFER_CMD` drives the relay, and it
  fails toward **not commanded** on comm loss — distinguishable from the
  retired Perimeter node's opposite fail direction (Phase B).
- OpenPLC and Ignition both show this node's live state (Phase C).
- The real R1/R2 divider values and the `LOW_BATTERY` threshold are
  calibrated against the actual battery pack in hand, not left as
  placeholders.

This is the last of the five BSH systems. Once complete, the same pattern
moves on to Phase 2 (Founder's Hall), starting with FH-FireAlarm, per
`Firmware/WORKFLOW.md` Section 1.
