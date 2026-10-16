# FH-HVAC — Bring-Up Checklist

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

**Purpose:** Founder's Hall slot 1 — an HVAC zone unit, adopted Sep 26, 2026 in
place of the retired FH-Elevator. Second FH system per `Firmware/WORKFLOW.md`'s
Phase 2 order, which only begins once all five BSH systems are complete.

System doc: `FH_Systems/01_FH-HVAC.md`. Sensor sheet:
`FH_Sensors/17_DHT22_Temp_Humidity_Sensor.md`.

> **Status 2026-10-07:** `FH_HVAC_Node.ino` exists (Phase B: WiFi + Modbus TCP at
> 192.168.50.21) and is bench-confirmed. Phases A0, A and B are done; Phase C
> (OpenPLC + SCADA) has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet). Hardware as built: DHT11, keyestudio relay
> (active-HIGH), DC motor fan on the board 5V pin with a 1N4007 flyback diode, RGB LED.
> See `FH-HVAC/` for the per-component logs.

> **Update 2026-10-07 (latest): Phase B complete** -- flashed and bench-confirmed at 192.168.50.21 (see `FH-HVAC/ESP32_Core_Modbus/Build_Log.md`). Phase C not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).
>
> **Update 2026-10-07 (later):** Phase A bench-confirmed (fault test pending), Phase B sketch written but not flashed. Hardware actually used: DHT11, keyestudio relay (active-HIGH), DC motor on the board's 5V with a 1N4007.
>
> **Update 2026-10-07:** `Firmware/FH-HVAC/` now exists with the Phase A standalone sketch
> (`FH_HVAC_Node/FH_HVAC_Node.ino`), Wokwi files, and per-component folders. Run in Wokwi
> and bench-confirmed in Phases A and B (WiFi + Modbus). Phase C is not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).
> IPs in this file were corrected from the stale 192.168.1.x plan to 192.168.50.x (Oct 6 move).

> **This node is not a mirror.** Every other FH system copies a BSH counterpart
> pin-for-pin and can lean on its bring-up notes. FH-HVAC cannot — no BSH system
> uses a DHT22 or a setpoint register. Expect this node to behave like a first
> build (it is one), and log accordingly.

---

## 0. Two design decisions to settle before writing the firmware

Both are called out in `FH_Systems/01_FH-HVAC.md` and neither should be decided
by accident while coding:

- [x] **DECIDED 2026-10-07: fail-OFF.** **`HVAC_ENABLE` fail direction on comms loss.** Fail-off (like
      `ELEVATOR_DISABLE`) avoids a compressor running unsupervised. Fail-on (like
      the retired `ZONE_DISABLE` on BSH-Perimeter once did) keeps the zone climate-controlled if the
      network drops. Pick one deliberately, document the reason, and say it out
      loud in any presentation of this node — the project already makes a point
      of `ZONE_DISABLE` being the deliberate odd one out.
- [x] **DECIDED 2026-10-07: tenths (23.4 C -> 234), both registers; humidity whole %.** **Register scaling for `TEMP_C` / `TARGET_TEMP_C`.** Whole degrees, or
      tenths (23.4°C → `234`)? Modbus registers are 16-bit integers and the DHT22
      reports tenths. Whatever is chosen must apply to *both* registers, or the
      setpoint comparison silently misbehaves.

---

## 1. Parts Needed

- ESP32 DevKit
- **DHT11, 3-pin module** (decided 2026-10-07: it is what was in hand; the design docs say
  DHT22 / AM2302). Pull-up already on the module -- no resistor. Whole-degree readings, 0-50 C
- 1-channel relay module (compressor/fan simulation)
- RGB LED, common cathode + 3× 220Ω resistors (idle / cooling / fault status)
- Small DC fan, switched through the relay (powered from the board's 5V pin for now; own supply if the board resets) (**included in the build,
  decided 2026-10-07**) + a diode across it if it is a plain brushed motor
- Breadboard, jumpers, USB cable with data wires, shared project Wi-Fi AP

---

## 2. Wiring

Provisional pin map — from `FH_Systems/01_FH-HVAC.md`:

| Component | ESP32 Pin | Notes |
|---|---|---|
| DHT22 DATA | GPIO 4 | 10kΩ pull-up to 3.3V. Must not be on GPIO 34–39 (input-only, no internal pull-ups) |
| DHT22 VCC | 3.3V | The part accepts 3.3–6V; 3.3V keeps it on the ESP32's own rail |
| Relay IN | GPIO 26 | Many modules are active-LOW — verify yours before trusting the logic |
| RGB LED R / G / B | GPIO 16 / 17 / 18 | Through 220Ω each, common cathode to GND |

- [x] Fan power: as built, the motor runs from the ESP32 board 5V pin through the
      relay with a 1N4007 flyback diode (battery-pack fallback if the board resets;
      never the DC jack). Revisit the separate-supply rule if resets appear.
- [x] Confirmed the relay module's trigger polarity: keyestudio module is
      active-HIGH (`RELAY_ACTIVE_LOW = false`), recorded 2026-10-07

---

## 3. Arduino IDE Setup

ESP32 board support installed, board = **ESP32 Dev Module**. Libraries:
**ModbusIP_ESP8266** (Phase B), plus **Adafruit DHT sensor library** and
**Adafruit Unified Sensor** (or `DHT_nonblocking`). Record the exact versions
installed in `References/INDEX.md`, per that file's rule.

- [x] (Done) When `FH_HVAC_Node.ino` was first written, set the static IP to
      **192.168.50.21** from the very first line of code. Two FH sketches have
      already shipped with a BSH IP copy-pasted in (FH-Door had `.13`, FH-Elevator
      had `.11`) — this node has the chance to simply not repeat that.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. **Build this one from scratch** —
`Firmware/FH-HVAC/Wokwi/` — since there is no BSH simulation to copy. All three
parts are native Wokwi parts, so no stand-ins are needed, which makes this the
most faithful simulation in the project.

- [x] `diagram.json` built (2026-10-07) with the GPIO numbers in Section 2 exactly
- [x] `sketch.ino` holds (2026-10-07) the standalone logic (no WiFi, no Modbus)
- [x] `README.md` written (2026-10-07): how to open it, what to click, what you should see,
      and a "Real bench differences" list
- [ ] Raising simulated temperature above setpoint closes the relay; lowering it
      opens it; the LED tracks idle vs cooling
- [ ] Clearing `HVAC_ENABLE` forces the relay open regardless of temperature
- [ ] Logged the run (date + anything surprising) in each component's `Build_Log.md`

The simulation cannot show you an `HVAC_FAULT` — it never fails a read. That path
gets proven on the bench in Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC yet)

Build in this order, one part at a time, then all three together:

1. **DHT22** — print temperature and humidity over Serial. Confirm the read is on
   a `millis()` timer at 2s or slower, **not** once per loop; confirm plausible
   room values, not 0 or NaN.
2. **RGB LED** — confirm all three channels drive independently.
3. **Relay** — confirm it switches, with the fan on its own supply, and that the
   polarity matches what Section 2 recorded.
4. **Combined standalone integration sketch** — non-blocking: read on a timer,
   compare to a hard-coded setpoint, drive relay + LED, latch a fault on a failed
   read. Same pattern as `BSH_FireAlarm_Node.ino`'s Phase A version.
5. **Force the fault path** — unplug the DHT22's data line while it runs.
   `HVAC_FAULT` should latch and the reading must not fall back to `0`.
6. Anything failing here is wiring or hardware. Fix it before Phase B.

Then write `Firmware/FH-HVAC/SYSTEM_REPORT.md`, per `Firmware/WORKFLOW.md`
Section 4, before moving on.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. Add WiFi + `ModbusIP_ESP8266` to the sketch, static IP **192.168.50.21**.
2. Confirm the Serial Monitor prints `192.168.50.21` — not `.11`, not `.21`'s BSH
   neighbour.
3. From a laptop Modbus TCP client on the same subnet:
   - Read IR0 / IR1 (`TEMP_C`, `HUMIDITY_PCT`) — plausible, and moving when you
     breathe on the sensor.
   - Write HR0 (`TARGET_TEMP_C`) below current temperature — the relay closes;
     above it — the relay opens.
   - Read DI0 (`COMPRESSOR_RUNNING`) — tracks the relay honestly.
   - Clear Coil0 (`HVAC_ENABLE`) — the relay opens and stays open regardless of
     `TARGET_TEMP_C`.
   - Unplug the sensor — DI1 (`HVAC_FAULT`) latches.
4. Cut the network mid-run and confirm the node does whatever Section 0's
   fail-direction decision says it should — this is the test that decision exists for.


**Connection-monitor check (added Oct 6, 2026):** read Input Register 2 (`HEARTBEAT`) twice a few seconds apart -- it must increase by about 1 per second. Read Input Register 3 (`RECONNECT_COUNT`) -- it must read 0 right after boot. Then switch the Wi-Fi router off for ~15 seconds and back on: the heartbeat resumes (the board kept counting) and `RECONNECT_COUNT` reads 1. Press the ESP32's reset button: both registers go back to 0 (reboot, not a Wi-Fi drop). If either register is missing here, the firmware is out of date -- fix it now, because OpenPLC will be configured to read them in Phase C.

---

## 6. Phase C — OpenPLC + SCADA Integration

> Drafted ladder diagram, register map and program for this node: `PLC_Logic/FH-HVAC/` (untested until Phase C).

1. Add this ESP32 as a **Generic Modbus TCP Device** in its own OpenPLC instance
   (`OpenPLC-FH-HVAC` — its own Docker container at 192.168.50.121 (in the Proxmox OpenPLC Docker VM), Web UI on port
   8080, Modbus server on port 502; it polls this node's ESP32 at 192.168.50.21,
   port 502). Set discrete input count = 2, coil count = 1, input register
   count = 4 (IR0 `TEMP_C`, IR1 `HUMIDITY_PCT`, IR2 `HEARTBEAT`, IR3 `RECONNECT_COUNT`), holding register count = 1.
2. Ladder logic: pass `HVAC_ENABLE` through to the ESP32, hold the safe state on
   comms loss per Section 0, and expose the setpoint write.
3. Confirm OpenPLC's Monitoring page also shows `HEARTBEAT` counting up and `RECONNECT_COUNT` (if either is missing or frozen at 0, the Input Register count in step 1 is too low). Then confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add a zone tile to the SCADA dashboard (Ignition) — temperature, setpoint, compressor
   state, fault — beside Founder's Hall's other four systems.
5. Build Scenario 11 (`Piranga_Workflow_v2.md` Section 11): spoof `TEMP_C` or
   rewrite `TARGET_TEMP_C` from an unauthorized Modbus client, and show the SCADA
   layer catching `COMPRESSOR_RUNNING` true while `TEMP_C` never converges.

---

## 7. Success Criteria — This System Is Done When:

- Both Section 0 decisions are made, documented, and reflected in the firmware.
- The DHT22 reads on a timer without disturbing anything else, and a pulled data
  line latches `HVAC_FAULT` rather than reporting a fake `0` (Phase A).
- A plain Modbus write to `TARGET_TEMP_C` visibly changes the relay's behaviour,
  and `HVAC_ENABLE` overrides it (Phase B).
- OpenPLC and the SCADA dashboard both show this zone live, and Scenario 11 demonstrably
  detects a spoofed temperature (Phase C).

At that point Phase 2 continues with FH-Door, per `Firmware/WORKFLOW.md`.
