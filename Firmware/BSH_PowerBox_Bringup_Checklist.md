# BSH-PowerBox — Bring-Up Checklist

**Purpose:** fourth BSH system in the build order (after Fire Alarm,
Elevator, and Door — see `Firmware/WORKFLOW.md`). No longer flagged
higher-risk than Fire Alarm — DALI was dropped Sep 28, 2026 in favor of a
plain relay for lighting, the same pattern already proven on Fire Alarm and
Elevator. Local override buttons and status feedback added Sep 29, 2026.
Switched from a single 2-channel relay module to two separate 1-channel
relay modules Sep 30, 2026 -- see Section 1 and 2 below. Phase A
(standalone bench test) completed Oct 5, 2026 -- see Section 4 below.

Firmware for this node: `BSH_PowerBox_Node.ino` (same folder as this file,
once Phase B starts). Until then, per `Firmware/WORKFLOW.md` Section 5,
Phase A uses a standalone no-WiFi/no-Modbus integration sketch, same
pattern as `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.

---

## 1. Parts Needed

| Qty | Part | Notes |
|---|---|---|
| 1 | ESP32 DevKit board | Any common DevKitC/DOIT-style board |
| 2 | 1-channel relay module | One per channel -- lights, fan. Swapped from a single 2-channel module Sep 30, 2026; each board gets its own VCC/GND. Both confirmed active-HIGH Oct 5, 2026 -- see polarity note below, `BSH_Sensors/09_Relay_Module.md` |
| 3 | LED (one per floor) | All 3 wired in parallel on the lights relay channel, each with its own series resistor — see `Firmware/BSH-PowerBox/Lights_Relay/Wiring.md` |
| 1 | Small fan (DC motor + fan blade, or stand-in) | Driven through the fan relay channel |
| 2 | Momentary push button | Local lights/fan override switches, see `BSH_Sensors/02_Push_Button.md` |
| 1 | Breadboard + jumper wires | |
| 1 | USB cable **with data wires** | |
| — | Dedicated project Wi-Fi AP, already broadcasting | Needed before Phase B |

---

## 2. Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| Lights relay | GPIO 13 | Its own 1-channel relay module -- own VCC/GND, see active-LOW note below |
| Fan relay | GPIO 15 | A separate 1-channel relay module from lights -- own VCC/GND, see active-LOW note below |
| Lights override button | GPIO 4 | `INPUT_PULLUP`, active LOW — wire the other leg to GND, no external resistor needed |
| Fan override button | GPIO 16 | Same as above |

**Relay polarity -- confirmed Oct 5, 2026:** both boards on this node
turned out active-HIGH. `RELAY_LIGHTS_ACTIVE_LOW = false` and
`RELAY_FAN_ACTIVE_LOW = false` in `BSH_PowerBox_Node.ino`, verified via
the Phase A Step 4 combined standalone sketch (Section 4). Each board was
checked independently -- don't assume this result for FH-PowerBox's
boards, which still need their own run (`FH_PowerBox_Bringup_Checklist.md`
Section 4).

---

## 3. Arduino IDE Setup

1. Install ESP32 board support (Boards Manager → "esp32").
2. Select board: **ESP32 Dev Module**.
3. Libraries (Phase B/C):
   - **ModbusIP_ESP8266**
4. Once Phase B starts: open `BSH_PowerBox_Node.ino` and fill in your real
   `ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [x] Open the simulation from `Firmware/BSH-PowerBox/Wokwi/` on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [x] Every part responds: lights relay, fan relay, lights button, fan button
- [x] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Relay polarity" and "Real bench differences"
      sections, then logged the run (date + anything surprising) in each
      component's `Build_Log.md`

**Phase A0 complete (Sep 29, 2026).** Build passed after fixing the
`ButtonState`-struct compile error (see `Piranga_Workflow_v2.md` Section 14),
and both relays plus both buttons behave correctly with
`RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` set active-HIGH. The
`Build_Log.md` entries are still open — worth filling in before starting
Phase A below, so the "what we learned in Wokwi" record doesn't get lost.

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

**Phase A complete (Oct 5, 2026).** In practice Steps 1-3 ended up
confirmed together as part of the Step 4 combined sketch rather than as
separate standalone runs -- see `Fan_Relay/Build_Log.md` and
`Lights_Relay/Build_Log.md` for the full account, including an earlier
attempt that flashed the wrong (older, lights-only) sketch by mistake,
which looked like dead fan/button hardware until the right sketch was
re-flashed.

1. **Fan relay alone** — plain digital output, confirm the fan turns
   on/off cleanly and check the active-LOW/active-HIGH polarity against
   your specific module. A Sep 30, 2026 test on the *original* 2-channel
   module found that module's fan channel active-HIGH, but that module
   was retired the same day. **Retested on the new 1-channel fan relay
   board and confirmed Oct 5, 2026: also active-HIGH**
   (`RELAY_FAN_ACTIVE_LOW = false`) -- see `Fan_Relay/Build_Log.md`.
2. **Lights relay alone** — same check on the (now separate) lights
   relay module. **Confirmed Oct 5, 2026: active-HIGH**
   (`RELAY_LIGHTS_ACTIVE_LOW = false`) -- see `Lights_Relay/Build_Log.md`.
   Don't assume it shares the fan relay's polarity result just because
   both landed the same way here -- it's a different physical board and
   was verified on its own.
3. **Buttons alone** — confirm each button reads LOW when pressed
   (`INPUT_PULLUP`) and debounces cleanly (no double-toggle on a single
   press). **Confirmed Oct 5, 2026** as part of Step 4: each button
   toggles only its own channel, matching the Serial log.
4. **Combined standalone integration sketch:** once the relays and buttons
   each work individually, build one no-WiFi/no-Modbus sketch that drives
   both relays together and reads both buttons, printing state to Serial.
   Model it on `Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`.
   **Confirmed working end-to-end Oct 5, 2026:** both relays respond
   correctly to Serial `l`/`f` commands, both override buttons toggle
   only their own channel, and the Serial-printed state matches physical
   reality on every transition.
5. If anything here doesn't work, it's a wiring/hardware issue — fix it
   before moving to Phase B. (One exception found this pass: a
   no-Serial-output symptom that looked like dead hardware turned out to
   be the wrong sketch flashed -- confirm the flashed code first.)

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials in `BSH_PowerBox_Node.ino` and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.14`.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Write Coil 0 (`LIGHTS_ON_CMD`) = 1 — the light should turn on, and
     Discrete Input 0 (`LIGHTS_STATUS`) should read back 1.
   - Write Coil 1 (`FAN_CMD`) = 1/0 — fan should follow directly, and
     Discrete Input 1 (`FAN_STATUS`) should track it.
   - Press the lights button on the bench — the light should flip
     immediately and `LIGHTS_STATUS` should update to match, **without**
     you writing to `LIGHTS_ON_CMD` at all.
   - With the light on from the button, write `LIGHTS_ON_CMD` = 0 from the
     client — the hub should regain authority and turn it back off (this
     is the "fresh coil write" edge, not the coil's unchanged last value).
   - Disconnect Wi-Fi (or otherwise simulate comm loss) with lights/fan on
     — confirm both default to OFF once the watchdog timeout elapses, but
     that the buttons still work locally even while Wi-Fi is down. A
     dark, idle room is this node's safe default and also makes a dropped node
     visually obvious during a resilience demo.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 0 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 1 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/BSH-PowerBox/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-BSH-PowerBox` — its own Docker container at 192.168.50.114 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.14, port 502). Set coil count = 2, discrete input
   count = 2, input register count = 2 (IR0 `HEARTBEAT`, IR1 `RECONNECT_COUNT`).
2. Write the minimal ladder: pass SCADA-writable internal coils through to
   `Coil0`/`Coil1`, and surface `DI0`/`DI1` read-only on the HMI.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values,
   including a case where a button press has made `DI0`/`DI1` disagree
   with the last coil write.
4. Add lights and fan tiles to the SCADA test dashboard (Ignition), showing both
   the last command and the actual reported state.

---

## 7. Success Criteria — This System Is Done When:

- Both relay outputs and both buttons respond correctly and independently
  with zero network dependency (Phase A). **Met Oct 5, 2026.**
- A plain Modbus write changes both outputs, both outputs fail safely to
  OFF on comm loss, a local button changes the physical state and is
  correctly reflected in `LIGHTS_STATUS`/`FAN_STATUS`, and a fresh hub
  command correctly regains authority over a button-set state (Phase B).
- OpenPLC and the SCADA dashboard both show the same live state, including any
  command-vs-actual mismatch caused by a button press (Phase C).

At that point all four BSH systems are done and this pattern moves on to Founder's Hall (Phase 2), per the project's
chosen build order (Backup Power was dropped Oct 5, 2026).
