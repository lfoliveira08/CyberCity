/*
 * FH-HVAC Node -- Phase B: WiFi + Modbus TCP slave
 * ============================================================================
 * STATUS (2026-10-07): Phase A confirmed on the bench the same day (idle, cooling,
 * HVAC_ENABLE; see Firmware/FH-HVAC/SYSTEM_REPORT.md). This is the Phase B upgrade:
 * same field-layer logic, now a Modbus TCP slave. Phase B bench-confirmed
 * 2026-10-07 at 192.168.50.21 (see the Build_Log).
 * Bring-up checklist: Firmware/FH-HVAC/ESP32_Core_Modbus/Build_Log.md
 *
 * Role: Founder's Hall zone HVAC unit. Reads a DHT11, compares the temperature to
 * a setpoint written over Modbus, drives a relay (fan motor) plus an RGB status LED.
 *
 * Architecture: plain Modbus TCP SLAVE. OpenPLC-FH-HVAC polls this ESP32 directly at
 * 192.168.50.21:502. No bridge. (FH_Systems/01_FH-HVAC.md, Piranga_Workflow_v2.md S6)
 *
 * Modbus register map (FH_Systems/01_FH-HVAC.md):
 *   Input Register  IR0   = TEMP_C             tenths of a degree C (24.0 C -> 240)
 *   Input Register  IR1   = HUMIDITY_PCT       whole percent
 *   Input Register  IR2   = HEARTBEAT          +1 per second, wraps at 65535
 *   Input Register  IR3   = RECONNECT_COUNT    WiFi rejoins since boot
 *   Discrete Input  DI0   = COMPRESSOR_RUNNING relay state, reported honestly
 *   Discrete Input  DI1   = HVAC_FAULT         latched on repeated bad sensor reads
 *   Holding Reg     HR0   = TARGET_TEMP_C      tenths of a degree C (master-written)
 *   Coil            Coil0 = HVAC_ENABLE        master-written; clear = unit forced off
 *
 * Decisions (settled 2026-10-07):
 *   - Scaling: tenths for TEMP_C and TARGET_TEMP_C (same scale). Humidity whole %.
 *     The bench sensor is a DHT11 (whole degrees), so TEMP_C is always a multiple of 10.
 *   - Fail direction: FAIL-OFF. Relay opens if HVAC_FAULT is latched, HVAC_ENABLE is
 *     clear, OR the WiFi link has been down longer than COMMS_LOSS_MS.
 *   - On a sensor fault TEMP_C holds its LAST GOOD value (never 0). HVAC_FAULT (DI1)
 *     is what tells the master the value is stale.
 *   - HR0 outside 10.0-35.0 C is clamped for control; the register itself is not rewritten.
 *   - Known limit: comms loss is detected from the WiFi link only. A master that stops
 *     polling while WiFi stays up is NOT detected yet (SCADA watches HEARTBEAT instead).
 *
 * Hardware: DHT11 DATA GPIO 4; relay IN GPIO 26 (active-HIGH, bench-confirmed);
 *           RGB R/G/B GPIO 16/17/18 (220 ohm each, common cathode).
 * LED: green idle, blue cooling, yellow disabled/comms lost, red FAULT.
 * Serial (115200): e = toggle HVAC_ENABLE, + / - = setpoint +/-1.0 C (writes the registers).
 *
 * Libraries: ModbusIP_ESP8266 (works on ESP32), "DHT sensor library" + "Adafruit Unified Sensor".
 * Board: "ESP32 Dev Module". ASCII only in this file.
 * ============================================================================
 */

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <DHT.h>

// ---------- WiFi (same network as the other nodes) ----------
const char* ssid     = "SCT5";
const char* password = "S3n!0rC@p";

// ---------- Static IP -- must match Piranga_Workflow_v2.md Section 6 ----------
IPAddress ip(192, 168, 50, 21);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------- Modbus addresses ----------
const int IREG_TEMP_C          = 0;
const int IREG_HUMIDITY_PCT    = 1;
const int IREG_HEARTBEAT       = 2;
const int IREG_RECONNECT_COUNT = 3;
const int ISTS_COMPRESSOR_RUN  = 0;
const int ISTS_HVAC_FAULT      = 1;
const int HREG_TARGET_TEMP_C   = 0;
const int COIL_HVAC_ENABLE     = 0;

// ---------- Pins ----------
const int PIN_DHT   = 4;
const int PIN_RELAY = 26;
const int PIN_LED_R = 16;
const int PIN_LED_G = 17;
const int PIN_LED_B = 18;

// Bench-confirmed 2026-10-07: keyestudio relay module is active-HIGH.
const bool RELAY_ACTIVE_LOW = false;

// ---------- Tuning ----------
const unsigned long SAMPLE_INTERVAL_MS = 2000;
const int  FAULT_AFTER_FAILS    = 3;
const int  HYSTERESIS_TENTHS    = 5;
const int  TARGET_MIN_TENTHS    = 100;
const int  TARGET_MAX_TENTHS    = 350;
const int  DEFAULT_TARGET_TENTHS = 240;
const int  TEMP_VALID_MIN_TENTHS = 0;     // DHT11 spec range
const int  TEMP_VALID_MAX_TENTHS = 500;
const unsigned long COMMS_LOSS_MS = 10000; // WiFi down this long -> fail-off

DHT dht(PIN_DHT, DHT11);
ModbusIP mb;

// ---------- State ----------
int  tempTenths  = 0;
int  humidityPct = 0;
bool haveReading = false;
bool cooling     = false;
bool faulted     = false;
int  failCount   = 0;
bool commsLost   = false;
unsigned long lastSampleMs = 0;

uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
unsigned long lastReconnectTry = 0;
unsigned long wifiDownSince = 0;
bool wifiWasUp = false;

// ---------- Helpers ----------
void setRelay(bool on) {
  digitalWrite(PIN_RELAY, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r ? HIGH : LOW);
  digitalWrite(PIN_LED_G, g ? HIGH : LOW);
  digitalWrite(PIN_LED_B, b ? HIGH : LOW);
}

void printTenths(int v) {
  if (v < 0) { Serial.print('-'); v = -v; }
  Serial.print(v / 10);
  Serial.print('.');
  Serial.print(v % 10);
}

int targetTenths() {
  int t = (int)mb.Hreg(HREG_TARGET_TEMP_C);
  if (t < TARGET_MIN_TENTHS) t = TARGET_MIN_TENTHS;
  if (t > TARGET_MAX_TENTHS) t = TARGET_MAX_TENTHS;
  return t;
}

void updateOutputs() {
  bool enabled = mb.Coil(COIL_HVAC_ENABLE) && !commsLost;
  int target = targetTenths();

  if (faulted || !enabled || !haveReading) {
    cooling = false;                       // fail-off
  } else if (!cooling && tempTenths >= target + HYSTERESIS_TENTHS) {
    cooling = true;
  } else if (cooling && tempTenths <= target - HYSTERESIS_TENTHS) {
    cooling = false;
  }
  setRelay(cooling);

  if (faulted)         setRGB(true,  false, false);  // red
  else if (!enabled)   setRGB(true,  true,  false);  // yellow
  else if (cooling)    setRGB(false, false, true);   // blue
  else                 setRGB(false, true,  false);  // green

  mb.Ists(ISTS_COMPRESSOR_RUN, cooling);
  mb.Ists(ISTS_HVAC_FAULT, faulted);
}

void sampleSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  bool ok = !isnan(t) && !isnan(h);
  int tt = 0;
  if (ok) {
    tt = (int)lroundf(t * 10.0f);
    if (tt < TEMP_VALID_MIN_TENTHS || tt > TEMP_VALID_MAX_TENTHS) ok = false;
  }

  if (ok) {
    failCount = 0;
    tempTenths = tt;
    humidityPct = (int)lroundf(h);
    haveReading = true;
    mb.Ireg(IREG_TEMP_C, (uint16_t)tempTenths);
    mb.Ireg(IREG_HUMIDITY_PCT, (uint16_t)humidityPct);
  } else {
    failCount++;
    if (failCount >= FAULT_AFTER_FAILS && !faulted) {
      faulted = true;
      Serial.println("HVAC_FAULT latched: sensor read failed repeatedly. Relay forced open. Reset to clear.");
    }
  }

  updateOutputs();

  Serial.print("T=");
  if (ok) printTenths(tempTenths); else Serial.print("---");
  Serial.print("C RH=");
  if (ok) Serial.print(humidityPct); else Serial.print("--");
  Serial.print("% target=");
  printTenths(targetTenths());
  Serial.print("C enable=");  Serial.print(mb.Coil(COIL_HVAC_ENABLE) ? 1 : 0);
  Serial.print(" cooling=");  Serial.print(cooling ? 1 : 0);
  Serial.print(" fault=");    Serial.print(faulted ? 1 : 0);
  Serial.print(" wifi=");     Serial.print(WiFi.status() == WL_CONNECTED ? 1 : 0);
  Serial.print(" commsLost=");Serial.println(commsLost ? 1 : 0);
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'e') {
      bool now = !mb.Coil(COIL_HVAC_ENABLE);
      mb.Coil(COIL_HVAC_ENABLE, now);
      Serial.print("HVAC_ENABLE -> "); Serial.println(now ? 1 : 0);
      updateOutputs();
    } else if (c == '+' || c == '-') {
      int t = (int)mb.Hreg(HREG_TARGET_TEMP_C) + ((c == '+') ? 10 : -10);
      if (t > TARGET_MAX_TENTHS) t = TARGET_MAX_TENTHS;
      if (t < TARGET_MIN_TENTHS) t = TARGET_MIN_TENTHS;
      mb.Hreg(HREG_TARGET_TEMP_C, (uint16_t)t);
      Serial.print("TARGET_TEMP_C -> "); printTenths(t); Serial.println("C");
      updateOutputs();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  setRelay(false);          // known-off before anything else
  setRGB(false, false, false);
  dht.begin();

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
    Serial.println("\nWiFi FAILED to connect - check SSID/password and AP range.");
    Serial.println("Unit stays failed-off (comms lost) until WiFi joins.");
  }

  mb.server();
  mb.addIreg(IREG_TEMP_C, 0);
  mb.addIreg(IREG_HUMIDITY_PCT, 0);
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  mb.addIsts(ISTS_COMPRESSOR_RUN, false);
  mb.addIsts(ISTS_HVAC_FAULT, false);
  mb.addHreg(HREG_TARGET_TEMP_C, DEFAULT_TARGET_TENTHS);
  mb.addCoil(COIL_HVAC_ENABLE, true);   // enabled at boot; the master may clear it

  wifiWasUp = (WiFi.status() == WL_CONNECTED);
  wifiDownSince = millis();
  updateOutputs();
  lastSampleMs = millis();
  Serial.println("FH-HVAC node ready (Phase B: WiFi + Modbus). Commands: e=enable toggle, +/- = setpoint");
}

void loop() {
  mb.task();  // ALWAYS call first -- services Modbus TCP requests

  // --- WiFi link tracking, comms-loss (fail-off), heartbeat (non-blocking) ---
  {
    unsigned long t = millis();
    bool wifiUp = (WiFi.status() == WL_CONNECTED);
    if (!wifiUp && t - lastReconnectTry >= 5000) {
      lastReconnectTry = t;
      WiFi.reconnect();
    }
    if (wifiUp && !wifiWasUp) {
      reconnectCount++;
      mb.Ireg(IREG_RECONNECT_COUNT, reconnectCount);
      Serial.print("WiFi rejoined. reconnectCount=");
      Serial.println(reconnectCount);
    } else if (!wifiUp && wifiWasUp) {
      wifiDownSince = t;
      Serial.println("WiFi link lost.");
    }
    wifiWasUp = wifiUp;

    bool lost = !wifiUp && (t - wifiDownSince >= COMMS_LOSS_MS);
    if (lost != commsLost) {
      commsLost = lost;
      Serial.println(commsLost ? "COMMS LOST: failing off." : "Comms restored.");
    }

    if (t - lastHeartbeatTick >= 1000) {
      lastHeartbeatTick = t;
      heartbeat++;
      mb.Ireg(IREG_HEARTBEAT, heartbeat);
    }
  }

  handleSerial();

  // Re-evaluate every pass so a Modbus write to HR0 / Coil0 takes effect immediately.
  updateOutputs();

  unsigned long now = millis();
  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;
    sampleSensor();
  }
}
