# BSH-Door (Gate) — Bring-Up Checklist

> **Update 2026-10-09: Phase B bench-confirmed** at 192.168.50.13 (see `BSH-Door/ESP32_Core_Modbus/Build_Log.md`). The file path note below describing a Phase A sketch is out of date: `BSH_Door_Node.ino` now holds the Phase B sketch and the Phase A one is archived. Phase C not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

**Purpose:** this is the third BSH system in the build order (Fire Alarm
-> Elevator -> Door -> Power Box — see
`Firmware/WORKFLOW.md`). It's this project's single highest firmware-risk
node (per `Piranga_Workflow_v2.md` Section 9), so it comes after the
Modbus/watchdog/state-machine pattern is proven on Fire Alarm and Elevator,
and its Wokwi simulation (Phase A0) matters more here than anywhere else.

Firmware for this node: `Firmware/BSH-Door/BSH_Door_Node/BSH_Door_Node.ino`
(the path stays the same across phases, `Firmware/WORKFLOW.md` Section 8).
Since Oct 9, 2026 it holds the Phase A standalone no-WiFi/no-Modbus
integration sketch, same pattern as
`Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`. Before
flashing, set `IR_ACTIVE_LOW` and `REED_LOW_MEANS_CLOSED` from the raw values
it prints, and check whether the reed module has its own pull-up (see
`Firmware/BSH-Door/README.md`, Current status).

**Do not build from `Stepper_Motor_Reference.ino`** (kept only in the Claude project). It's
a saved user reference sketch (stepper + PIR), not the current design — it
conflicts with the documented SG90 servo actuator, uses blocking
`delay()`/`step()` calls that would freeze Modbus polling, collides with the
RFID reader's SPI pins (GPIO 18/19), and triggers on plain motion rather
than RFID access. See its own header comment for the full rundown.

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 1 | MFRC522/RC522 RFID reader (SPI) | **3.3V-only — never wire VCC to 5V.** See `BSH_Sensors/11_RFID_RC522_Reader.md`. Replaces the original Wiegand reader (Sep 2026 swap). |
| 1+ | RFID cards/fobs to badge | You'll need at least one real card's UID for `AUTHORIZED_CARDS[]` |
| 1 | SG90 micro servo (gate arm) | **External 5V supply, not the ESP32's own rail** — see `BSH_Sensors/06_SG90_Servo.md` |
| 1 | Common-cathode RGB LED (or 3 discrete LEDs) | Only 2 colors used (red/green); blue wired for BOM consistency |
| 3 | 220Ω resistors | RGB LED current limiting |
| 1 | IR sensor module | Approach detection — `BSH_Sensors/07_IR_Sensor.md` |
| 1 | Magnetic reed switch | Position feedback — see pull-up caveat below, `BSH_Sensors/08_Reed_Switch.md` |
| 1 | ~10kΩ resistor | External pull-up for the reed switch (see caveat below) |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Servo signal | GPIO 13 | External 5V supply for the servo itself |
| RGB LED — Red | GPIO 25 | Through a 220Ω resistor |
| RGB LED — Green | GPIO 26 | Through a 220Ω resistor |
| RGB LED — Blue | GPIO 27 | Through a 220Ω resistor (unused in this node's 2-color scheme) |
| IR sensor | GPIO 14 | Direct digital input |
| Reed switch | GPIO 34 | **See pull-up caveat below — external ~10kΩ to 3.3V required** |
| RFID SCK | GPIO 18 | VSPI default |
| RFID MISO | GPIO 19 | VSPI default |
| RFID MOSI | GPIO 23 | VSPI default |
| RFID SDA/SS (chip select) | GPIO 32 | |
| RFID RST | GPIO 33 | |
| RFID VCC | **3.3V — not 5V** | See hazard below |

**Hazard 1 — RFID power:** the MFRC522 module is a 3.3V-only part. Wiring
VCC to 5V risks damaging it. Double-check before powering up — it's an easy
mistake if you reuse a 5V rail out of habit from other sensors on this
node.

**Hazard 2 — reed switch pull-up:** GPIO 34-39 are the ESP32's input-only
ADC pins, which have **no internal pull-up/down resistors in silicon**.
`INPUT_PULLUP` on GPIO 34 compiles fine but silently does nothing, leaving
the input floating when the switch is open. Wire a real external ~10kΩ
pull-up resistor from GPIO 34 to 3.3V, or `DOOR_POSITION` will read
unreliable/noisy. With the pull-up in place, the switch reads LOW when its
contacts are closed — verify against your actual magnet mounting and flip
`REED_LOW_MEANS_CLOSED` in the firmware if it reads backwards.

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries (Phase B/C, via Library Manager / GitHub):
   - **ModbusIP_ESP8266**
   - **ESP32Servo** (standard `Servo.h` is NOT ESP32-compatible)
   - **MFRC522** (miguelbalboa/rfid — the standard library for this chip)
   - Built-in **SPI** library (bundled with the ESP32 core)
4. Once Phase B starts: open `BSH_Door_Node.ino`, fill in your real
   `ssid`/`password`, then **badge each real card with Serial Monitor open,
   copy the printed UID bytes, and replace the `0xDE, 0xAD, 0xBE, 0xEF`
   placeholder in `AUTHORIZED_CARDS[]`.**

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-Door/Wokwi/` on wokwi.com (new ESP32 project →
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

Build order for this node (RFID first, since it's the real unknown):

1. **RFID reader alone**, over Serial only — confirm VCC is 3.3V before
   powering up, then flash a bare MFRC522 example sketch and confirm a real
   card's UID prints out reliably and repeatably.
2. **RGB LED** and **IR sensor** — simple GPIO, same patterns already
   proven on Fire Alarm/other nodes.
3. **Reed switch** — wire the external pull-up first, confirm
   open/closed reads correctly with a multimeter or Serial print, and
   determine which polarity matches your actual magnet placement.
4. **SG90 servo** — external 5V supply; same brownout caution as every
   other servo in this project (don't power it from the ESP32's onboard
   regulator).
5. **Combined standalone integration sketch:** once all four are
   individually confirmed, build one no-WiFi/no-Modbus sketch exercising
   them together — badge a card, watch the servo open and the LED turn
   red, confirm the reed switch reports open, then auto-close and confirm
   green again. Model it on
   `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.
6. If anything here doesn't work, it's a wiring/hardware issue — fix it
   before moving to Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials and your real card UID(s) in
   `BSH_Door_Node.ino`, then reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.13`.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Badge an authorized card — `ACCESS_GRANTED` (DI2) should pulse true
     for ~2s, the gate should open, and `DOOR_POSITION` (DI1) should flip.
   - Badge an unrecognized card — access should be denied, gate stays as-is.
   - Read `IR_DETECTED` (DI0) — should track the IR sensor live.
   - Write Coil 0 (`DOOR_OPEN_CMD`) = 1 with **no** card badged — the gate
     should still open. This is deliberate and is the node's core teaching
     point: a remote/attacker-issued open command is honored without a
     matching `ACCESS_GRANTED` event, which is exactly the
     authorized-vs-injected-access contrast this node exists to
     demonstrate (see `BSH_Systems/03_BSH-Door.md` "Notes").
   - Write Coil 1 (`DOOR_CLOSE_CMD`) = 1 — gate should close.
4. Disconnect Wi-Fi mid-test — confirm new remote open/close commands stop
   being honored, but the card reader and the gate's own auto-close dwell
   timer keep working locally regardless.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 0 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 1 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/BSH-Door/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-BSH-Door` — its own Docker container at 192.168.50.113 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.13, port 502). Set discrete input count = 3, input register count = 2 (IR0 `HEARTBEAT`, IR1 `RECONNECT_COUNT`), coil count = 2.
2. Write the minimal ladder: expose `DI0`/`DI1`/`DI2` for monitoring, pass
   SCADA-writable internal coils through to `Coil0`/`Coil1`.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a status tile to the SCADA test dashboard (Ignition), and specifically a
   view that makes the `ACCESS_GRANTED` vs. `DOOR_OPEN_CMD`-without-a-card
   contrast visible — that's this node's signature demo.

---

## 7. Success Criteria — This System Is Done When:

- All four components (RFID, servo, IR, reed switch) work correctly
  together with zero network dependency (Phase A).
- A badged authorized card opens the gate; an unrecognized card doesn't; a
  bare remote Modbus write also opens it with no card event (Phase B).
- OpenPLC and the SCADA dashboard both show the same live state, including the
  authorized-vs-injected contrast (Phase C).
- Real card UID(s) are in `AUTHORIZED_CARDS[]` (not the placeholder).

At that point this pattern moves on to BSH-PowerBox, per the project's
chosen build order.
