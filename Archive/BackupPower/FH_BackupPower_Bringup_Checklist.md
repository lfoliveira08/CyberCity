# FH-BackupPower — Bring-Up Checklist

**Purpose:** Founder's Hall mirror of BSH-BackupPower — fifth (last) FH
system per `Firmware/WORKFLOW.md`'s Phase 2 order. Identical hardware,
wiring, and logic to BSH-BackupPower; only the static IP should differ.
Replaces the retired FH-Perimeter node in this slot (see
`Piranga_Workflow_v2.md` Section 14, Sep 28, 2026).

Firmware for this node: `FH_BackupPower_Node.ino` (same folder as this
file, once Phase B starts). **Not written yet** — neither BSH's nor FH's
sketch exists yet. When it is written, double-check the `IPAddress ip(...)`
line is set to this node's own documented `192.168.1.25`
(`FH_Systems/05_FH-BackupPower.md`), not BSH-BackupPower's `192.168.1.15`
— this exact class of copy-paste bug has already bitten FH-Door,
FH-Elevator, and FH-PowerBox in this project (see their bring-up
checklists / Section 14), so it's worth checking before it happens a
fourth time rather than after.

---

## 1. Parts Needed

Identical BOM to BSH-BackupPower — see
`Firmware/BSH_BackupPower_Bringup_Checklist.md` Section 1: ESP32 DevKit,
SPST toggle switch, 1-channel relay module, small demo load, two
divider resistors sized to whatever battery pack is used, a small battery
pack, an ACS712 current sensor module, breadboard/jumpers, USB cable with
data wires, shared project Wi-Fi AP.

---

## 2. Wiring

Identical pin map to BSH-BackupPower — see `FH_Sensors/18_Toggle_Switch.md`,
`09_Relay_Module.md`, `19_Voltage_Divider_Battery_Sense.md`,
`20_ACS712_Current_Sensor.md`.

| Component | ESP32 Pin | Notes |
|---|---|---|
| Utility-loss toggle switch | GPIO 26 | `INPUT_PULLUP`, active LOW |
| ATS transfer relay | GPIO 27 | Verify active-LOW/active-HIGH against your module |
| Battery voltage divider tap | GPIO 34 | ADC1 — same "size it to the real pack" caution as BSH |
| ACS712 OUT | GPIO 35 | ADC1 — measure OUT swing with a multimeter first |

---

## 3. Arduino IDE Setup

Same as BSH-BackupPower: ESP32 board support, board = **ESP32 Dev
Module**, **ModbusIP_ESP8266** (no other third-party library needed). Once
Phase B starts, open `FH_BackupPower_Node.ino`, confirm the IP is
`192.168.1.25`, and fill in real `ssid`/`password`.

---

## 3b. Phase A0 — Wokwi Simulation (before touching real parts)

Rule: `Firmware/WORKFLOW.md` Section 7. Goal: understand what each part
does and how it's wired before the bench build.

- [ ] Open the simulation from `Firmware/BSH-BackupPower/Wokwi/` (FH mirrors BSH — copy it to `Firmware/FH-BackupPower/Wokwi/`) on wokwi.com (new ESP32 project →
      paste `sketch.ino` and `diagram.json`) and press play
- [ ] Every part responds: toggle switch, relay/LED, two potentiometers standing in for the battery divider and the ACS712
- [ ] Every scenario in the Wokwi folder's `README.md` behaves as described
- [ ] Read the README's "Real bench differences" list, then logged the run
      (date + anything surprising) in each component's `Build_Log.md`

Simulation doesn't prove the wiring is electrically safe — the divider
math and the ACS712's real voltage swing still get checked on the bench in
Phase A.

---

## 4. Phase A — Standalone Hardware Test (no Wi-Fi/OpenPLC needed yet)

Same build order as BSH-BackupPower:

1. **Toggle switch alone** — confirm `UTILITY_POWER_LOST` flips cleanly.
2. **Relay alone** — confirm on/off and polarity.
3. **Voltage divider alone** — verify against a multimeter reading of the
   real battery before trusting the percentage math.
4. **ACS712 alone** — confirm baseline and load-current response.
5. **Combined standalone integration sketch** — all four signals over
   Serial, no WiFi/Modbus.
6. If anything here doesn't work, it's wiring/hardware — fix it before
   Phase B.

---

## 5. Phase B — Modbus TCP Verification (no OpenPLC yet)

1. **Confirm the IP is correct** (see the callout above), then fill in
   real Wi-Fi credentials and reflash.
2. Confirm the Serial Monitor prints `Connected. IP address: 192.168.1.25`
   — if it prints `.15` instead, the sketch still has BSH's hardcoded IP.
3. From a laptop on the same Wi-Fi/subnet, use a Modbus TCP client:
   - Read DI0/DI1, IR0/IR1 — same checks as BSH-BackupPower Phase B.
   - Write Coil0 (`ATS_TRANSFER_CMD`) = 1 — relay/demo load responds.
   - Disconnect Wi-Fi with `ATS_TRANSFER_CMD` active — confirm it fails
     toward **not commanded**, same as BSH.
4. With both BSH-BackupPower and FH-BackupPower up simultaneously **and
   both on their correct, distinct IPs**, confirm a Modbus client can
   reach and control each independently.

---

## 6. Phase C — OpenPLC + SCADA Integration

1. Add this ESP32 as a **Generic Modbus TCP Device** in a new OpenPLC
   instance (`OpenPLC-FH-BackupPower` — its own Docker container at
   192.168.1.125, Web UI on port 8080, Modbus server on port 502; it polls
   this node's ESP32 at 192.168.1.25, port 502). Set discrete input
   count = 2, coil count = 1, input register count = 2.
2. Write the same minimal ladder as BSH-BackupPower.
3. Confirm OpenPLC's Monitoring page matches Phase B's live values.
4. Add this node's tiles to the shared Ignition dashboard, next to
   BSH-BackupPower's.

---

## 7. Success Criteria — This System Is Done When:

- The IP is confirmed correct on real hardware (Serial prints `.25`, not
  `.15`).
- All four signals report correctly with zero network dependency
  (Phase A).
- A plain Modbus write to this node's own IP drives the relay, and it
  fails toward **not commanded** on comm loss (Phase B).
- OpenPLC and Ignition both show this node's live state, distinguishable
  side-by-side from BSH-BackupPower's (Phase C).

This is the last of the ten project nodes, per `Firmware/WORKFLOW.md`'s
Phase 2 order.
