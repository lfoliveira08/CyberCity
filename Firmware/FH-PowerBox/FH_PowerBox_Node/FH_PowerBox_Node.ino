/*
  ============================================================================
  FH-PowerBox Node -- power rails: WiFi + Modbus TCP slave
  ============================================================================
  STATUS (2026-10-28): REPLACES the lights-and-fan sketch (archived as
  Archive/FH_PowerBox_Node_lights-and-fan.ino). Mirrors BSH-PowerBox's
  redesign (BSH-PowerBox/BSH_PowerBox_Node.ino) -- same logic, pin numbers
  and decisions, only the IP differs. The FH-PowerBox no longer drives
  lights or a fan; it switches power to the other three FH systems:
  FH-HVAC, FH-FireAlarm, FH-Door. Not yet bench-confirmed on FH hardware --
  mirrors BSH-PowerBox's bench results, reported without values.

  Architecture: plain Modbus TCP SLAVE at 192.168.50.24:502 (the old FH-
  PowerBox address, reused on purpose, mirroring BSH-PowerBox's .14). The
  OpenPLC-FH-PowerBox container (192.168.50.124) polls it directly. No bridge.

  Modbus register map (Prototype_PowerRails/Register_Map_DRAFT.md pattern,
  mirrored for FH -- FH_Systems/04_FH-PowerBox.md to be updated, adoption
  Step 3 for FH):
    Coil0  HVAC_RAIL_CMD    (master -> slave) 1 = FH-HVAC node powered
    Coil1  FIRE_RAIL_CMD    (master -> slave) 1 = FH-FireAlarm node powered
    Coil2  DOOR_RAIL_CMD    (master -> slave) 1 = FH-Door node powered
    DI0    HVAC_RAIL_STATUS  (slave -> master) rail as this node drives it
    DI1    FIRE_RAIL_STATUS
    DI2    DOOR_RAIL_STATUS
    IR0    HEARTBEAT         (+1 per second, wraps at 65535)
    IR1    RECONNECT_COUNT   (WiFi rejoins since boot; 0 after a reboot)
    (Status inputs report the commanded relay state, not a measured
     voltage. HEARTBEAT and RECONNECT_COUNT stay the last two input
     registers.)

  Hardware (same GPIO map as BSH-PowerBox; verify against this node's own
  wiring before trusting it -- not yet documented in Schematic/Connections.md):
    GPIO 25 -> Rail 1 relay IN (FH-HVAC)
    GPIO 26 -> Rail 2 relay IN (FH-FireAlarm)
    GPIO 27 -> Rail 3 relay IN (FH-Door)
  Relay contact: NO (normally open), mirrors BSH-PowerBox's Oct 27, 2026
  decision. Relay coil on = rail powered, coil off = no power. Each relay's
  COM/NO wiring is not yet confirmed on this node's own hardware -- see
  BSH-PowerBox's Schematic/Connections.md for the pattern to copy, then
  write this node's own as-built wiring (do not assume it is identical
  without checking).

  BOOT (mirrors BSH-PowerBox's Oct 28, 2026 decision, option A): all rails
  are OFF after power-up until the hub commands them on.

  HUB-WRITE RULE (same as every node, Oct 21, 2026 for PowerBox): a Modbus
  coil write counts as a command only when its value DIFFERS from the last
  value the hub wrote, because OpenPLC rewrites coils about every 100 ms.
  After a WiFi rejoin the last hub value is forgotten, so the first write
  after reconnect counts.

  FAIL-SAFE (mirrors BSH-PowerBox): if the WiFi LINK stays down longer than
  WATCHDOG_MS, all rails go OFF. This detects WiFi link loss only, not a
  stopped PLC container or a silent master.

  LOCAL TEST: Serial Monitor at 115200, '1' '2' '3' toggle a rail, 'a' cut
  all, 'r' restore all. No Serial connection in the finished build.

  Libraries: ModbusIP_ESP8266 (works on ESP32). Board: ESP32 Dev Module.
  ASCII only in this file (no em dashes) -- Windows Arduino IDE rule.
  CREDENTIALS: the WiFi name and password below are placeholders on purpose.
  Put the real values in your own copy before flashing; never commit them.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

// ---------------------------------------------------------------------------
// WiFi + static IP (must match Piranga_Workflow_v2.md Section 6)
// ---------------------------------------------------------------------------
const char* ssid     = "SCT5";      // placeholder, set locally
const char* password = "S3n!0rC@p";  // placeholder, set locally

IPAddress ip(192, 168, 50, 24);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus addresses
// ---------------------------------------------------------------------------
const int COIL_RAIL_CMD[3]     = {0, 1, 2};
const int DI_RAIL_STATUS[3]    = {0, 1, 2};
const int IREG_HEARTBEAT       = 0;
const int IREG_RECONNECT_COUNT = 1;

// ---------------------------------------------------------------------------
// Pins and relay polarity
// ---------------------------------------------------------------------------
const int PIN_RAIL_RELAY[3] = {25, 26, 27};
const char* RAIL_NAME[3]    = {"HVAC", "FireAlarm", "Door"};

const bool RELAY_ACTIVE_LOW = false;  // mirrors BSH-PowerBox's bench boards; TEST this node's own boards independently
const bool RAILS_ON_AT_BOOT = false;  // option A: off until the hub commands them on

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

// One callback per coil (same pattern as BSH-PowerBox's sketch).
uint16_t onHvacCoilWrite(TRegister* reg, uint16_t val) { hubWrite(0, val); return val; }
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
  Serial.println("=== FH-PowerBox Node (power rails: WiFi + Modbus) ===");

  for (int i = 0; i < 3; i++) {
    pinMode(PIN_RAIL_RELAY[i], OUTPUT);
    setRelay(PIN_RAIL_RELAY[i], railOn[i]);
  }

  if (strcmp(ssid, "FILL_IN_SSID") == 0) {
    Serial.println("STOP: this copy still has the placeholder WiFi name and password.");
    Serial.println("Set the real values in your own copy before flashing.");
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
  mb.onSetCoil(COIL_RAIL_CMD[0], onHvacCoilWrite);
  mb.onSetCoil(COIL_RAIL_CMD[1], onFireCoilWrite);
  mb.onSetCoil(COIL_RAIL_CMD[2], onDoorCoilWrite);

  Serial.println("Ready. All rails boot OFF until the hub commands them on.");
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
      Serial.println("WATCHDOG: WiFi link lost, all rails forced OFF.");
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
