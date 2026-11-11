# BSH-PowerBox — Bring-Up Checklist

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

**Redesigned Oct 28, 2026.** This node no longer drives lights and a fan — it switches power to the other three BSH systems (Elevator, FireAlarm, Door). Built and bench-tested first as a prototype on a spare ESP32 (`Prototype_PowerRails/`), then adopted into this path the same day. The old checklist (lights + fan, Phase A complete Oct 5, 2026) described the design now archived — see `Archive/README.md`.

Firmware for this node: `BSH_PowerBox_Node.ino` (same folder as this file).

---

## 1. Parts needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | The former spare board, on a breadboard (not the original lights-and-fan board, which is set aside). Pin count and exact model not recorded |
| 3 | 1-channel 5V relay module | One per rail — Elevator, FireAlarm, Door. Songle SRD-05VDC-SL-C type reported on the bench; test each board's polarity independently |
| — | No LEDs, no fan, no push buttons | dropped Oct 28, 2026 |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires**, to this PowerBox board | the three downstream nodes' own USB cables are **unplugged** while powered through their relay instead |
| — | Dedicated project Wi-Fi AP, already broadcasting | |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Elevator rail relay | GPIO 25 | IN; relay VCC/GND from this board's own 5V/GND |
| FireAlarm rail relay | GPIO 26 | same |
| Door rail relay | GPIO 27 | same |

See `Schematic/Connections.md` for the full as-built wiring, including the relay contact side (COM/NO) and how each downstream node is fed. **Relay polarity — not yet confirmed per board on this build; each board must be checked independently, same rule as every relay on this project** (`BSH_Sensors/09_Relay_Module.md`).

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries: **ModbusIP_ESP8266**.
4. Open `BSH_PowerBox_Node.ino`. The WiFi name and password are placeholders in this repo copy — fill in the real values in your own copy before flashing, never commit them.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7.

- [ ] Open the simulation from `Firmware/BSH-PowerBox/Wokwi/` on wokwi.com (new ESP32 project → paste `sketch.ino` and `diagram.json`) and press play
- [ ] All three rail relays respond to `1`/`2`/`3`, and `a`/`r` cut/restore all three
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Logged the run (date + anything surprising) in `ESP32_Core_Modbus/Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — current draw, relay contact rating, and the DevKit's VIN-path limit still get checked on the bench.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

**Not formally run as a standalone Phase A** — this build went straight to a combined bench test with WiFi and Modbus (see Phase B below and `Prototype_PowerRails/` for the prototype bench record). The checks below are still worth doing if the hardware is rebuilt:

1. **Each relay alone** — confirm it clicks on and off cleanly, and check the active-LOW/active-HIGH polarity against the specific module (`setRelay()`, `RELAY_ACTIVE_LOW` in the sketch). **Not individually confirmed per board on this build.**
2. **Contact check with a multimeter** — continuity across COM/NO with the relay commanded on, and open with it off. **Done once, on one relay board, during the prototype phase** (`Logs/2026-10-28.md`); not repeated for all three on the final build.
3. If anything here doesn't work, it's a wiring/hardware issue — fix it before moving on.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials in your own copy of `BSH_PowerBox_Node.ino` and flash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.14`. **Done** (Luis, Oct 28, 2026 — pasted Serial output: `wifi=up`, heartbeat counting).
3. From a laptop or the Kali VM on the same Wi-Fi/subnet, use a Modbus TCP client (`modbus_coil_tool.py`, standard library only):
   - Write Coil 0 (`ELEV_RAIL_CMD`) = 1 — the Elevator rail should power on, and Discrete Input 0 (`ELEV_RAIL_STATUS`) should read back 1. **Done via the PLC's own coils on container .114** (see Phase C below), not yet tested writing this ESP32's coils directly.
   - Disconnect Wi-Fi (or otherwise simulate comm loss) with a rail on — confirm all three rails default to OFF once the watchdog timeout (5 s) elapses. **Not yet confirmed with a measured timing; the rail did reset to off during bench testing but the cause (WiFi drop vs. a PLC restart) was not pinned down** (`Logs/2026-10-28.md`).
4. **Connection-monitor check:** read Input Register 0 (`HEARTBEAT`) twice a few seconds apart — it must increase by about 1 per second. **Done** (reported counting). Read Input Register 1 (`RECONNECT_COUNT`) — must read 0 right after boot. Switch WiFi off ~15 s and back on: heartbeat resumes, `RECONNECT_COUNT` reads 1. Press reset: both registers go back to 0. **Not yet run as a deliberate, timed test.**
5. **Current and voltage, not yet done:**
   - [ ] Measure current draw of each downstream node through its relay
   - [ ] Measure current drawn by this PowerBox board itself (its own ESP32 + 3 relay coils, about 210 mA for the coils alone) from its USB source
   - [ ] Measure voltage at a downstream node's power pins with its rail commanded on, to confirm power is actually reaching it (not just that the relay's coil/LED is energised)

---

## 6. Phase C — OpenPLC + SCADA Integration

> Ladder diagram, register map and program for this node: `PLC_Logic/BSH-PowerBox/`.

1. Slave device on container 192.168.50.114: **Generic Modbus TCP Device**, IP = this ESP32's (192.168.50.14), slave ID 0, port 502. Coil count = 3, discrete input count = 3 (were 2 each), input register count = 2 (unchanged). **Done** (Oct 28, 2026 — Slave Devices page confirmed `%IX100.0 to %IX100.2`, `%QX100.0 to %QX100.2`, `%IW100 to %IW101`).
2. Ladder: 8 rungs, built in the OpenPLC Editor project `BSH-PowerBox-ESP32/` (edited in place, keeping the project's history) — 3 command forwards, 3 status publishes, heartbeat and reconnect count. **Done** (Oct 28, 2026, 18:25 — checked against the saved Editor project; `Program.st` regenerated from it).
3. Upload and compile `Program.st` on container .114. **Done, reported clean compile** (no compile log text captured yet).
4. Confirm OpenPLC's Monitoring page matches live values, including the heartbeat counting. **Done** (reported).
5. On-command test: write a SCADA coil (`%QX0.0` etc.) from a Modbus client and watch the rail power through the real chain. **Confirmed once, Elevator rail, via `modbus_coil_tool.py`** — coil 0 and coil 3 both read back 1 after the write, and the relay's onboard LED lit. FireAlarm and Door rails **not yet individually confirmed this way**.
6. SCADA (Ignition) tags and screen for this node: **not started** (adoption Step 5).

---

## 7. Success Criteria — This System Is Done When:

- [ ] All three relay outputs respond correctly and independently, with each board's polarity confirmed on the bench (not yet — only partially tested)
- [x] A plain Modbus write through the PLC changes a rail's state and the status bit reads back correctly (confirmed once, Elevator, Oct 28, 2026)
- [ ] All three rails confirmed this way, not just Elevator
- [ ] All three rails fail to OFF on a WiFi-link loss of about 5 s, confirmed with timing and a known cause (not yet — the one observed reset to OFF was not root-caused)
- [ ] Current measured for each downstream node and for this PowerBox board itself
- [ ] Voltage measured at a downstream node's power pins with its rail on
- [ ] OpenPLC and the SCADA dashboard both show the same live state (SCADA/Ignition tags for this node not yet built)
- [ ] The system doc, this checklist, the PLC docs and the SCADA docs all agree (in progress — this is adoption Step 3)

This list replaces the old Phase A/B/C checklist above it (lights + fan), which is historical; do not re-run those steps against the new design.
