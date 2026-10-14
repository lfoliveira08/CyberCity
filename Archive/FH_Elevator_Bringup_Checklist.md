# FH-Elevator — Bring-Up Checklist

**Purpose:** Founder's Hall mirror of BSH-Elevator — second FH
system per `Firmware/WORKFLOW.md`'s Phase 2 order. Identical hardware, wiring, and logic to BSH-Elevator; only
the static IP should differ.

Firmware for this node: `FH_Elevator_Node.ino` (same folder as this file,
once Phase B starts).

> **⚠ STOP — IP address bug, fix before flashing.** As of this writing,
> `FH_Elevator_Node.ino` has `IPAddress ip(192, 168, 1, 11);` hardcoded —
> that's **BSH-Elevator's** address, not this node's own documented
> 192.168.1.21 (`FH_Systems/01_FH-Elevator.md`). If flashed as-is, this
> node would collide on the network with BSH-Elevator — the same class of
> bug already caught and fixed for the Door pair (see
> `FH_Door_Node.ino`'s header) and separately flagged for FH-PowerBox
> above. Change the `IPAddress ip(...)` line to
> `IPAddress ip(192, 168, 1, 21);` before Phase B (irrelevant for Phase A,
> which never touches WiFi). Worth fixing the Claude project's copy of
> this file too.

Same rewrite note as BSH-Elevator applies here: this is not the old
standalone `Elevator_Motor.ino` prototype. It drives movement as a
non-blocking state machine and drops the 7-segment display, which isn't in
this node's documented BOM either.

---

## 1. Parts Needed

Identical BOM to BSH-Elevator — see
`Firmware/BSH_Elevator_Bringup_Checklist.md` Section 1: ESP32 DevKit, L293D
motor driver + small DC motor, 3× momentary push buttons, 16x2 I2C LCD,
external motor power supply, breadboard/jumpers, USB cable with data
wires, shared project Wi-Fi AP.

---

## 2. Wiring

Identical pin map to BSH-Elevator — see `FH_Sensors/01_L293D_Motor_Driver.md`,
`02_Push_Button.md`, `03_I2C_LCD_Display.md`.

| Component | ESP32 Pin | Notes |
|---|---|---|
| Motor ENABLE (PWM) | GPIO 18 | |
| Motor DIRA | GPIO 19 | |
| Motor DIRB | GPIO 5 | |
| Floor button 1 | GPIO 4 | `INPUT_PULLUP`, active LOW |
| Floor button 2 | GPIO 16 | `INPUT_PULLUP`, active LOW |
| Floor button 3 | GPIO 17 | `INPUT_PULLUP`, active LOW |
| LCD SDA | GPIO 21 | I2C, address `0x27` in firmware |
| LCD SCL | GPIO 22 | |

No dividers or external pull-ups needed, same as BSH-Elevator.

---

## 3. Arduino IDE Setup

Same as BSH-Elevator: ESP32 board support, board = **ESP32 Dev Module**,
**ModbusIP_ESP8266**, **LiquidCrystal_I2C**. Once Phase B starts, open
`FH_Elevator_Node.ino`, **fix the IP bug flagged above**, and fill in real
`ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-Elevator/Wokwi/ (FH mirrors BSH — copy it to `Firmware/FH-Elevator/Wokwi/`)` on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: 3 floor buttons, I2C LCD, L293D/motor (LED stand-ins)
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Same build order as BSH-Elevator (the IP bug above doesn't matter yet):

1. **Push buttons** — confirm all 3 read correctly.
2. **I2C LCD** — confirm it responds at `0x27` (scan if not).
3. **L293D motor driver** — external supply, wire and test last.
4. **Combined standalone integration sketch** — non-blocking state machine
   (door-closing delay → moving → arrived) reproduced without WiFi/Modbus.
5. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. **Fix the IP bug first** (see the callout above), then fill in real
   Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.1.21`
   — if it still prints `.11`, the fix in step 1 wasn't saved before
   flashing.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read IR0 (`CURRENT_FLOOR`) — reads 1 at boot.
   - Write Coil 0/1/2 (`CALL_FLOOR_1/2/3`) — cab moves, `ELEVATOR_MOVING`
     (DI0) tracks it, `CURRENT_FLOOR` updates on arrival.
   - Write Coil 3 (`ELEVATOR_DISABLE`) = 1 — both local and remote calls
     are ignored.
   - Stall the motor mid-move — `ELEVATOR_FAULT` (DI1) latches and stays
     latched, distinguishable from `ELEVATOR_DISABLE`.
4. With both BSH-Elevator and FH-Elevator up simultaneously **and both on
   their correct, distinct IPs**, confirm a Modbus client can reach and
   control each independently — direct proof the IP bug above is actually
   fixed.

---

## 6. Phase C — OpenPLC + SCADA Integration

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-Elevator` — its own Docker container at 192.168.1.121,
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.1.21, port 502). Set discrete input
   count = 2, coil count = 4, input register count = 1.
2. Write the same minimal ladder as BSH-Elevator.
3. Confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a floor-call panel and status tile to the shared Node-RED
   dashboard, next to BSH-Elevator's.

---

## 7. Success Criteria — This System Is Done When:

- The IP bug is fixed and confirmed on real hardware (Serial prints
  `.21`, not `.11`).
- All three buttons, the LCD, and the motor work together as one
  non-blocking state machine with zero network dependency (Phase A).
- A plain Modbus write to this node's own IP moves the cab, and a stalled
  motor latches a genuine fault distinguishable from `ELEVATOR_DISABLE`
  (Phase B).
- OpenPLC and Node-RED both show this node's live state, distinguishable
  side-by-side from BSH-Elevator's (Phase C).

At that point this pattern moves on to FH-Door, per
`Firmware/WORKFLOW.md`'s Phase 2 order.
