# BSH-Elevator — Bring-Up Checklist

> **Update 2026-10-07: Phase B complete** -- flashed and bench-confirmed at 192.168.50.11 (see `BSH-Elevator/ESP32_Core_Modbus/Build_Log.md`). The display is an Adafruit SSD1306 OLED (libraries Adafruit SSD1306 + GFX), not the I2C LCD named further down. Phase C not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

**Purpose:** second BSH system in the build order (right after the Fire
Alarm pilot — see `Firmware/WORKFLOW.md`). It's the lowest firmware risk of
the remaining nodes (no field-bus protocol, just GPIO/I2C), so it's a good
second step before the higher-risk Door (RFID) node. (Power Box was also
once higher-risk for its DALI library, but DALI was retired Sep 28, 2026 in
favor of a plain relay.)

Firmware for this node: `BSH_Elevator_Node.ino` (same folder as this file,
once Phase B starts). Until then, per `Firmware/WORKFLOW.md` Section 5,
Phase A uses a standalone no-WiFi/no-Modbus integration sketch, same
pattern as `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.

**Important — this firmware is a rewrite, not the original prototype.** If
you built anything from the original standalone `Elevator_Motor.ino`
prototype (no networking, no Modbus, a 7-segment display), that's not what
`BSH_Elevator_Node.ino` uses. The rewrite keeps the same motor/button/LCD
wiring role but drives movement as a non-blocking state machine instead of
`delay()` (the old prototype's multi-second blocking calls would freeze
`mb.task()` long enough to look like a dead Modbus slave), and drops the
7-segment display entirely — it's not in this node's documented BOM. If you
already wired one from the old prototype, it's fine to leave it unwired.

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 1 | L293D motor driver + small DC motor | Cab movement — see `BSH_Sensors/01_L293D_Motor_Driver.md` |
| 3 | Momentary push buttons | Floor-call buttons 1/2/3 — `BSH_Sensors/02_Push_Button.md` |
| 1 | 16x2 I2C LCD display | Status text — `BSH_Sensors/03_I2C_LCD_Display.md` |
| 1 | External motor power supply | Don't power the motor from the ESP32's own rail |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B |

No 7-segment display, no field-bus transceiver — this is the simplest
remaining BOM in the project.

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Motor ENABLE (PWM) | GPIO 18 | LEDC PWM output |
| Motor DIRA | GPIO 19 | |
| Motor DIRB | GPIO 5 | |
| Floor button 1 | GPIO 4 | `INPUT_PULLUP`, active LOW — no external resistor needed |
| Floor button 2 | GPIO 16 | `INPUT_PULLUP`, active LOW |
| Floor button 3 | GPIO 17 | `INPUT_PULLUP`, active LOW |
| LCD SDA | GPIO 21 | I2C |
| LCD SCL | GPIO 22 | I2C |

No voltage dividers or external pull-ups needed on this node — all three
buttons use the ESP32's internal `INPUT_PULLUP` (none of them are on the
input-only ADC pins 34-39 that lack it), and the LCD's I2C address is
`0x27` in the firmware — confirm that matches your specific module (some
ship at `0x3F`) with an I2C scanner sketch if the display stays blank.

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries (Phase B/C):
   - **ModbusIP_ESP8266**
   - **LiquidCrystal_I2C**
4. Once Phase B starts: open `BSH_Elevator_Node.ino` and fill in your real
   `ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-Elevator/Wokwi/` on wokwi.com (new ESP32 project →
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

Build order for this node:

1. **Push buttons** — simplest, get all 3 reading correctly first
   (confirm each pulls LOW when pressed).
2. **I2C LCD** — confirms the I2C bus works before adding the motor
   driver's own wiring nearby; run an I2C scanner if it doesn't respond at
   `0x27`.
3. **L293D motor driver** — wire and bench-test last among the three,
   since a mis-wired motor driver is the easiest of the three to
   accidentally short something on. Use an external motor supply, not the
   ESP32's own rail.
4. **Combined standalone integration sketch:** once all three are
   individually confirmed, build one no-WiFi/no-Modbus sketch that
   reproduces the real firmware's state machine (door-closing delay →
   moving → arrived) driven by `millis()`, not `delay()` — pressing a
   floor button should move the cab and update the LCD without ever
   freezing. Model it on
   `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`'s
   non-blocking structure.
5. If anything here doesn't work, it's a wiring/hardware issue — fix it
   before moving to Phase B.

**Phase A status: complete (2026-09-25).** All 3 buttons, the OLED status
display (substituted for the documented 16x2 LCD — see
`I2C_LCD_Display/Wiring.md`), and the L293D/motor all bench-tested
individually and then confirmed together in one combined non-blocking
integration sketch: `BSH_Elevator_Node/BSH_Elevator_Node.ino`. Full writeup:
`Firmware/BSH-Elevator/SYSTEM_REPORT.md`.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials in `BSH_Elevator_Node.ino` and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.11`.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read Input Register 0 (`CURRENT_FLOOR`) — should read 1 at boot.
   - Write Coil 0/1/2 (`CALL_FLOOR_1/2/3`) momentarily — the cab should
     move, `ELEVATOR_MOVING` (DI0) should go true during travel, and
     `CURRENT_FLOOR` should update on arrival. These coils self-clear —
     you shouldn't need to write them back to 0.
   - Write Coil 3 (`ELEVATOR_DISABLE`) = 1, then try both a local button
     press and a remote floor call — both should be ignored while disabled.
   - To exercise `ELEVATOR_FAULT` (DI1), physically stall the motor/cab
     during a move (or briefly disconnect it) and confirm the fault
     latches and stays latched — it does not self-clear like
     `ELEVATOR_DISABLE` does, representing "needs a technician." Reset the
     board to clear it.
4. Confirm `ELEVATOR_FAULT` and `ELEVATOR_DISABLE` stay visibly
   distinguishable on your test client — a remotely-toggled disable should
   never look identical to a genuine stall.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 1 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 2 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/BSH-Elevator/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-BSH-Elevator` — its own Docker container at 192.168.50.111 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.11, port 502). Set discrete input
   count = 2, coil count = 4, input register count = 3 (IR0 `CURRENT_FLOOR`, IR1 `HEARTBEAT`, IR2 `RECONNECT_COUNT` -- a count of 1 would hide the heartbeat from OpenPLC).
2. Write the minimal ladder: expose `DI0`/`DI1`/`IR0`-`IR2` for monitoring, pass
   SCADA-writable internal coils through to `Coil0`-`Coil3`.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a floor-call panel and status tile to the SCADA test dashboard (Ignition).

---

## 7. Success Criteria — This System Is Done When:

- All three buttons, the LCD, and the motor work together as one
  non-blocking state machine with zero network dependency (Phase A).
- Remote floor calls move the cab exactly like local buttons do; a stalled
  motor latches a genuine, non-self-clearing fault; `ELEVATOR_DISABLE`
  stays remotely toggleable and distinguishable from that fault (Phase B).
- OpenPLC and the SCADA dashboard both show the same live state (Phase C).

At that point this pattern moves on to Door, per the project's chosen
build order — the highest-risk of the five BSH systems.
