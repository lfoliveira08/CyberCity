# FH-Door (Gate) — Bring-Up Checklist

> **Update 2026-10-09: Phase B bench-confirmed** at 192.168.50.23 (see `FH-Door/ESP32_Core_Modbus/Build_Log.md`). Phase C not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

**Purpose:** Founder's Hall mirror of BSH-Door — third FH system per
`Firmware/WORKFLOW.md`'s Phase 2 order (FireAlarm → Elevator → Door →
PowerBox, the same order as BSH). Identical hardware, wiring, and logic to BSH-Door (including its own
independent authorized-card list); only the static IP differs. Still worth
the same care BSH-Door needed — this is the same firmware, and the same
highest-risk components (RFID, reed-switch pull-up) apply here too.

Firmware for this node: `Firmware/FH-Door/FH_Door_Node/FH_Door_Node.ino`. Since
Oct 9, 2026 it holds the Phase A standalone sketch (no WiFi, no Modbus), so it
has no static IP yet. The Phase B version must use 192.168.50.23; the
first-pass Phase B sketch in the Claude project had a hardcoded BSH `.13`
that has since been fixed there. Same open checks as BSH-Door: set
`IR_ACTIVE_LOW` and `REED_LOW_MEANS_CLOSED` from the raw values, check the
reed module's pull-up, and confirm the card UID in the sketch (3D D2 C6 62)
belongs to FH. No IP
collision to worry about here, unlike two of the other FH nodes flagged
below in the FH-PowerBox checklist (and in the FH-Elevator one, before that node was retired — it is in `Archive/`).

---

## 1. Parts Needed

Identical BOM to BSH-Door — see `Firmware/BSH_Door_Bringup_Checklist.md`
Section 1: ESP32 DevKit, MFRC522/RC522 RFID reader (3.3V-only), cards to
badge, SG90 servo (external 5V supply), RGB LED + resistors, IR sensor,
reed switch + ~10kΩ pull-up resistor, breadboard/jumpers, USB cable with
data wires, shared project Wi-Fi AP.

---

## 2. Wiring

Identical pin map to BSH-Door — see `FH_Sensors/06_SG90_Servo.md`,
`07_IR_Sensor.md`, `08_Reed_Switch.md`, `11_RFID_RC522_Reader.md`,
`05_RGB_LED.md`.

| Component | ESP32 Pin | Notes |
|---|---|---|
| Servo signal | GPIO 13 | External 5V supply |
| RGB LED — Red | GPIO 25 | |
| RGB LED — Green | GPIO 26 | |
| RGB LED — Blue | GPIO 27 | Unused in this node's 2-color scheme |
| IR sensor | GPIO 14 | |
| Reed switch | GPIO 34 | **External ~10kΩ pull-up required — same caveat as BSH-Door** |
| RFID SCK/MISO/MOSI | GPIO 18/19/23 | VSPI default |
| RFID SDA/SS | GPIO 32 | |
| RFID RST | GPIO 33 | |
| RFID VCC | **3.3V — not 5V** | Same hazard as BSH-Door |

Same two hazards as BSH-Door apply here: the RFID module is 3.3V-only, and
GPIO 34-39 have no internal pull resistors in silicon, so `INPUT_PULLUP`
on the reed switch's GPIO 34 silently does nothing without a real external
resistor.

---

## 3. Arduino IDE Setup

Same as BSH-Door: ESP32 board support, board = **ESP32 Dev Module**,
**ModbusIP_ESP8266**, **ESP32Servo**, **MFRC522** (miguelbalboa/rfid), plus
the built-in **SPI** library. Once Phase B starts, open `FH_Door_Node.ino`,
fill in real `ssid`/`password`, and **badge FH's own cards** — this node's
`AUTHORIZED_CARDS[]` is independent of BSH-Door's; don't assume BSH's
badged cards carry over.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/FH-Door/Wokwi/` (a copy of BSH-Door's, made Oct 9, 2026) on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: servo, RGB LED, RFID reader (6 preset cards), IR sensor and reed switch (slide-switch stand-ins)
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Same build order as BSH-Door:

1. **RFID reader alone**, over Serial — verify 3.3V power first, then
   confirm a real card's UID prints reliably.
2. **RGB LED** and **IR sensor** — simple GPIO.
3. **Reed switch** — external pull-up wired first, confirm polarity
   against your actual magnet mounting.
4. **SG90 servo** — external 5V supply.
5. **Combined standalone integration sketch** — badge a card, watch servo
   open and LED turn red, reed switch confirms open, then auto-close and
   green again. You can likely start from BSH-Door's Phase A sketch and
   just swap in FH's own card UID(s).
6. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials and FH's own card UID(s) in
   `FH_Door_Node.ino`, then reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.23`
   — this is already correct in the current firmware (see the fix note at
   the top of this checklist), but confirm it on real hardware anyway.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Badge an authorized (FH) card — `ACCESS_GRANTED` (DI2) pulses true,
     gate opens, `DOOR_POSITION` (DI1) flips.
   - Badge an unrecognized card — access denied.
   - Write Coil 0 (`DOOR_OPEN_CMD`) = 1 with no card badged — gate should
     still open (same authorized-vs-injected-access contrast as BSH-Door).
   - Write Coil 1 (`DOOR_CLOSE_CMD`) = 1 — gate closes.
4. With both BSH-Door and FH-Door up simultaneously, confirm an attack or
   fault on one doesn't affect the other's reported state — this
   side-by-side comparison (same function, same protocol, independent
   hardware) is specifically called out as a good resilience-scenario
   source in `FH_Systems/03_FH-Door.md`.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 0 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 1 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/FH-Door/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-Door` — its own Docker container at 192.168.50.123 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.23, port 502). Set discrete input count = 3, input register count = 2 (IR0 `HEARTBEAT`, IR1 `RECONNECT_COUNT`), coil count = 2.
2. Write the same minimal ladder as BSH-Door.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a status tile to the shared SCADA dashboard (Ignition), next to
   BSH-Door's, with the authorized-vs-injected contrast visible for both.

---

## 7. Success Criteria — This System Is Done When:

- All four components work together with zero network dependency, using
  FH's own badged card(s) (Phase A).
- Modbus confirms the correct IP (192.168.50.23) and independent operation
  from BSH-Door (Phase B).
- OpenPLC and the SCADA dashboard both show this node's live state, distinguishable
  side-by-side from BSH-Door's (Phase C).

At that point this pattern moves on to FH-PowerBox, per
`Firmware/WORKFLOW.md`'s Phase 2 order.
