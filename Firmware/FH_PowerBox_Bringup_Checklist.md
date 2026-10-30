# FH-PowerBox — Bring-Up Checklist

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

**Redesigned Oct 28, 2026, mirroring `Firmware/BSH_PowerBox_Bringup_Checklist.md`.** This node no longer drives lights and a fan — it switches power to the other three FH systems (FH-HVAC, FH-FireAlarm, FH-Door). **Not yet bench-confirmed on FH's own hardware** — everything below mirrors BSH-PowerBox's design and bench results; each step needs its own confirmation on this node.

Firmware for this node: `FH_PowerBox_Node.ino` (same folder as this file).

---

## 1. Parts needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Not yet decided whether this reuses a spare board (as BSH-PowerBox did) or the original FH lights-and-fan board |
| 3 | 1-channel 5V relay module | One per rail — FH-HVAC, FH-FireAlarm, FH-Door. Test each board's polarity independently, same rule as BSH |
| — | No LEDs, no fan, no push buttons | dropped Oct 28, 2026 |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires**, to this PowerBox board | the three downstream nodes' own USB cables are **unplugged** while powered through their relay instead |
| — | Dedicated project Wi-Fi AP, already broadcasting | |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| FH-HVAC rail relay | GPIO 25 | same pins as BSH-PowerBox |
| FH-FireAlarm rail relay | GPIO 26 | same |
| FH-Door rail relay | GPIO 27 | same |

See `Schematic/Connections.md` (rewritten Oct 28, 2026, draft quality, assumed to mirror BSH-PowerBox's pattern -- not individually confirmed per pin on this node).

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries: **ModbusIP_ESP8266**.
4. Open `FH_PowerBox_Node.ino`. The WiFi name and password are placeholders in this repo copy — fill in the real values in your own copy before flashing, never commit them.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7.

- [ ] Copy `Firmware/BSH-PowerBox/Wokwi/` into `Firmware/FH-PowerBox/Wokwi/` (not yet done — does not exist for this node yet)
- [ ] Open it on wokwi.com, press play
- [ ] All three rail relays respond to `1`/`2`/`3`, and `a`/`r` cut/restore all three
- [ ] Logged the run in `ESP32_Core_Modbus/Build_Log.md`

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Same approach as BSH-PowerBox: no formal standalone Phase A planned — go straight to a combined bench test with WiFi and Modbus (Phase B below).

1. **Each relay alone** — confirm it clicks on and off, check active-LOW/active-HIGH polarity against the specific module. **Not done on FH's own boards.**
2. **Contact check with a multimeter** — continuity across COM/NO, relay on vs. off. **Not done.**
3. If anything here doesn't work, it's a wiring/hardware issue — fix it before moving on.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials in your own copy of `FH_PowerBox_Node.ino` and flash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.24`. **Not done.**
3. From a laptop or the Kali VM, use `Kali/modbus_coil_tool.py` (standard library only, no install needed):
   - Write Coil 0 (`HVAC_RAIL_CMD`) = 1 — the FH-HVAC rail should power on, Discrete Input 0 should read back 1. **Not done.**
   - Disconnect Wi-Fi with a rail on — confirm all three rails default to OFF after about 5 s. **Not done.**
4. **Connection-monitor check:** `HEARTBEAT` counts up about once a second; `RECONNECT_COUNT` reads 0 after boot, 1 after a WiFi drop/rejoin. **Not done.**
5. **Current and voltage, not yet done** (same open items as BSH-PowerBox):
   - [ ] Measure current draw of each downstream node through its relay
   - [ ] Measure current drawn by this PowerBox board itself
   - [ ] Measure voltage at a downstream node's power pins with its rail commanded on

---

## 6. Phase C — OpenPLC + SCADA Integration

> Ladder diagram, register map and program for this node: `PLC_Logic/FH-PowerBox/`.

1. Slave device on container 192.168.50.124: **Generic Modbus TCP Device**, IP = this ESP32's (192.168.50.24), slave ID 0, port 502. Coil count = 3, discrete input count = 3 (were 2 each), input register count = 2 (unchanged). **Not yet done on this container.**
2. Ladder: 8 rungs, same shape as `PLC_Logic/BSH-PowerBox/Ladder_Diagram.md` with HVAC in place of Elevator — **Luis builds these himself in the OpenPLC Editor project `FH-PowerBox-ESP32/`, editing it in place**, same as BSH-PowerBox. **Not started.**
3. Upload and compile `Program.st` (regenerated from the Editor build once it exists) on container .124. **Not started.**
4. On-command test: write a SCADA coil from `Kali/modbus_coil_tool.py` and watch the rail power through the real chain. **Not started.**
5. SCADA (Ignition) tags and screen for this node: **not started.**

---

## 7. Success Criteria — This System Is Done When:

Same list as `Firmware/BSH_PowerBox_Bringup_Checklist.md` Section 7, with FH-HVAC/FH-FireAlarm/FH-Door in place of Elevator/FireAlarm/Door. **Nothing on this list is checked yet for FH-PowerBox.**

This list replaces the old Phase A/B/C checklist (lights + fan) for this node, which is historical; do not re-run those steps against the new design.
