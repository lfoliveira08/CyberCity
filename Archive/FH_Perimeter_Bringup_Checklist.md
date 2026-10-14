# FH-Perimeter — Bring-Up Checklist

**Purpose:** Founder's Hall mirror of BSH-Perimeter — fifth and final FH
system in the build order, and the last of all ten project nodes. Identical hardware, wiring, and logic to BSH-Perimeter;
only the static IP differs.

Firmware for this node: `FH_Perimeter_Node.ino` (same folder as this file,
once Phase B starts) — confirmed identical to `BSH_Perimeter_Node.ino`
except its IP. Until Phase B, use a standalone no-WiFi/no-Modbus
integration sketch — you can likely reuse BSH-Perimeter's combined Phase A
sketch unmodified, since the hardware and GPIO map are identical.

---

## 1. Parts Needed

Identical BOM to BSH-Perimeter: ESP32 DevKit, PIR motion sensor, HC-SR04
ultrasonic sensor, 1kΩ + 2kΩ resistors (ECHO divider), breadboard/jumpers,
a USB cable with data wires, and the shared project Wi-Fi AP. No local
actuator — sensor-only node, same as BSH-Perimeter.

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| PIR OUT | GPIO 32 | Direct digital input |
| PIR VCC | 5V | |
| PIR GND | GND | |
| Ultrasonic TRIG | GPIO 14 | Direct, 3.3V-safe |
| Ultrasonic ECHO | GPIO 27 | **Through the 1kΩ/2kΩ divider — same hazard as BSH-Perimeter** |
| Ultrasonic VCC | 5V | |
| Ultrasonic GND | GND | Shared with the divider's ground leg |

Same ECHO-is-5V hazard as BSH-Perimeter: build and multimeter-verify the
divider (~3.3V at the GPIO side) before ever connecting it. Same PIR
trigger-mode jumper note: set to repeatable (H).

---

## 3. Arduino IDE Setup

Same as BSH-Perimeter: ESP32 board support, board = **ESP32 Dev Module**,
**ModbusIP_ESP8266** (Phase B/C only). Once Phase B starts, open
`FH_Perimeter_Node.ino` and fill in real `ssid`/`password` — this node's
own copy, separate from BSH-Perimeter's.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-Perimeter/Wokwi/ (FH mirrors BSH — copy it to `Firmware/FH-Perimeter/Wokwi/`)` on wokwi.com (new ESP32 project →
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

1. **PIR alone** — same bench test as BSH-Perimeter: confirm a clean,
   repeatable trigger after the ~30-60s settle time.
2. **Ultrasonic alone** — divider built and multimeter-verified first,
   then confirm sane distance readings and a `9999` "no echo" sentinel
   when nothing is in range.
3. **Combined standalone integration sketch** — both sensors together,
   printed to Serial, no WiFi/Modbus.
4. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.1.25`
   — **not** `.15` (BSH-Perimeter's address).
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read DI0 (`MOTION_DETECTED`), DI1 (`OBJECT_IN_RANGE`), IR0
     (`DISTANCE_CM`) — should track both sensors live.
   - Write Coil 0 (`ZONE_DISABLE`) = 1 — both discrete inputs force false
     while comms stay live; `DISTANCE_CM` keeps reporting real readings.
   - Disconnect Wi-Fi with `ZONE_DISABLE` still set — confirm the
     discrete inputs go back to reflecting the raw sensors (same
     fail-toward-"still watching" behavior as BSH-Perimeter).
4. With both BSH-Perimeter and FH-Perimeter up simultaneously, confirm
   they report independently.

---

## 6. Phase C — OpenPLC + SCADA Integration

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-Perimeter` — its own Docker container at 192.168.1.125,
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.1.25, port 502). Set discrete input
   count = 2, input register count = 1, coil count = 1.
2. Write the same minimal ladder as BSH-Perimeter.
3. Confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a status tile to the shared Node-RED dashboard, next to
   BSH-Perimeter's.

---

## 7. Success Criteria — This System Is Done When:

- Both sensors read cleanly and independently over Serial with zero
  network dependency (Phase A).
- Modbus confirms the correct IP (192.168.1.25) and the same
  fail-toward-enabled behavior as BSH-Perimeter (Phase B).
- OpenPLC and Node-RED both show this node's live state (Phase C).
- `OBJECT_RANGE_THRESHOLD_CM` recalibrated against FH's actual printed
  perimeter geometry — it's a separate physical zone from BSH's, so don't
  assume BSH-Perimeter's calibrated value carries over.

At that point all ten project nodes (five BSH, five FH) are built — the
CyberUIW testbed's full sensing/actuation layer is complete, ready for
scenario development and validation.
