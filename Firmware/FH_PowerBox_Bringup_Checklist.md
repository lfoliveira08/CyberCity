# FH-PowerBox — Bring-Up Checklist

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

**Purpose:** Founder's Hall mirror of BSH-PowerBox — fourth FH system per
`Firmware/WORKFLOW.md`'s Phase 2 order. Identical hardware, wiring, and
logic to BSH-PowerBox; only the static IP should differ.

Firmware for this node: `FH-PowerBox/FH_PowerBox_Node/FH_PowerBox_Node.ino`
(Phase B version, written Oct 6, 2026, static IP 192.168.50.24).

No longer flagged higher-risk than Fire Alarm — DALI was dropped Sep 28,
2026 in favor of a plain relay for lighting, mirroring BSH-PowerBox. Local
override buttons and status feedback added Sep 29, 2026, also mirroring
BSH-PowerBox. Switched from a single 2-channel relay module to two
separate 1-channel relay modules Sep 30, 2026, mirroring BSH-PowerBox --
see Section 1 and 2 below. The IP bug previously flagged here (`FH_PowerBox_Node.ino`
hardcoding BSH-PowerBox's 192.168.50.14) was fixed in the same pass that
rewrote the firmware for the relay-only design — confirm the Serial
Monitor prints `.24`, not `.14`, during Phase B regardless.

**Status (Oct 7, 2026): Phase B complete** -- flashed and bench-confirmed at 192.168.50.24 (both outputs OFF at boot with both flags `false`, so FH's boards are active-HIGH; coils, status, buttons, hub-regains-authority, 5 s watchdog and heartbeat/reconnect all passed). The note below is the pre-flash history. **Oct 21, 2026 (Phase C):** the hub-write rule described below ("every write is a fresh command / regains authority") was changed: a coil write now counts only when its value differs from the last hub value, because OpenPLC rewrites the coil every poll. See `PLC_Logic/FH-PowerBox/Build_Log.md`.

**Status note (Oct 6, 2026):** FH-PowerBox was built ahead of the BSH build order at Luis's request, and its Phase B sketch is written but NOT yet flashed. Its standalone Phase A was never logged separately, and BSH's relay polarity (active-HIGH) cannot be assumed for these boards: at boot both outputs must be OFF; if not, flip `RELAY_LIGHTS_ACTIVE_LOW` / `RELAY_FAN_ACTIVE_LOW` in the sketch.

**Earlier status note (Oct 5, 2026):** BSH-PowerBox completed its own Phase A
bench test and confirmed both its relay boards active-HIGH -- see
`Firmware/BSH_PowerBox_Bringup_Checklist.md` Section 4. This node (FH) never had a
separate Phase A bench test; its two boards were verified directly during
Phase B on Oct 7, 2026 (both active-HIGH).

---

## 1. Parts Needed

Identical BOM to BSH-PowerBox — see
`Firmware/BSH_PowerBox_Bringup_Checklist.md` Section 1: ESP32 DevKit, two
separate 1-channel relay modules (one per channel -- lights, fan; each
with its own VCC/GND), 3 LEDs (one per floor) with their own resistors,
small fan (DC motor + fan blade, or stand-in), two momentary push buttons,
breadboard/jumpers, USB cable with data wires, shared project Wi-Fi AP.

---

## 2. Wiring

Identical pin map to BSH-PowerBox — see `FH_Sensors/09_Relay_Module.md`
and `FH_Sensors/02_Push_Button.md`.

| Component | ESP32 Pin | Notes |
|---|---|---|
| Lights relay | GPIO 13 | Its own 1-channel relay module -- own VCC/GND, verify active-LOW/active-HIGH independently |
| Fan relay | GPIO 15 | A separate 1-channel relay module from lights -- own VCC/GND, verify active-LOW/active-HIGH independently |
| Lights override button | GPIO 4 | `INPUT_PULLUP`, active LOW |
| Fan override button | GPIO 16 | `INPUT_PULLUP`, active LOW |

Since Sep 30, 2026 lights and fan are on two separate relay boards, not
two channels of one module -- don't assume one board's polarity result
tells you anything about the other's. Firmware uses two independent
flags, `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW`, to match.
BSH-PowerBox's Oct 5, 2026 bench test found both of its boards
active-HIGH -- worth trying first here, but not a substitute for this
node's own test.

---

## 3. Arduino IDE Setup

Same as BSH-PowerBox: ESP32 board support, board = **ESP32 Dev Module**,
**ModbusIP_ESP8266**. Once Phase B starts, open `FH_PowerBox_Node.ino` and
fill in real `ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-PowerBox/Wokwi/` (FH mirrors BSH — copy it to `Firmware/FH-PowerBox/Wokwi/`) on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: lights relay, fan relay, lights button, fan button
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Relay polarity" and "Real bench differences"
      sections, then logged the run (date + anything surprising) in each
      component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — voltage
dividers, external pull-ups, and supply rails still get checked on the
bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

**No standalone Phase A was logged for this node** (Oct 5, 2026 note; superseded by the Phase B run on Oct 7, which doubled as the hardware check) -- BSH-PowerBox completed
its own Phase A the same day; see
`Firmware/BSH_PowerBox_Bringup_Checklist.md` Section 4 for the pattern
this node's own run is expected to follow (including testing fan/lights/
buttons together via a combined sketch rather than strictly separate
standalone runs, and double-checking the flashed sketch is actually
current before troubleshooting a "no Serial output at all" symptom as a
hardware fault).

Same build order as BSH-PowerBox (Phase A never touches WiFi, so the fixed
IP doesn't matter yet):

1. **Fan relay alone** — confirm on/off and polarity. Don't assume
   BSH-PowerBox's bench result carries over -- this is FH's own relay
   board (though BSH's Oct 5, 2026 finding of active-HIGH is a reasonable
   first guess to try).
2. **Lights relay alone** — same check on the second relay module.
3. **Buttons alone** — confirm each reads LOW when pressed and debounces
   cleanly.
4. **Combined standalone integration sketch** — both relays and both
   buttons together, no WiFi/Modbus, printed to Serial.
5. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Fill in real Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.50.24`.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Write Coil 0 (`LIGHTS_ON_CMD`) = 1 — light turns on, `LIGHTS_STATUS`
     (DI0) reads back 1.
   - Write Coil 1 (`FAN_CMD`) = 1/0 — fan follows, `FAN_STATUS` (DI1)
     tracks it.
   - Press a button on the bench and confirm it flips the output and its
     status DI immediately, without any coil write, and that a
     subsequent fresh coil write from the client correctly regains
     authority over it. (Oct 21, 2026: only a write with a DIFFERENT
     value than the last hub value counts; a repeated same value is
     ignored.)
   - Disconnect Wi-Fi with lights/fan on — both default to OFF once the
     watchdog timeout elapses, but the buttons keep working locally.
4. With both BSH-PowerBox and FH-PowerBox up simultaneously **and both on
   their correct, distinct IPs**, confirm a Modbus client can reach and
   control each independently.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 0 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 1 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/FH-PowerBox/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-PowerBox` — its own Docker container at 192.168.50.124 (in the Proxmox OpenPLC Docker VM),
   Web UI on port 8080, Modbus server on port 502; it polls this node's
   ESP32 at 192.168.50.24, port 502). Set coil count = 2, discrete input
   count = 2, input register count = 2 (IR0 `HEARTBEAT`, IR1 `RECONNECT_COUNT`).
2. Write the same minimal ladder as BSH-PowerBox.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add lights and fan tiles to the shared SCADA dashboard (Ignition), next to
   BSH-PowerBox's, showing both commanded and actual state.

---

## 7. Success Criteria — This System Is Done When:

- Serial confirms `.24`, not `.14`.
- Both relay outputs and both buttons respond correctly with zero network
  dependency (Phase A).
- A plain Modbus write to this node's own IP changes both outputs, both
  fail safely to OFF on comm loss, and local button overrides plus the
  hub-regains-authority-on-fresh-command rule (Oct 21, 2026: a fresh command means a changed value) behave the same as
  BSH-PowerBox (Phase B).
- OpenPLC and the SCADA dashboard both show this node's live state, distinguishable
  side-by-side from BSH-PowerBox's (Phase C).

At that point FH-PowerBox — the last of the eight project nodes, per `Firmware/WORKFLOW.md`'s Phase 2 order — is done.
