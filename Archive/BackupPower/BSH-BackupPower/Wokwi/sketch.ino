/*
  ============================================================================
  BSH-BackupPower -- Wokwi Simulation (Phase A0)
  ============================================================================
  Purpose: learn how the Backup Power / ATS Monitor system works -- what each
  input reports, how it is wired, and how the four signals stay independent --
  in the Wokwi simulator before the real bench build. Rule: Firmware/WORKFLOW.md
  Section 7.

  Same pin map as BSH_Systems/05_BSH-BackupPower.md. No WiFi, no Modbus, no
  libraries beyond the ESP32 core. Register names are printed so the Phase B
  mapping is obvious:

    GPIO 26  UTILITY_POWER_LOST   (wokwi-slide-switch, INPUT_PULLUP, active LOW)
    GPIO 27  ATS_TRANSFER_CMD out (wokwi-relay-module -> green LED "Demo load")
    GPIO 34  UPS_BATTERY_PCT      (STAND-IN: potentiometer, ADC1)
    GPIO 35  LOAD_CURRENT_MA      (STAND-IN: potentiometer, ADC1)

  Simulation-only differences:
    - ATS_TRANSFER_CMD is a Modbus coil on the real node. There is no network
      here, so type  1  or  0  (or  t  to toggle) in the Serial Monitor and
      press Enter to stand in for the master writing Coil0.
    - Wokwi's relay module energizes on HIGH (found on BSH-PowerBox). Real
      modules vary -- RELAY_ACTIVE_LOW is a flag so the bench can flip it.
    - The battery % and current scaling below are PLACEHOLDERS. Real values
      need the actual divider R1/R2, pack voltage and ACS712 variant.

  Like the real node, nothing is fused locally: the four signals are reported
  raw and independently. Judging "transfer commanded with no outage" is the
  SCADA layer's job, not this sketch's.
  ============================================================================
*/

// ---------------------------------------------------------------------------
// Pin map
// ---------------------------------------------------------------------------
const int PIN_UTILITY_SWITCH = 26;  // INPUT_PULLUP, LOW = utility power lost
const int PIN_ATS_RELAY      = 27;  // output
const int PIN_BATTERY        = 34;  // ADC1, input-only
const int PIN_CURRENT        = 35;  // ADC1, input-only

// ---------------------------------------------------------------------------
// Relay polarity -- Wokwi's relay energizes on HIGH. CHECK ON REAL HARDWARE.
// ---------------------------------------------------------------------------
const bool RELAY_ACTIVE_LOW = false;

// ---------------------------------------------------------------------------
// Placeholders pending bench calibration
// ---------------------------------------------------------------------------
const int  LOW_BATTERY_PCT   = 20;    // LOW_BATTERY when UPS_BATTERY_PCT < this
const int  CURRENT_ZERO_RAW  = 2048;  // ADC counts at zero current (pot mid-point)
const float MA_PER_COUNT     = 4.36;  // 5A ACS712 (185 mV/A), no divider: 3.3V/4095/0.185
const int  CURRENT_MAX_MA    = 5000;  // clamp to the sensor's range

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
bool atsTransferCmd = false;           // stands in for Coil0

const unsigned long DEBOUNCE_MS = 50;
unsigned long lastSwitchChange  = 0;
bool rawLostLast                = false;
bool utilityPowerLost           = false;  // debounced

const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint = 0;

void setRelay(bool on) {
  digitalWrite(PIN_ATS_RELAY, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

int readBatteryPct() {
  int raw = analogRead(PIN_BATTERY);
  return (int)((long)raw * 100 / 4095);
}

int readLoadMa() {
  int raw = analogRead(PIN_CURRENT);
  long counts = raw - CURRENT_ZERO_RAW;
  if (counts < 0) counts = -counts;
  long ma = (long)(counts * MA_PER_COUNT);
  if (ma > CURRENT_MAX_MA) ma = CURRENT_MAX_MA;
  return (int)ma;
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1')      atsTransferCmd = true;
    else if (c == '0') atsTransferCmd = false;
    else if (c == 't' || c == 'T') atsTransferCmd = !atsTransferCmd;
    else continue;   // ignore newline etc.
    Serial.print(">> ATS_TRANSFER_CMD (Coil0) = ");
    Serial.println(atsTransferCmd ? "1 (transfer commanded)" : "0 (not commanded)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-BackupPower -- Wokwi Simulation (Phase A0, no WiFi/Modbus) ===");

  pinMode(PIN_UTILITY_SWITCH, INPUT_PULLUP);  // GPIO 26 has an internal pull-up
  pinMode(PIN_ATS_RELAY, OUTPUT);
  setRelay(false);  // boot state: no transfer commanded
  // PIN_BATTERY / PIN_CURRENT are ADC1 input-only -- no pinMode() needed

  Serial.println("Try: flip the utility switch LEFT, turn the two potentiometers,");
  Serial.println("and type 1 / 0 / t + Enter to command the ATS transfer relay.");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  handleSerialCommands();

  // Utility-loss switch, debounced (active LOW)
  bool rawLost = (digitalRead(PIN_UTILITY_SWITCH) == LOW);
  if (rawLost != rawLostLast) {
    lastSwitchChange = now;
    rawLostLast = rawLost;
  }
  if (now - lastSwitchChange > DEBOUNCE_MS) {
    utilityPowerLost = rawLost;
  }

  // Analog signals -- both always reported raw
  int batteryPct = readBatteryPct();
  bool lowBattery = (batteryPct < LOW_BATTERY_PCT);
  int loadMa = readLoadMa();

  // ATS relay follows the (simulated) coil only -- no local fusing
  setRelay(atsTransferCmd);

  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("DI0 UTILITY_POWER_LOST=");
    Serial.print(utilityPowerLost ? "YES" : "no");
    Serial.print("  DI1 LOW_BATTERY=");
    Serial.print(lowBattery ? "YES" : "no");
    Serial.print("  IR0 UPS_BATTERY_PCT=");
    Serial.print(batteryPct);
    Serial.print("  IR1 LOAD_CURRENT_MA=");
    Serial.print(loadMa);
    Serial.print("  Coil0 ATS_TRANSFER_CMD=");
    Serial.println(atsTransferCmd ? "1 (relay ON, load lit)" : "0 (relay off)");
  }
}
