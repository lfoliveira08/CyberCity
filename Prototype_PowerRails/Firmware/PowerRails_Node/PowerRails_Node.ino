/*
  ============================================================================
  PowerRails Node -- PROTOTYPE: WiFi + Modbus TCP slave (draft, Oct 28, 2026)
  ============================================================================
  DRAFT. NOT the real BSH-PowerBox firmware and NOT part of the real design.
  Written from the real Firmware/BSH-PowerBox/BSH_PowerBox_Node/ sketch (same
  structure, same library, same hub-write rule) with the lights, fan and
  buttons removed and three power rails added. Never compiled, never flashed,
  never run. Luis flashes it himself on the PROTOTYPE spare ESP32 only.

  Architecture: Modbus TCP SLAVE on port 502. A PROTOTYPE OpenPLC container
  polls it. It must NOT use a real node address (192.168.50.11-.14, .21-.24) or
  a real container address (.111-.114, .121-.124).

  Modbus register map (Prototype_PowerRails/Register_Map_DRAFT.md):
    Coil0  ELEV_RAIL_CMD   (master -> slave) 1 = Elevator node powered
    Coil1  FIRE_RAIL_CMD   (master -> slave) 1 = FireAlarm node powered
    Coil2  DOOR_RAIL_CMD   (master -> slave) 1 = Door node powered
    DI0    ELEV_RAIL_STATUS  (slave -> master) rail as this node drives it
    DI1    FIRE_RAIL_STATUS
    DI2    DOOR_RAIL_STATUS
    IR0    HEARTBEAT         (+1 per second, wraps at 65535)
    IR1    RECONNECT_COUNT   (WiFi rejoins since boot)

  Hardware (rail-to-pin map by Luis, Oct 28, 2026; prototype only):
    GPIO 25 -> Rail 1 relay IN (Elevator)
    GPIO 26 -> Rail 2 relay IN (FireAlarm)
    GPIO 27 -> Rail 3 relay IN (Door)
  Relay contact: NO (fail-dark). Relay coil on = rail powered. Relay board
  polarity is tested per board; RELAY_ACTIVE_LOW below is the setting.

  HUB-WRITE RULE (same as the real node, changed Oct 21, 2026): a Modbus coil
  write counts as a command only when its value DIFFERS from the last value the
  hub wrote, because OpenPLC rewrites coils about every 100 ms. After a WiFi
  rejoin the last hub value is forgotten so the first write counts.

  FAIL-SAFE (decided by Luis Oct 27, 2026): if the WiFi link stays down longer
  than WATCHDOG_MS, all rails go OFF. This detects WiFi LINK loss only, not a
  silent master or a stopped PLC container (same limit as the real node).

  BOOT: all rails OFF until the hub commands them on (RAILS_ON_AT_BOOT false).
  Open decision 8 in the prototype README (how rails come on after power-up);
  this constant is only the draft default.

  LOCAL TEST: Serial Monitor at 115200, '1' '2' '3' toggle a rail, 'a' cut all,
  'r' restore all. A serial command acts like a local action.

  Libraries: ModbusIP_ESP8266 (works on ESP32). Board: ESP32 Dev Module.
  ASCII only in this file (no em dashes). NO WiFi password or IP is stored in
  this repo: fill the three lines marked FILL IN on your own machine.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

// ---------------------------------------------------------------------------
// WiFi + static IP  --  FILL IN locally, do not commit real values
// ---------------------------------------------------------------------------
const char* ssid     = "SCT5";      // FILL IN
const char* password = "S3n!0rC@p";  // FILL IN

// FILL IN the last number: a free address, NOT .11-.14, .21-.24, .111-.114, .121-.124
IPAddress ip(192, 168, 50, 14);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus addresses
// ---------------------------------------------------------------------------
const int COIL_RAIL_CMD[3]    = {0, 1, 2};
const int DI_RAIL_STATUS[3]   = {0, 1, 2};
const int IREG_HEARTBEAT      = 0;
const int IREG_RECONNECT_COUNT = 1;

// ---------------------------------------------------------------------------
// Pins and relay polarity
// ---------------------------------------------------------------------------
const int PIN_RAIL_RELAY[3] = {25, 26, 27};
const char* RAIL_NAME[3]    = {"Elevator", "FireAlarm", "Door"};

const bool RELAY_ACTIVE_LOW = false;  // test each real board; boards seen so far: active-HIGH (reported)
const bool RAILS_ON_AT_BOOT = false;  // draft default, decision 8 open

const unsigned long WATCHDOG_MS       = 5000;
const unsigned long PRINT_INTERVAL_MS = 1000;

ModbusIP mb;

bool railOn[3] = {RAILS_ON_AT_BOOT, RAILS_ON_AT_BOOT, RAILS_ON_AT_BOOT};

// Set from the Modbus callbacks (run inside mb.task()), consumed in loop().
// railLastHub: last value the hub wrote (-1 = none yet / forgotten on rejoin).
volatile int8_t railLastHub[3]      = {-1, -1, -1};
volatile bool railWritePending[3]   = {false, false, false};
volatile bool railWriteValue[3]     = {false, false, false};

uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
unsigned long wifiDownSince = 0;
bool wifiWasDown = false;
bool failSafeApplied = false;
unsigned long lastPrint = 0;
unsigned long lastReconnectTry = 0;

void setRelay(int pin, bool energised) {
  bool level = RELAY_ACTIVE_LOW ? !energised : energised;
  digitalWrite(pin, level ? HIGH : LOW);
}

void hubWrite(int i, uint16_t val) {
  int8_t v = (val != 0) ? 1 : 0;
  if (v != railLastHub[i]) {
    railLastHub[i] = v;
    railWriteValue[i] = (v == 1);
    railWritePending[i] = true;
  }
}

// One callback per coil (same pattern as the real node's sketch).
uint16_t onElevCoilWrite(TRegister* reg, uint16_t val) { hubWrite(0, val); return val; }
uint16_t onFireCoilWrite(TRegister* reg, uint16_t val) { hubWrite(1, val); return val; }
uint16_t onDoorCoilWrite(TRegister* reg, uint16_t val) { hubWrite(2, val); return val; }

void applyOutputs() {
  for (int i = 0; i < 3; i++) {
    setRelay(PIN_RAIL_RELAY[i], railOn[i]);   // NO contact: coil on = powered
    mb.Ists(DI_RAIL_STATUS[i], railOn[i]);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== PowerRails Node PROTOTYPE (WiFi + Modbus) ===");

  for (int i = 0; i < 3; i++) {
    pinMode(PIN_RAIL_RELAY[i], OUTPUT);
    setRelay(PIN_RAIL_RELAY[i], railOn[i]);
  }

  if (ip[3] == 0) {
    Serial.println("STOP: set a free last IP number in this sketch (FILL IN), not a real node address.");
    while (true) { delay(1000); }
  }

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
    Serial.println("Serial commands still work; Modbus will not until WiFi joins.");
  }

  mb.server();
  for (int i = 0; i < 3; i++) {
    mb.addCoil(COIL_RAIL_CMD[i], false);
    mb.addIsts(DI_RAIL_STATUS[i], false);
  }
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  mb.onSetCoil(COIL_RAIL_CMD[0], onElevCoilWrite);
  mb.onSetCoil(COIL_RAIL_CMD[1], onFireCoilWrite);
  mb.onSetCoil(COIL_RAIL_CMD[2], onDoorCoilWrite);

  Serial.println("Ready. Rails per RAILS_ON_AT_BOOT. '1' '2' '3' toggle, 'a' cut all, 'r' restore all.");
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
      for (int i = 0; i < 3; i++) railOn[i] = false;
      failSafeApplied = true;
      Serial.println("WATCHDOG: comm lost, all rails forced OFF.");
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
    // Drop stale command state, forget last hub values so the first write counts.
    for (int i = 0; i < 3; i++) {
      mb.Coil(COIL_RAIL_CMD[i], false);
      railLastHub[i] = -1;
    }
  }

  // --- serial test commands (act like a local action) ---
  while (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '3') {
      int i = c - '1';
      railOn[i] = !railOn[i];
      Serial.print("Serial: rail ");
      Serial.print(RAIL_NAME[i]);
      Serial.println(railOn[i] ? " POWERED" : " CUT");
    } else if (c == 'a' || c == 'A') {
      for (int i = 0; i < 3; i++) railOn[i] = false;
      Serial.println("Serial: all rails CUT");
    } else if (c == 'r' || c == 'R') {
      for (int i = 0; i < 3; i++) railOn[i] = true;
      Serial.println("Serial: all rails POWERED");
    }
  }

  // --- fresh hub writes regain authority ---
  for (int i = 0; i < 3; i++) {
    if (railWritePending[i]) {
      railWritePending[i] = false;
      railOn[i] = railWriteValue[i];
      Serial.print("Hub wrote rail ");
      Serial.print(RAIL_NAME[i]);
      Serial.println(railOn[i] ? " -> POWERED" : " -> CUT");
    }
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
    for (int i = 0; i < 3; i++) {
      Serial.print(RAIL_NAME[i]);
      Serial.print("=");
      Serial.print(railOn[i] ? "powered" : "CUT");
      Serial.print("  ");
    }
    Serial.print("hb=");
    Serial.print(heartbeat);
    Serial.print(" rc=");
    Serial.print(reconnectCount);
    Serial.print("  wifi=");
    Serial.println(wifiUp ? "up" : "DOWN");
  }
}
