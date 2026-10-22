/*
  ============================================================================
  FH-PowerBox Node -- Phase B: WiFi + Modbus TCP slave
  ============================================================================
  STATUS (2026-10-06): mirrors BSH-PowerBox's bench-confirmed Phase B sketch --
  same pins, logic and registers. Phase B bench-confirmed on FH hardware
  2026-10-07 at 192.168.50.24. Results in ESP32_Core_Modbus/Build_Log.md. Built
  ahead of the BSH build order at Luis's request. ONE difference: static IP 192.168.50.24, NOT .14 (BSH's).
  Check the Serial output says .24.

  RELAY POLARITY: FH's boards were bench-confirmed active-HIGH on 2026-10-07 (both
  outputs OFF at boot with both flags false). Polarity belongs to the physical
  board, so a replacement board must be re-checked. At boot both outputs must be OFF. If a
  light or the fan comes ON at boot, or follows commands backwards, flip
  RELAY_LIGHTS_ACTIVE_LOW / RELAY_FAN_ACTIVE_LOW below and re-flash.

  CHANGED 2026-10-21 (Phase C): hub-write rule is now "different value only"
  -- see AUTHORITY RULE below. Applied ahead of FH Phase C so the node does not
  repeat the BSH button-override problem. NOT yet flashed or bench-tested.

  Architecture: plain Modbus TCP SLAVE at 192.168.50.24:502. OpenPLC-FH-PowerBox
  (192.168.50.124) will poll it directly in Phase C.

  Modbus register map (per FH_Systems/04_FH-PowerBox.md):
    Coil0  LIGHTS_ON_CMD   (master -> slave)
    Coil1  FAN_CMD         (master -> slave)
    DI0    LIGHTS_STATUS   (slave -> master) actual physical state
    DI1    FAN_STATUS      (slave -> master) actual physical state
    IR0    HEARTBEAT       (+1 per second, wraps at 65535)
    IR1    RECONNECT_COUNT (WiFi rejoins since boot; 0 after a reboot)

  Hardware (same as BSH-PowerBox):
    GPIO 13 -> Lights relay IN
    GPIO 15 -> Fan relay IN
    GPIO 4  -> Lights override button (INPUT_PULLUP, other leg to GND)
    GPIO 16 -> Fan override button    (INPUT_PULLUP, other leg to GND)

  AUTHORITY RULE (whichever source acted most recently wins):
    - A debounced button press toggles the output immediately.
    - The hub only regains control when it writes a coil value that is
      DIFFERENT from the last value it wrote (detected in the onSetCoil
      callback). Changed Oct 21, 2026 (Phase C), same as BSH-PowerBox:
      OpenPLC rewrites the coil on every poll (about 100 ms), so the old rule
      ("every write counts, even the same value") let the PLC undo every
      button press within one poll. Now a repeated write of the same value is
      ignored, so a button press sticks until SCADA sends a different command.
      The previous behavior is archived in
      Archive/FH_PowerBox_Node_PhaseB_every-write-wins.ino.
    - Cost: the hub cannot re-force the SAME value after a button press; it
      must send the opposite value and then the original.
    - After a WiFi rejoin the "last hub value" is forgotten, so the first
      write after reconnect counts.
    - Status DIs always report the real physical state.

  FAIL-SAFE: if WiFi stays down longer than WATCHDOG_MS, both outputs go OFF.
  Buttons keep working locally. The watchdog detects WiFi LINK loss, not a
  silent master.

  Libraries: ModbusIP_ESP8266. Board: ESP32 Dev Module.
  ASCII only in this file (no em dashes) -- Windows Arduino IDE rule.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

// ---------------------------------------------------------------------------
// WiFi + static IP (must match Piranga_Workflow_v2.md Section 6)
// ---------------------------------------------------------------------------
const char* ssid     = "SCT5";
const char* password = "S3n!0rC@p";

IPAddress ip(192, 168, 50, 24);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus addresses
// ---------------------------------------------------------------------------
const int COIL_LIGHTS_ON_CMD = 0;
const int COIL_FAN_CMD       = 1;
const int DI_LIGHTS_STATUS   = 0;
const int DI_FAN_STATUS      = 1;
const int IREG_HEARTBEAT     = 0;
const int IREG_RECONNECT_COUNT = 1;

// ---------------------------------------------------------------------------
// Pins and relay polarity (CHECK ON FH BOARDS -- BSH result was active-HIGH)
// ---------------------------------------------------------------------------
const int PIN_LIGHTS_RELAY  = 13;
const int PIN_FAN_RELAY     = 15;
const int PIN_LIGHTS_BUTTON = 4;
const int PIN_FAN_BUTTON    = 16;

const bool RELAY_LIGHTS_ACTIVE_LOW = false;
const bool RELAY_FAN_ACTIVE_LOW    = false;

const unsigned long WATCHDOG_MS        = 5000;
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long PRINT_INTERVAL_MS  = 1000;

ModbusIP mb;

bool lightsOn = false;
bool fanOn    = false;

// Set from the Modbus callbacks (run inside mb.task()), consumed in loop().
// lastHub*: last value the hub wrote (-1 = none yet / forgotten on rejoin).
volatile int8_t lightsLastHub = -1;
volatile int8_t fanLastHub    = -1;
volatile bool lightsWritePending = false;
volatile bool lightsWriteValue   = false;
volatile bool fanWritePending    = false;
volatile bool fanWriteValue      = false;

uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
unsigned long wifiDownSince = 0;
bool wifiWasDown = false;
bool failSafeApplied = false;
unsigned long lastPrint = 0;
unsigned long lastReconnectTry = 0;

// Plain per-button variables, not a struct (Arduino auto-prototype bug).
int lightsButtonLastReading = HIGH;
int lightsButtonStableState = HIGH;
unsigned long lightsButtonLastChangeTime = 0;
int fanButtonLastReading = HIGH;
int fanButtonStableState = HIGH;
unsigned long fanButtonLastChangeTime = 0;

void setRelay(int pin, bool on, bool activeLow) {
  bool level = activeLow ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

bool readButtonEdge(int pin, int &lastReading, int &stableState, unsigned long &lastChangeTime) {
  int reading = digitalRead(pin);
  if (reading != lastReading) {
    lastChangeTime = millis();
  }
  bool pressedEdge = false;
  if ((millis() - lastChangeTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) {
        pressedEdge = true;
      }
    }
  }
  lastReading = reading;
  return pressedEdge;
}

// Modbus write callbacks: fire on every write request, even if the value is
// unchanged. Only a write whose value differs from the last hub value counts
// as a fresh command (Oct 21, 2026: OpenPLC rewrites the coil every poll).
uint16_t onLightsCoilWrite(TRegister* reg, uint16_t val) {
  int8_t v = (val != 0) ? 1 : 0;
  if (v != lightsLastHub) {
    lightsLastHub = v;
    lightsWriteValue = (v == 1);
    lightsWritePending = true;
  }
  return val;
}
uint16_t onFanCoilWrite(TRegister* reg, uint16_t val) {
  int8_t v = (val != 0) ? 1 : 0;
  if (v != fanLastHub) {
    fanLastHub = v;
    fanWriteValue = (v == 1);
    fanWritePending = true;
  }
  return val;
}

void applyOutputs() {
  setRelay(PIN_LIGHTS_RELAY, lightsOn, RELAY_LIGHTS_ACTIVE_LOW);
  setRelay(PIN_FAN_RELAY, fanOn, RELAY_FAN_ACTIVE_LOW);
  mb.Ists(DI_LIGHTS_STATUS, lightsOn);
  mb.Ists(DI_FAN_STATUS, fanOn);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== FH-PowerBox Node (Phase B: WiFi + Modbus) ===");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  pinMode(PIN_FAN_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false, RELAY_LIGHTS_ACTIVE_LOW);
  setRelay(PIN_FAN_RELAY, false, RELAY_FAN_ACTIVE_LOW);
  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FAN_BUTTON, INPUT_PULLUP);

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
    Serial.println("Buttons still work locally; Modbus will not until WiFi joins.");
  }

  mb.server();
  mb.addCoil(COIL_LIGHTS_ON_CMD, false);
  mb.addCoil(COIL_FAN_CMD, false);
  mb.addIsts(DI_LIGHTS_STATUS, false);
  mb.addIsts(DI_FAN_STATUS, false);
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  mb.onSetCoil(COIL_LIGHTS_ON_CMD, onLightsCoilWrite);
  mb.onSetCoil(COIL_FAN_CMD, onFanCoilWrite);

  Serial.println("Ready. Both channels boot OFF.");
}

void loop() {
  mb.task();  // always first, every iteration

  // --- WiFi link watchdog + reconnect ---
  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  unsigned long now = millis();
  if (!wifiUp) {
    if (!wifiWasDown) {
      wifiWasDown = true;
      wifiDownSince = now;
      failSafeApplied = false;
      Serial.println("WiFi link lost.");
    }
    if (!failSafeApplied && (now - wifiDownSince) >= WATCHDOG_MS) {
      lightsOn = false;
      fanOn = false;
      failSafeApplied = true;
      Serial.println("WATCHDOG: comm lost, lights and fan forced OFF.");
    }
    if (now - lastReconnectTry >= 5000) {
      lastReconnectTry = now;
      WiFi.reconnect();
    }
  } else if (wifiWasDown) {
    wifiWasDown = false;
    Serial.println("WiFi link restored.");
    reconnectCount++;
    mb.Ireg(IREG_RECONNECT_COUNT, reconnectCount);
    // Drop any stale command state so a held coil cannot silently re-arm.
    mb.Coil(COIL_LIGHTS_ON_CMD, false);
    mb.Coil(COIL_FAN_CMD, false);
    // Forget the last hub values so the first write after rejoin counts.
    lightsLastHub = -1;
    fanLastHub = -1;
  }

  // --- local override buttons: act immediately ---
  if (readButtonEdge(PIN_LIGHTS_BUTTON, lightsButtonLastReading, lightsButtonStableState, lightsButtonLastChangeTime)) {
    lightsOn = !lightsOn;
    Serial.print("Lights button -> ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }
  if (readButtonEdge(PIN_FAN_BUTTON, fanButtonLastReading, fanButtonStableState, fanButtonLastChangeTime)) {
    fanOn = !fanOn;
    Serial.print("Fan button -> ");
    Serial.println(fanOn ? "ON" : "OFF");
  }

  // --- fresh hub writes regain authority ---
  if (lightsWritePending) {
    lightsWritePending = false;
    lightsOn = lightsWriteValue;
    Serial.print("Hub wrote LIGHTS_ON_CMD -> ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }
  if (fanWritePending) {
    fanWritePending = false;
    fanOn = fanWriteValue;
    Serial.print("Hub wrote FAN_CMD -> ");
    Serial.println(fanOn ? "ON" : "OFF");
  }

  applyOutputs();

  // Heartbeat: +1 every second, non-blocking
  if (now - lastHeartbeatTick >= 1000) {
    lastHeartbeatTick = now;
    heartbeat++;
    mb.Ireg(IREG_HEARTBEAT, heartbeat);
  }

  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("lights=");
    Serial.print(lightsOn ? "ON" : "off");
    Serial.print("  fan=");
    Serial.print(fanOn ? "ON" : "off");
    Serial.print("  hb=");
    Serial.print(heartbeat);
    Serial.print(" rc=");
    Serial.print(reconnectCount);
    Serial.print("  wifi=");
    Serial.println(wifiUp ? "up" : "DOWN");
  }
}
