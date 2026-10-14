# BSH-Perimeter — Bring-Up Checklist

**Purpose:** this is the fifth and final BSH system in the build order
(Fire Alarm -> Elevator -> Door -> Power Box -> Perimeter/Motion — see
`Firmware/WORKFLOW.md`). It's one of the simpler nodes — two sensors, no
actuator — so it closes out BSH with a low-risk build.

Firmware for this node: `BSH_Perimeter_Node.ino` (same folder as this file,
once Phase B starts). Until then, per `Firmware/WORKFLOW.md` Section 5,
Phase A uses a standalone no-WiFi/no-Modbus integration sketch — modeled on
`Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino` — not
this real firmware file.

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 1 | PIR motion sensor (HC-SR501-style) | 3-pin: OUT/VCC/GND — see `BSH_Sensors/13_PIR_Motion_Sensor.md` |
| 1 | HC-SR04 ultrasonic distance sensor | 4-pin: VCC/TRIG/ECHO/GND — see `BSH_Sensors/14_HC-SR04_Ultrasonic_Sensor.md` |
| 1 | 1kΩ resistor | ECHO divider, high side |
| 1 | 2kΩ resistor | ECHO divider, low side (to GND) |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | Charge-only cables won't program the board |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B below |

No local actuator on this node (unlike Fire Alarm's buzzer/LED) — it's
sensor-only. Correlating its two signals into an "intrusion" judgment is
deliberately left to the SCADA layer, not this ESP32 (see the DESIGN NOTE
in `BSH_Perimeter_Node.ino`).

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| PIR OUT | GPIO 32 | Direct digital input, no divider needed |
| PIR VCC | 5V | |
| PIR GND | GND | |
| Ultrasonic TRIG | GPIO 14 | Direct connection, 3.3V-safe |
| Ultrasonic ECHO | GPIO 27 | **Through a 1kΩ/2kΩ voltage divider — see caveat below** |
| Ultrasonic VCC | 5V | |
| Ultrasonic GND | GND | Shared with the divider's ground leg |

**Read this before wiring ECHO:** the HC-SR04's ECHO pin outputs 5V, which
is not safe to wire directly into an ESP32 GPIO. Build the divider first —
1kΩ from ECHO to GPIO 27, 2kΩ from GPIO 27 to GND (drops 5V to
5V × 2000/3000 ≈ 3.33V) — and **verify the voltage at the GPIO side with a
multimeter** before ever connecting it to the ESP32.

Also set the PIR's trigger-mode jumper to **repeatable (H)** so the output
stays HIGH while motion continues, instead of pulsing once.

---

## 3. Arduino IDE Setup

1. Install ESP32 board support in Arduino IDE (Boards Manager → search
   "esp32" → install).
2. Select board: **ESP32 Dev Module**.
3. Install the **ModbusIP_ESP8266** library via Library Manager (needed for
   Phase B/C only — the Phase A standalone sketch needs no libraries beyond
   the ESP32 core).
4. Once Phase B starts: open `BSH_Perimeter_Node.ino` and fill in your real
   `ssid`/`password` — keep real values in a local, non-shared copy; leave
   placeholders in anything that goes to the Drive folder or git.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-Perimeter/Wokwi/` on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: PIR motion sensor, HC-SR04 ultrasonic
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Two sensors, bench-tested independently first, then together — no network,
no Modbus.

1. **PIR alone:** wire per Section 2, flash a simple sketch that prints
   `digitalRead(32)` on every change. Allow ~30-60s settle time after
   power-on. Wave a hand in front of it and confirm a clean, repeatable
   trigger (stays HIGH while motion continues, given the jumper is on
   repeatable mode). Watch for false triggers from HVAC vents or sunlit
   windows near the mounting spot.
2. **Ultrasonic alone:** build and multimeter-verify the ECHO divider
   *before* connecting it. Flash a sketch that pulses TRIG and prints the
   computed distance from a bounded `pulseIn()` (25ms timeout — this keeps
   a hung sensor from freezing the sketch). Confirm it reports a sane
   distance at a few known ranges, and a clear "no echo" sentinel (never a
   bare 0) when pointed at nothing solid.
3. **Combined standalone integration sketch:** once both are individually
   confirmed, build one sketch that reads both every loop (PIR raw, HC-SR04
   sampled every 150ms) and prints both to Serial together — this is the
   "does the hardware work as one system" step, kept deliberately separate
   from Phase B so hardware and network problems are never debugged at the
   same time. Model it on
   `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`'s
   structure (read all inputs, print state once a second).
4. If anything here doesn't work, it's a wiring/hardware issue — fix it
   before moving on.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in the real Wi-Fi credentials in `BSH_Perimeter_Node.ino` and
   reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.1.15`.
3. From a laptop on the same Wi-Fi/subnet, use a free Modbus TCP client
   (`modpoll` or QModMaster):
   - Read Discrete Input 0 (`MOTION_DETECTED`) — should track the PIR live.
   - Read Discrete Input 1 (`OBJECT_IN_RANGE`) and Input Register 0
     (`DISTANCE_CM`) — should track the ultrasonic sensor live; confirm
     `DISTANCE_CM` reads `9999`, not `0`, with nothing in range.
   - Write Coil 0 (`ZONE_DISABLE`) = 1 — both discrete inputs should force
     false while comms stay live; `DISTANCE_CM` should keep reporting real
     readings regardless (it's not a security judgment on its own).
   - Disconnect Wi-Fi (or otherwise simulate comm loss) with `ZONE_DISABLE`
     still set — confirm the discrete inputs go back to reflecting the raw
     sensors rather than staying forced false. This is a deliberate
     fail-direction difference from Door/Elevator's disable coils: a
     security sensor should keep watching by default if it loses contact
     with its own PLC, not go silently blind.
4. Watch specifically for Modbus timeout/slave-not-responding symptoms
   during normal ultrasonic sampling — the bounded `pulseIn()` approach is
   flagged in the firmware as not yet proven alongside the other nine
   nodes on real hardware.

---

## 6. Phase C — OpenPLC + SCADA Integration

Only move here once Phase B works cleanly.

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-BSH-Perimeter` — its own Docker container at 192.168.1.115,
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.1.15, port 502). Set discrete input count = 2, input register count = 1,
   coil count = 1.
2. Write the minimal ladder: expose `DI0`/`DI1`/`IR0` for monitoring, pass
   a SCADA-writable internal coil through to `Coil0`.
3. Confirm OpenPLC's Monitoring page shows the same live values seen in
   Phase B.
4. Point a Node-RED Modbus connection node at this OpenPLC instance and add
   a status tile to the test dashboard.

---

## 7. Success Criteria — This System Is Done When:

- Both sensors read cleanly and independently over Serial with no network
  involved (Phase A).
- A plain Modbus read from a laptop shows both live sensor signals, and a
  Modbus write to `ZONE_DISABLE` forces them false only while comms are
  live (Phase B).
- OpenPLC and Node-RED both show the same live state (Phase C).
- `OBJECT_RANGE_THRESHOLD_CM` (50cm placeholder in the firmware) has been
  recalibrated against the actual printed perimeter geometry — not left
  for the night before the demo.

At that point this system's Build_Log checklists (PIR, HC-SR04,
ESP32_Core_Modbus) should all be checked off, and
`Firmware/BSH-Perimeter/SYSTEM_REPORT.md` can be written per
`Firmware/WORKFLOW.md` Section 4. All five BSH systems are then done, and
the Founder's Hall mirrors begin with FH-FireAlarm.
