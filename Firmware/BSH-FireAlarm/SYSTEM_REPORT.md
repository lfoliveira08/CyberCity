# BSH-FireAlarm — System Report

**Status: Phase A complete** (standalone hardware integration test passing).
**Phase B (WiFi + Modbus TCP) bench-confirmed 2026-10-06** at 192.168.50.12 (see
`ESP32_Core_Modbus/Build_Log.md`; the sketch in `BSH_FireAlarm_Node/` is now the
Phase B version, and this report keeps the Phase A record). Phase C
(OpenPLC/SCADA) has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet) — see `Firmware/WORKFLOW.md`.

This is the first per-system completion report, produced per the process in
`Firmware/WORKFLOW.md` Section 4: written once a system's Phase A is done,
before moving on to the next system.

---

## 1. How the system works

BSH-FireAlarm is one of eight node builds in Piranga (4 systems per building, BSH and FH;
this count was 10 when this report was written, before Backup Power was dropped on Oct 5, 2026). Its job is to represent a fire/gas alarm point for the
BSH building: it watches three independent local trigger sources and drives
one shared alarm output (buzzer + RGB LED) whenever any of them fires.

**Trigger sources (OR'd together into one `alarmActive` flag):**
1. **Pull-station button** — a manual "someone pulled the alarm" trigger,
   debounced in software (50 ms).
2. **Flame sensor** — fast, direct IR-based flame detection.
3. **Gas/smoke sensor** — slower, indirect detection of rising combustible-gas
   concentration; compares a live analog reading against a clean-air baseline
   captured at boot.

Once Phase B (WiFi/Modbus) is added, a fourth source — a remote
`ALARM_ACTIVATE` Modbus coil written from the central hub — will OR into the
same flag, so the hub can also trigger this node's alarm remotely (used for
the "false alarm injection" cybersecurity demo scenario). That coil does not
exist yet in the current Phase A firmware.

**Output behavior:**
- Normal: buzzer off, RGB LED steady green.
- Alarm active (any source): buzzer on continuously, RGB LED blinks red
  (300 ms toggle).

**Observability:** once a second, the firmware prints one line to Serial
showing every input's raw/derived state and the combined alarm flag — e.g.
`button=released  flame=clear  gasRaw=612 (baseline=600, alert>1000 ok)  ==>
alarmActive=false (green)`. This is how Phase A was bench-verified without
Modbus or a laptop polling tool.

## 2. The code

Current firmware (`Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino`)
is the Phase A standalone integration test sketch — no WiFi, no Modbus. It is
standing in as this node's firmware for now; the WiFi+Modbus version will
replace it when Phase B starts (see `Firmware/WORKFLOW.md` Section 5). Full
sketch, as it exists on the bench today:

```cpp
/*
  ============================================================================
  BSH-FireAlarm — Standalone Integration Test (NO WiFi / NO Modbus)
  ============================================================================
  Purpose: test all five bring-up components TOGETHER on one ESP32, the way
  they'll actually behave once integrated — but without touching the network
  or Modbus layer yet. This is the "does the hardware work as one system"
  step, deliberately kept separate from Phase B (WiFi/Modbus TCP slave) so
  hardware problems and network problems never get debugged at the same
  time.

  STATUS (2026-09-21): Phase A hardware bring-up confirmed working — button,
  flame, and gas/smoke sensors all OR correctly into the buzzer/LED alarm
  logic. This file is standing in as the current BSH-FireAlarm firmware for
  now, in place of the full WiFi+Modbus version, until WiFi and OpenPLC
  integration (Phase B/C) are set up next.

  What it does:
    - Reads the pull-station button (debounced, active-LOW)
    - Reads the flame sensor (active-LOW)
    - Reads the gas/smoke sensor (analog, MQ-9 on this bench — see
      Firmware/BSH-FireAlarm/MQ-2_Gas_Smoke_Sensor/Wiring.md), computes a
      clean-air baseline at boot, and alerts on drift above that baseline
    - ORs all three into one alarmActive flag
    - Drives the buzzer + RGB LED exactly like the real firmware:
        normal    -> steady green, buzzer off
        triggered -> blinking red, buzzer on
    - Prints every input's raw/derived state to Serial once a second, so
      you can watch all three sensors and the combined alarm state at once
      without needing Modbus or a laptop polling tool

  Hardware note: gas/smoke sensor's AO pin should already be voltage-checked
  safe for GPIO 34 (under ~3.3V) before running this — see the Wiring.md
  safety note if that hasn't been confirmed yet.

  Board: "ESP32 Dev Module"
  No libraries beyond the ESP32 core needed (no ModbusIP_ESP8266 here).
  ============================================================================
*/

// ---------------------------------------------------------------------------
// GPIO pin map — identical to the full BSH-FireAlarm node firmware
// ---------------------------------------------------------------------------
const int PIN_BUZZER      = 23;
const int PIN_RGB_RED     = 25;
const int PIN_RGB_GREEN   = 26;
const int PIN_RGB_BLUE    = 27;
const int PIN_PULL_BUTTON = 4;   // plain INPUT, active LOW — Keyestudio module has its own resistor
const int PIN_FLAME       = 18;  // plain INPUT, active LOW
const int PIN_GAS         = 34;  // ADC1, input-only — gas/smoke sensor (MQ-9 on this bench)

// ---------------------------------------------------------------------------
// Debounce state (pull-station button)
// ---------------------------------------------------------------------------
const unsigned long DEBOUNCE_MS = 50;
unsigned long lastButtonChange   = 0;
bool debouncedButtonState        = false;  // true = pressed / triggered
bool rawButtonLast               = false;

// ---------------------------------------------------------------------------
// Gas/smoke calibration & threshold — same approach as the real firmware
// ---------------------------------------------------------------------------
const int GAS_CALIBRATION_SAMPLES        = 20;
const unsigned long GAS_CALIBRATION_INTERVAL_MS = 0;  // no delay between samples — near-instant boot, gas baseline is rougher (no averaging over time) but fine while we're only testing flame right now
const int GAS_ALERT_MARGIN               = 400;  // placeholder — tune on real hardware
int gasBaseline                          = 0;

// ---------------------------------------------------------------------------
// Non-blocking alarm blink state
// ---------------------------------------------------------------------------
const unsigned long BLINK_INTERVAL_MS = 300;
unsigned long lastBlinkToggle          = 0;
bool blinkOn                           = false;

// ---------------------------------------------------------------------------
// Status print throttle (once a second, so Serial isn't a firehose)
// ---------------------------------------------------------------------------
const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint                = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(PIN_RGB_RED,   r ? HIGH : LOW);
  digitalWrite(PIN_RGB_GREEN, g ? HIGH : LOW);
  digitalWrite(PIN_RGB_BLUE,  b ? HIGH : LOW);
}

void calibrateGasSensor() {
  Serial.println("Calibrating gas/smoke baseline (fast, no delay -- rough estimate, fine for a flame-only test run)...");
  long total = 0;
  for (int i = 0; i < GAS_CALIBRATION_SAMPLES; i++) {
    total += analogRead(PIN_GAS);
    if (GAS_CALIBRATION_INTERVAL_MS > 0) delay(GAS_CALIBRATION_INTERVAL_MS);
  }
  gasBaseline = total / GAS_CALIBRATION_SAMPLES;
  Serial.print("Gas/smoke baseline established: ");
  Serial.println(gasBaseline);
  Serial.print("Alert will trigger above: ");
  Serial.println(gasBaseline + GAS_ALERT_MARGIN);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-FireAlarm Standalone Integration Test (no WiFi/Modbus) ===");

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  pinMode(PIN_PULL_BUTTON, INPUT);  // plain INPUT -- Keyestudio module supplies its own pull resistor
  pinMode(PIN_FLAME, INPUT);
  // PIN_GAS is analog-only (ADC1) -- no pinMode() call needed for analogRead()

  digitalWrite(PIN_BUZZER, LOW);
  setRgb(false, true, false);   // boot state: steady green = normal

  calibrateGasSensor();

  Serial.println("Setup complete. Watching button / flame / gas together.");
  Serial.println("Trigger any one of them and confirm buzzer + red blink respond.");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  // -------------------------------------------------------------------
  // Debounced pull-station button read
  // -------------------------------------------------------------------
  bool rawPressed = (digitalRead(PIN_PULL_BUTTON) == LOW);
  if (rawPressed != rawButtonLast) {
    lastButtonChange = now;
    rawButtonLast = rawPressed;
  }
  if (now - lastButtonChange > DEBOUNCE_MS) {
    debouncedButtonState = rawPressed;
  }

  // -------------------------------------------------------------------
  // Flame sensor (active LOW)
  // -------------------------------------------------------------------
  bool flameDetected = (digitalRead(PIN_FLAME) == LOW);

  // -------------------------------------------------------------------
  // Gas/smoke sensor
  // -------------------------------------------------------------------
  int gasRaw = analogRead(PIN_GAS);
  bool gasSmokeAlert = (gasRaw > gasBaseline + GAS_ALERT_MARGIN);

  // -------------------------------------------------------------------
  // Combine trigger sources (OR logic -- same as the real firmware, minus
  // the remote Modbus coil, which doesn't exist in this standalone test)
  // -------------------------------------------------------------------
  bool alarmActive = debouncedButtonState || flameDetected || gasSmokeAlert;

  // -------------------------------------------------------------------
  // Drive outputs
  // -------------------------------------------------------------------
  if (alarmActive) {
    digitalWrite(PIN_BUZZER, HIGH);
    if (now - lastBlinkToggle >= BLINK_INTERVAL_MS) {
      lastBlinkToggle = now;
      blinkOn = !blinkOn;
      setRgb(blinkOn, false, false);   // blinking red
    }
  } else {
    digitalWrite(PIN_BUZZER, LOW);
    setRgb(false, true, false);        // steady green = normal
  }

  // -------------------------------------------------------------------
  // Status print, once a second -- shows every input + the combined state
  // -------------------------------------------------------------------
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("button=");
    Serial.print(debouncedButtonState ? "PRESSED" : "released");
    Serial.print("  flame=");
    Serial.print(flameDetected ? "DETECTED" : "clear");
    Serial.print("  gasRaw=");
    Serial.print(gasRaw);
    Serial.print(" (baseline=");
    Serial.print(gasBaseline);
    Serial.print(", alert>");
    Serial.print(gasBaseline + GAS_ALERT_MARGIN);
    Serial.print(gasSmokeAlert ? " ALERT" : " ok");
    Serial.print(")  ==> alarmActive=");
    Serial.println(alarmActive ? "TRUE (buzzer on, red blink)" : "false (green)");
  }
}
```

A version of this same sketch, adapted only in its header comments (FH
mirrors BSH pin-for-pin), is also in use as FH-FireAlarm's Phase A stand-in:
`Firmware/FH_FireAlarm_Standalone_Test/FH_FireAlarm_Standalone_Test.ino`.

## 3. Wiring (consolidated across all five components)

| Component | Part | Pin | ESP32 pin | Notes |
|---|---|---|---|---|
| Active buzzer | generic active buzzer module | Signal | GPIO 23 | GND -> GND. Digital output, no PWM needed (on/off only). |
| RGB LED | common-cathode RGB LED | Red | GPIO 25 | 220Ω resistor per channel |
| | | Green | GPIO 26 | 220Ω resistor per channel |
| | | Blue | GPIO 27 | 220Ω resistor per channel |
| | | Common | GND | Common-cathode leg |
| Pull-station button | Keyestudio Button Switch module (S/V/G) | S (signal) | GPIO 4 | Active-LOW (pressed = LOW). Module has its own onboard resistor — pin must be plain `INPUT`, **not** `INPUT_PULLUP`. V -> 3.3V, G -> GND. |
| Flame sensor | 3-pin IR flame sensor (LM393 comparator breakout) | D0 (digital out) | GPIO 18 | Active-LOW. Plain `INPUT`, no pull-up needed — module drives both states. VCC -> 3.3V or 5V (check module rating), GND -> GND. |
| Gas/smoke sensor | MQ-9 module ("MH Sensor series," documented as MQ-2 — see Section 5) | AO (analog out) | GPIO 34 | ADC1, input-only. GPIO 21 was considered and rejected — no ADC hardware there. |
| | | DO (digital out) | GPIO 19 | Wired for future use; not read by current firmware. |
| | | VCC | VIN | Heater coil needs 5V — VIN ties to the board's 5V rail. |
| | | GND | GND | Shared ground; also where a voltage divider's ground leg would land if one is added. |

**Safety note carried over from `MQ-2_Gas_Smoke_Sensor/Wiring.md`:** the
gas sensor's AO pin should be multimeter-checked (sensor powered, clean air)
to confirm it stays under ~3.3V before trusting GPIO 34. If it swings closer
to 5V, add a 1kΩ/2kΩ voltage divider (same pattern used for the HC-SR04's
ECHO pin elsewhere in the project) between AO and GPIO 34. **This check was
not done before wiring** — see Section 5, it is still open.

## 4. Schema — target Modbus register map

Phase A has no Modbus layer yet (see `Firmware/WORKFLOW.md` Section 5). This
is the register map `BSH_FireAlarm_Node.ino`'s real WiFi/Modbus version will
expose once Phase B starts, so wiring and firmware stay aligned with the
eventual network layer from the start:

| Register | Type | Index | Source | Meaning |
|---|---|---|---|---|
| `MANUAL_PULL_TRIGGERED` | Discrete Input | DI0 | Pull-station button | 1 = button currently pressed (debounced) |
| `FLAME_DETECTED` | Discrete Input | DI1 | Flame sensor | 1 = flame/IR detected |
| `GAS_SMOKE_ALERT` | Discrete Input | DI2 | Gas/smoke sensor | 1 = raw reading has crossed `gasBaseline + GAS_ALERT_MARGIN` |
| `GAS_RAW_VALUE` | Input Register | IR0 | Gas/smoke sensor | Raw ADC reading, 0-4095 |
| `HEARTBEAT` | Input Register | IR1 | Firmware (Phase B, added Oct 6, 2026) | Counts up once a second; frozen = node lost |
| `RECONNECT_COUNT` | Input Register | IR2 | Firmware (Phase B, added Oct 6, 2026) | WiFi rejoins since boot |
| `ALARM_ACTIVATE` | Coil | Coil0 | Central hub (write) | Hub-issued remote trigger; will OR into local `alarmActive` once Phase B is implemented. Used for the false-alarm-injection cybersecurity demo scenario. Not present in current Phase A firmware. |

## 5. Troubleshooting / Lessons Learned

Consolidated from each component's `Build_Log.md`/`Wiring.md`, plus two
issues seen during live bench testing that are diagnosed but **not yet
confirmed fixed** — flagged clearly below so they aren't lost.

### Resolved / confirmed during Phase A

- **Gas sensor is actually an MQ-9, not the documented MQ-2** — confirmed
  from the model number printed on the sensor can (the PCB alone is a
  generic "MH Sensor" breakout shared across many MQ variants and doesn't
  indicate which one). Decision: proceed with the MQ-9 as-is rather than
  source a real MQ-2 — the project's threshold-relative-to-baseline logic
  and Modbus register names (`GAS_RAW_VALUE`, `GAS_SMOKE_ALERT`) are already
  generic enough to still be accurate, just reframed as "general combustible
  gas detection" rather than "smoke/LPG detection" specifically.
- **MQ-9 real preheat time is 24-48 hours**, not the 20-60 seconds originally
  assumed from the MQ-2 plan (per the electropeak MQ-9 calibration
  reference). A minute or two is enough for an "is it alive" sanity check
  only, not for a trustworthy calibration. Build bench-calibration sessions
  around this.
- **GPIO 21 was caught before wiring** as a candidate for the gas sensor's
  analog line — it has no ADC hardware on the ESP32 at all, `analogRead()`
  would not have worked there. Worth checking a board's ADC-capable pin list
  before assuming any GPIO number supports `analogRead()`.
- **Pull-station button is a 3-pin Keyestudio module, not a bare button** —
  it has its own onboard resistor, so the ESP32 side needed plain `INPUT`,
  not `INPUT_PULLUP` (using both together risks inconsistent readings).
  Confirmed active-LOW polarity empirically with a throwaway test sketch
  before touching the real firmware (released = HIGH, pressed = LOW) —
  worth reusing that raw-Serial-print pattern for other digital sensors with
  unknown modules on the remaining systems.
- **RGB LED bench-tested standalone and passed** — a throwaway color-cycle
  test sketch confirmed all three channels (GPIO 25/26/27) wired correctly,
  common leg to GND.
- **Buzzer bench-tested standalone and passed** — confirmed silent at boot,
  steady beeping once toggled. Lesson: touching a jumper from GPIO 23 to
  3.3V by hand is only safe with the ESP32 unpowered; a throwaway toggle
  test sketch is the safer way to bench-test a digital output.
- **Gas calibration delay removed for faster flame-only iteration** —
  `GAS_CALIBRATION_INTERVAL_MS` set to 0 so the 20-sample boot-time baseline
  read happens with no delay between samples. Baseline becomes a rougher
  estimate as a result (no time-averaging), acceptable while bench-testing
  flame detection specifically, but should be reconsidered before relying on
  the gas baseline for real calibration.

### Open — diagnosed, not yet confirmed fixed

- **RGB LED shows green during an active alarm, when it should blink red.**
  A pasted Serial log confirmed the firmware logic itself is correct
  (`alarmActive=TRUE` was printed while the LED showed green), which rules
  out a code bug. Standalone RGB bench test (color-cycle sketch, passed —
  see above) used different wiring than the current integrated build, so
  the most likely cause is a **physical Red/Green wire swap** introduced
  when the LED was rewired into the combined test. Next step: swap the
  GPIO 25 / GPIO 26 wires, or verify each LED leg with a multimeter in
  diode-test mode before re-testing.
- **Flame sensor false-triggers with no real flame present.** Not yet
  isolated to a specific cause. Most likely candidates: ambient IR light
  (sunlight or indoor lighting hitting the sensor), the module's onboard
  sensitivity trim-pot set too high, or a loose GND leaving the comparator
  output floating. Recommended next step: a "cover the sensor" isolation
  test (should read "no flame" when fully covered) to narrow down which of
  the three it is.
- **Gas sensor AO voltage was never multimeter-checked before wiring** (see
  Section 3 safety note). Wiring was completed first; the check should
  still be done now, before trusting any `GAS_RAW_VALUE` readings, in case a
  divider needs to be retrofitted.

### General pattern worth carrying into the next systems

The generic "MH Sensor"/breakout-style PCB used for MQ gas sensors (and
separately for the flame sensor) is shared across many different specific
sensor variants — always check the printed model number on the actual
sensing element, not just the PCB, before assuming which part is in hand.

---

## 6. Simulation (Phase A0 — added retroactively)

BSH-FireAlarm finished Phase A before the Wokwi rule existed
(`Firmware/WORKFLOW.md` Section 7), so its simulation was built afterward
as a learning aid and a template for the other nine systems:
`Firmware/BSH-FireAlarm/Wokwi/`.

- **Stand-ins used:** flame sensor → slide switch (no Wokwi flame part);
  the buzzer is driven with `tone()` in simulation because Wokwi's buzzer
  behaves like a passive piezo. The real firmware keeps `digitalWrite`.
- **Simulated vs. bench:** the simulation's gas sensor is an MQ-2 model;
  the bench runs an MQ-9, so `gasRaw`/baseline numbers won't match. The
  simulation also skips heater warm-up and AO-voltage safety.
- **Differences found:** none yet — log any here when the simulation and
  the bench disagree.
