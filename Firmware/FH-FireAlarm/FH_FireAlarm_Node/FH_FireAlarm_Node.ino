/*
  ============================================================================
  FH-FireAlarm Node -- Phase B: WiFi + Modbus TCP slave
  ============================================================================
  STATUS (2026-10-06): mirrors BSH-FireAlarm's bench-confirmed Phase B sketch
  exactly -- same components, same GPIO map, same logic -- with ONE difference:
  the static IP is 192.168.50.22, NOT 192.168.50.12 (BSH-FireAlarm's). Two FH
  sketches have already shipped with a copy-pasted BSH address, so check the
  Serial output for .22 before testing. Replaces the old FH Phase A standalone
  sketch in place (same logic as BSH's Phase A, see BSH-FireAlarm/SYSTEM_REPORT.md).
  Phase B bench-confirmed on FH hardware 2026-10-06 at 192.168.50.22. Results in
  ESP32_Core_Modbus/Build_Log.md. Built ahead of the BSH build order at Luis's
  request (BSH-Door and the rest of BSH are not finished).

  Modbus register map (per FH_Systems/02_FH-FireAlarm.md):
    DI0  MANUAL_PULL_TRIGGERED  pull station pressed
    DI1  FLAME_DETECTED         flame sensor tripped
    DI2  GAS_SMOKE_ALERT        gas reading above baseline + margin
    IR0  GAS_RAW_VALUE          raw ADC reading 0-4095
    IR1  HEARTBEAT              counts up once a second (wraps at 65535)
    IR2  RECONNECT_COUNT        WiFi rejoins since boot (0 after a reboot)
    Coil0 ALARM_ACTIVATE        remote/hub-triggered alarm

  Alarm = button OR flame OR gas OR Coil0. Local sources always sound the
  alarm regardless of network state. HEARTBEAT / RECONNECT_COUNT let SCADA
  tell when this ESP32 drops off WiFi (OpenPLC keeps answering with stale
  values, so only a counter this board increments itself proves it is alive).

  FH's own gas sensor may differ from BSH's (BSH's bench uses an MQ-9); check
  the sensor's AO voltage is under ~3.3V for GPIO 34 and tune GAS_ALERT_MARGIN
  on this hardware.

  Libraries: ModbusIP_ESP8266. Board: ESP32 Dev Module.
  ASCII only in this file (no em dashes) -- Windows Arduino IDE rule.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

const char* ssid     = "SCT5";
const char* password = "YOUR_WIFI_PASSWORD";

IPAddress ip(192, 168, 50, 22);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

const int DI_MANUAL_PULL_TRIGGERED = 0;
const int DI_FLAME_DETECTED        = 1;
const int DI_GAS_SMOKE_ALERT       = 2;
const int IREG_GAS_RAW_VALUE       = 0;
const int IREG_HEARTBEAT           = 1;
const int IREG_RECONNECT_COUNT     = 2;
const int COIL_ALARM_ACTIVATE      = 0;

ModbusIP mb;
unsigned long lastReconnectTry = 0;
uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
bool wifiWasUp = false;

// ---------------------------------------------------------------------------
// GPIO pin map -- identical to BSH-FireAlarm (FH mirrors BSH exactly)
// ---------------------------------------------------------------------------
const int PIN_BUZZER      = 23;
const int PIN_RGB_RED     = 25;
const int PIN_RGB_GREEN   = 26;
const int PIN_RGB_BLUE    = 27;
const int PIN_PULL_BUTTON = 4;   // plain INPUT, active LOW -- Keyestudio module has its own resistor
const int PIN_FLAME       = 18;  // plain INPUT, active LOW
const int PIN_GAS         = 34;  // ADC1, input-only -- gas/smoke sensor 

// ---------------------------------------------------------------------------
// Debounce state (pull-station button)
// ---------------------------------------------------------------------------
const unsigned long DEBOUNCE_MS = 50;
unsigned long lastButtonChange   = 0;
bool debouncedButtonState        = false;  // true = pressed / triggered
bool rawButtonLast               = false;

// ---------------------------------------------------------------------------
// Gas/smoke calibration & threshold -- same approach as the real firmware
// ---------------------------------------------------------------------------
const int GAS_CALIBRATION_SAMPLES        = 20;
const unsigned long GAS_CALIBRATION_INTERVAL_MS = 100;  // 20 samples x 100ms = 2s real averaging (Phase A test used 0)
const int GAS_ALERT_MARGIN               = 400;  // placeholder -- tune on real hardware
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
  // FH's MQ-4 was found (2026-11-16, Kali/Ignition bench session) to still be
  // climbing well past a 2s calibration window -- a fixed baseline grabbed
  // that early false-alarms for minutes. Fix: wait for the reading to stop
  // moving before calibrating, instead of trusting a fixed delay.
  // STABLE_WINDOW_COUNTS/STABLE_READINGS_NEEDED/WARMUP_TIMEOUT_MS are
  // first-pass placeholders -- tune from the warm-up Serial log on this
  // hardware, same as GAS_ALERT_MARGIN below.
  const int STABLE_WINDOW_COUNTS        = 15;     // max swing between two readings to count as "settled"
  const int STABLE_READINGS_NEEDED      = 5;      // consecutive settled readings (seconds) required
  const unsigned long WARMUP_TIMEOUT_MS = 120000; // give up and calibrate anyway rather than hang forever

  Serial.println("Waiting for gas/smoke sensor to warm up (keep sensor in clean air)...");
  int lastReading = analogRead(PIN_GAS);
  int stableCount = 0;
  unsigned long warmupStart = millis();

  while (stableCount < STABLE_READINGS_NEEDED && (millis() - warmupStart) < WARMUP_TIMEOUT_MS) {
    // Keep answering Modbus during the wait -- otherwise the PLC sees this
    // node go quiet for the whole warm-up (same symptom as the WiFi-order
    // bug this was meant to fix). 10x 100ms + mb.task() beats one delay(1000).
    // 2026-11-16, second pass: mb.task() alone wasn't enough -- Ignition's
    // Status is driven by HEARTBEAT staleness (Device UDT rule), and
    // heartbeat only ticks in loop(), which hasn't started yet. Confirmed on
    // the bench: Status blipped 1 then 2 during this wait, back to 0 once
    // loop() resumed. Tick heartbeat here too, once per outer pass (~1 Hz,
    // same cadence as loop()), so SCADA sees this node as alive throughout.
    for (int i = 0; i < 10; i++) {
      mb.task();
      delay(100);
    }
    heartbeat++;
    mb.Ireg(IREG_HEARTBEAT, heartbeat);
    int reading = analogRead(PIN_GAS);
    int delta = abs(reading - lastReading);
    Serial.print("  warm-up reading=");
    Serial.print(reading);
    Serial.print(" delta=");
    Serial.print(delta);
    if (delta < STABLE_WINDOW_COUNTS) {
      stableCount++;
      Serial.print("  settled(");
      Serial.print(stableCount);
      Serial.println(")");
    } else {
      stableCount = 0;
      Serial.println("  still drifting, reset settle count");
    }
    lastReading = reading;
  }
  if ((millis() - warmupStart) >= WARMUP_TIMEOUT_MS) {
    Serial.println("WARNING: gas sensor never settled within the warm-up timeout -- calibrating anyway.");
  }

  Serial.println("Calibrating gas/smoke baseline (keep sensor in clean air, ~2s)...");
  long total = 0;
  for (int i = 0; i < GAS_CALIBRATION_SAMPLES; i++) {
    mb.task();  // keep answering Modbus through this ~2s averaging pass too
    // ~2s total (20 x 100ms) is under the 3s yellow threshold on its own,
    // but tick heartbeat at the 1s mark anyway so it never drifts stale
    // even if GAS_CALIBRATION_INTERVAL_MS is changed later.
    if (i == 9) {
      heartbeat++;
      mb.Ireg(IREG_HEARTBEAT, heartbeat);
    }
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
  Serial.println("=== FH-FireAlarm Node (Phase B: WiFi + Modbus) ===");

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  pinMode(PIN_PULL_BUTTON, INPUT);  // plain INPUT -- Keyestudio module supplies its own pull resistor
  pinMode(PIN_FLAME, INPUT);
  // PIN_GAS is analog-only (ADC1) -- no pinMode() call needed for analogRead()

  digitalWrite(PIN_BUZZER, LOW);
  setRgb(false, true, false);   // boot state: steady green = normal

  // 2026-11-16: join WiFi and bring Modbus up FIRST, before the gas warm-up
  // wait, which can now take up to 120s (see calibrateGasSensor()). With the
  // old 2-second calibration the order didn't matter; now, calibrating first
  // meant OpenPLC saw this node as "disconnected" for the whole warm-up on
  // every boot/reflash. Connect and start serving Modbus, THEN calibrate.
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected. IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED - check SSID/password and AP range.");
    Serial.println("Local alarm logic still works; Modbus will not until WiFi joins.");
  }

  mb.server();
  mb.addIsts(DI_MANUAL_PULL_TRIGGERED, false);
  mb.addIsts(DI_FLAME_DETECTED, false);
  mb.addIsts(DI_GAS_SMOKE_ALERT, false);
  mb.addIreg(IREG_GAS_RAW_VALUE, 0);
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  wifiWasUp = (WiFi.status() == WL_CONNECTED);
  mb.addCoil(COIL_ALARM_ACTIVATE, false);

  // Modbus is already answering (gasRaw=0 / gasSmokeAlert=false until this
  // finishes) by the time this runs -- calibrateGasSensor() now calls
  // mb.task() itself throughout the warm-up wait, so the PLC keeps seeing
  // this node connected the whole time instead of going quiet.
  calibrateGasSensor();

  Serial.println("Setup complete. Modbus slave ready.");
  Serial.println();
}

void loop() {
  mb.task();  // always first, every iteration -- never block this loop
  unsigned long now = millis();

  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  if (!wifiUp && now - lastReconnectTry >= 5000) {
    lastReconnectTry = now;
    WiFi.reconnect();
  }
  if (wifiUp && !wifiWasUp) {
    reconnectCount++;
    mb.Ireg(IREG_RECONNECT_COUNT, reconnectCount);
    Serial.print("WiFi rejoined. reconnectCount=");
    Serial.println(reconnectCount);
  } else if (!wifiUp && wifiWasUp) {
    Serial.println("WiFi link lost.");
  }
  wifiWasUp = wifiUp;

  // Heartbeat: +1 every second, non-blocking
  if (now - lastHeartbeatTick >= 1000) {
    lastHeartbeatTick = now;
    heartbeat++;
    mb.Ireg(IREG_HEARTBEAT, heartbeat);
  }

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
  // Combine trigger sources (OR logic): three physical signals + remote coil
  // -------------------------------------------------------------------
  bool remoteAlarm = mb.Coil(COIL_ALARM_ACTIVATE);
  bool alarmActive = debouncedButtonState || flameDetected || gasSmokeAlert || remoteAlarm;

  // Publish raw truth, each signal separately, for SCADA to correlate
  mb.Ists(DI_MANUAL_PULL_TRIGGERED, debouncedButtonState);
  mb.Ists(DI_FLAME_DETECTED, flameDetected);
  mb.Ists(DI_GAS_SMOKE_ALERT, gasSmokeAlert);
  mb.Ireg(IREG_GAS_RAW_VALUE, gasRaw);

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
    Serial.print(")  remoteCoil=");
    Serial.print(remoteAlarm ? "1" : "0");
    Serial.print("  hb=");
    Serial.print(heartbeat);
    Serial.print(" rc=");
    Serial.print(reconnectCount);
    Serial.print("  ==> alarmActive=");
    Serial.println(alarmActive ? "TRUE (buzzer on, red blink)" : "false (green)");
  }
}
