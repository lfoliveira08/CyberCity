# BSH-FireAlarm Pilot — Bring-Up Checklist

**Purpose:** this is the very first hardware/firmware milestone for Automated Cyber-Physical Campus System Design. The goal is
*not* to finish the Fire Alarm system — it's to prove the whole chain (ESP32 → OpenPLC →
SCADA) works at all, with the least hardware complexity of any of the project's nodes, before
committing time to the others.

Firmware for this node: `BSH_FireAlarm_Node.ino` (same folder as this file).

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 1 | Active buzzer | 3.3V/5V digital drive — see `BSH_Sensors/04_Active_Buzzer.md` |
| 1 | Common-cathode RGB LED (or 3 discrete LEDs) | See `BSH_Sensors/05_RGB_LED.md` |
| 3 | 220Ω resistors | One per RGB LED channel (current limiting) |
| 1 | Momentary push button | Pull-station stand-in — `BSH_Sensors/02_Push_Button.md` |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | Charge-only cables won't program the board — see Common Pitfall #1 in the skill guide if the board doesn't show up in Device Manager |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B below |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Buzzer signal | GPIO 23 | Straight digital drive, no resistor needed |
| Buzzer GND | GND | |
| RGB LED — Red | GPIO 25 | Through a 220Ω resistor |
| RGB LED — Green | GPIO 26 | Through a 220Ω resistor |
| RGB LED — Blue | GPIO 27 | Through a 220Ω resistor |
| RGB LED — common cathode | GND | |
| Push button — one leg | GPIO 4 | Other leg to GND. No external resistor needed — firmware uses `INPUT_PULLUP` |

All four GPIOs used here (4, 23, 25, 26, 27) are on the ESP32's safe list — none of them touch the boot-strapping pins (0, 1, 2, 3, 6–11).

---

## 3. Arduino IDE Setup

1. Install ESP32 board support in Arduino IDE (Boards Manager → search "esp32" → install).
2. Select board: **ESP32 Dev Module**.
3. Install the **ModbusIP_ESP8266** library via Library Manager (works on ESP32 despite the name).
4. Open `BSH_FireAlarm_Node.ino` and fill in your real `ssid` / `password` — keep the real values in a local, non-shared copy; leave placeholders in anything that goes to the Drive folder or git.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

The firmware is written so the **local pull-station path works independently of the
network** — even if Wi-Fi fails to connect, the button still drives the buzzer/LED. Use
this to isolate hardware problems from network problems.

1. Flash the sketch as-is (Wi-Fi credentials can even be left as placeholders for this phase).
2. Open the Serial Monitor at 115200 baud.
3. Press and hold the button — buzzer should sound and the RGB LED should blink red.
4. Release — buzzer stops, LED returns to steady green.
5. If this doesn't work, it's a wiring/hardware issue — fix it here before moving on. Don't debug hardware and network problems at the same time.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

Now confirm the ESP32 is a correctly-behaving Modbus TCP slave, using a plain Modbus
client tool instead of OpenPLC — this isolates "is my firmware a valid Modbus slave"
from "is OpenPLC configured correctly," which is a much easier problem to debug alone.

1. Fill in the real Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.12`.
3. From a laptop on the same Wi-Fi/subnet, use a free Modbus TCP client — `modpoll` (CLI) or QModMaster (GUI) both work:
   - Read Discrete Input 0 (`MANUAL_PULL_TRIGGERED`) — should read 0 normally, 1 while the button is held.
   - Write Coil 0 (`ALARM_ACTIVATE`) = 1 — buzzer should sound and LED should blink red, exactly like the button does.
   - Write Coil 0 = 0 — alarm should stop (assuming the button isn't also pressed).
4. This step is also your first real demonstration of Scenario 1 in miniature: an unauthenticated Modbus write drives a physical output, with nothing checking who sent it.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 1 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 2 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/BSH-FireAlarm/` (untested until Phase C).

Only move here once Phase B works cleanly.

1. Install OpenPLC Runtime on the hub machine if not already done.
2. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC instance (`OpenPLC-BSH-FireAlarm` — its own Docker container at 192.168.50.112 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.12, port 502). Set discrete input count = 3, input register count = 3, coil count = 1.
3. Write the minimal ladder rung: pass a SCADA-writable internal coil straight through to `Coil0`, and expose `DI0` for monitoring — this is the whole ladder program for this node at this stage.
4. Confirm OpenPLC's own Monitoring page shows the same live values you saw in Phase B, including `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or stuck at 0, the Input Register count in step 2 is too low -- it must be 3).
5. Point one Ignition Modbus TCP device connection at this OpenPLC instance and put a single status tile on a test dashboard.

---

## 7. Success Criteria — This Pilot Is Done When:

- The button triggers the alarm locally with zero network dependency (Phase A).
- A plain Modbus write from a laptop triggers the alarm remotely (Phase B).
- OpenPLC shows the same state, live (Phase C).
- The SCADA dashboard (Ignition) shows the same state, live (Phase C).

At that point the architecture itself is proven end-to-end, and building the remaining
four BSH systems (then the five FH mirrors) is repeating a working pattern rather than
debugging an unknown one. The two highest-risk nodes — Door (Wiegand) and Power Box
(DALI) — are worth starting to prototype in parallel with this pilot rather than after,
since they're the two pieces of firmware with the least precedent in this project.
