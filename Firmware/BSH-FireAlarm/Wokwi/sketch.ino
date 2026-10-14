/*
  ============================================================================
  BSH-FireAlarm -- Wokwi Simulation (Phase A0)
  ============================================================================
  Purpose: learn how the Fire Alarm system works -- what each sensor outputs,
  how it's wired, and how the alarm logic reacts -- in the Wokwi simulator,
  before (or alongside) the real bench build. Rule: Firmware/WORKFLOW.md
  Section 7.

  This is the same logic as the real standalone test sketch,
  Firmware/BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino -- same
  pin map, same debounce, same gas baseline calibration, same OR logic,
  same LED/buzzer behavior. The ONLY simulation-specific change is marked
  with WOKWI_SIM below:

    - Buzzer: Wokwi's buzzer behaves like a PASSIVE piezo and stays silent
      on a constant HIGH, so in simulation it's driven with tone()/noTone().
      The real bench uses an ACTIVE buzzer, where digitalWrite(HIGH) is
      enough. Set WOKWI_SIM to 0 (or use the real sketch) on real hardware.

  Parts in diagram.json (same GPIOs as BSH_Systems/02_BSH-FireAlarm.md):
    GPIO 23  buzzer            (wokwi-buzzer)
    GPIO 25  RGB red           (wokwi-rgb-led, common cathode, 220R)
    GPIO 26  RGB green         (220R)
    GPIO 27  RGB blue          (220R)
    GPIO 4   pull station      (wokwi-pushbutton + 10k pull-up, active LOW)
    GPIO 18  flame sensor D0   (STAND-IN: wokwi-slide-switch, left = flame)
    GPIO 34  gas/smoke AO      (wokwi-gas-sensor pin AOUT, MQ-2 model)

  No WiFi, no Modbus, no libraries beyond the ESP32 core.
  ============================================================================
*/

#define WOKWI_SIM 1   // 1 = running in Wokwi, 0 = real hardware

// ---------------------------------------------------------------------------
// GPIO pin map -- identical to the full BSH-FireAlarm node firmware
// ---------------------------------------------------------------------------
const int PIN_BUZZER      = 23;
const int PIN_RGB_RED     = 25;
const int PIN_RGB_GREEN   = 26;
const int PIN_RGB_BLUE    = 27;
const int PIN_PULL_BUTTON = 4;   // plain INPUT, active LOW -- pull-up supplied externally (10k in the sim, the module's own resistor on the bench)
const int PIN_FLAME       = 18;  // plain INPUT, active LOW
const int PIN_GAS         = 34;  // ADC1, input-only -- gas/smoke sensor

#if WOKWI_SIM
const int BUZZER_TONE_HZ  = 2000;  // simulation only -- see header
#endif

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
const unsigned long GAS_CALIBRATION_INTERVAL_MS = 0;
const int GAS_ALERT_MARGIN               = 400;  // placeholder -- tune on real hardware
int gasBaseline                          = 0;

// ---------------------------------------------------------------------------
// Non-blocking alarm blink state
// ---------------------------------------------------------------------------
const unsigned long BLINK_INTERVAL_MS = 300;
unsigned long lastBlinkToggle          = 0;
bool blinkOn                           = false;

// ---------------------------------------------------------------------------
// Status print throttle (once a second)
// ---------------------------------------------------------------------------
const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint                = 0;

bool buzzerIsOn = false;

void setBuzzer(bool on) {
  if (on == buzzerIsOn) return;   // only act on changes (restarting tone() every loop would stutter)
  buzzerIsOn = on;
#if WOKWI_SIM
  if (on) tone(PIN_BUZZER, BUZZER_TONE_HZ);
  else    noTone(PIN_BUZZER);
#else
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
#endif
}

void setRgb(bool r, bool g, bool b) {
  digitalWrite(PIN_RGB_RED,   r ? HIGH : LOW);
  digitalWrite(PIN_RGB_GREEN, g ? HIGH : LOW);
  digitalWrite(PIN_RGB_BLUE,  b ? HIGH : LOW);
}

void calibrateGasSensor() {
  Serial.println("Calibrating gas/smoke baseline...");
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
  if (gasBaseline + GAS_ALERT_MARGIN > 4095) {
    Serial.println("WARNING: baseline too high for the alert to ever fire (ADC max is 4095).");
    Serial.println("         Lower the gas sensor's ppm in diagram.json and restart.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-FireAlarm -- Wokwi Simulation (Phase A0, no WiFi/Modbus) ===");

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  pinMode(PIN_PULL_BUTTON, INPUT);  // plain INPUT -- external pull-up
  pinMode(PIN_FLAME, INPUT);
  // PIN_GAS is analog-only (ADC1) -- no pinMode() needed for analogRead()

  setBuzzer(false);
  digitalWrite(PIN_BUZZER, LOW);
  setRgb(false, true, false);   // boot state: steady green = normal

  calibrateGasSensor();

  Serial.println("Setup complete. Try: press the red button, slide the flame switch");
  Serial.println("LEFT, or raise the gas sensor's level. Watch the LED and buzzer.");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  // Debounced pull-station button read (active LOW)
  bool rawPressed = (digitalRead(PIN_PULL_BUTTON) == LOW);
  if (rawPressed != rawButtonLast) {
    lastButtonChange = now;
    rawButtonLast = rawPressed;
  }
  if (now - lastButtonChange > DEBOUNCE_MS) {
    debouncedButtonState = rawPressed;
  }

  // Flame sensor (active LOW) -- slide switch LEFT = LOW = flame
  bool flameDetected = (digitalRead(PIN_FLAME) == LOW);

  // Gas/smoke sensor -- alert when the reading drifts above the boot baseline
  int gasRaw = analogRead(PIN_GAS);
  bool gasSmokeAlert = (gasRaw > gasBaseline + GAS_ALERT_MARGIN);

  // OR all local trigger sources (the real firmware also ORs the remote
  // Modbus coil ALARM_ACTIVATE -- not present until Phase B)
  bool alarmActive = debouncedButtonState || flameDetected || gasSmokeAlert;

  // Drive outputs
  if (alarmActive) {
    setBuzzer(true);
    if (now - lastBlinkToggle >= BLINK_INTERVAL_MS) {
      lastBlinkToggle = now;
      blinkOn = !blinkOn;
      setRgb(blinkOn, false, false);   // blinking red
    }
  } else {
    setBuzzer(false);
    setRgb(false, true, false);        // steady green = normal
  }

  // Status print, once a second
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
