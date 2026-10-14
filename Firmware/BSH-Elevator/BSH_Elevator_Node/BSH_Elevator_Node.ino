/*
 * BSH-Elevator Node -- Phase B: WiFi + Modbus TCP
 * ============================================================================
 * STATUS (2026-10-07): Phase A confirmed on the real bench (2026-09-25) -- see
 * Firmware/BSH-Elevator/SYSTEM_REPORT.md. Phase B (this sketch) bench-confirmed
 * 2026-10-07 at 192.168.50.11: remote floor calls, momentary coils, DISABLE
 * lockout, local buttons, heartbeat/reconnect. Results in
 * Firmware/BSH-Elevator/ESP32_Core_Modbus/Build_Log.md. Known limits: DI1 fault
 * cannot latch, DISABLE does not stop a move in progress, no fail-off on WiFi loss.
 *
 * Role: simulates a 3-floor elevator in the BSH building. Runs off local
 * button presses and/or remote "call floor" Modbus coil writes, reporting
 * state to its own dedicated OpenPLC-BSH-Elevator instance.
 *
 * Architecture: plain Modbus TCP SLAVE. OpenPLC-BSH-Elevator polls this
 * ESP32 directly at 192.168.50.11:502. No bridge, no other backhaul protocol.
 * (See BSH_Systems/01_BSH-Elevator.md and Piranga_Workflow_v2.md Section 6.)
 *
 * Modbus Register Map (per BSH_Systems/01_BSH-Elevator.md):
 *   Discrete Input  DI0   = ELEVATOR_MOVING    (slave->master)
 *   Discrete Input  DI1   = ELEVATOR_FAULT     (slave->master)
 *   Coil            Coil0 = CALL_FLOOR_1       (master->slave, momentary)
 *   Coil            Coil1 = CALL_FLOOR_2       (master->slave, momentary)
 *   Coil            Coil2 = CALL_FLOOR_3       (master->slave, momentary)
 *   Coil            Coil3 = ELEVATOR_DISABLE   (master->slave, level/lockout)
 *   Input Register  IR0   = CURRENT_FLOOR      (slave->master, 1-3)
 *   Input Register  IR1   = HEARTBEAT          (+1 per second, wraps at 65535)
 *   Input Register  IR2   = RECONNECT_COUNT    (WiFi rejoins since boot)
 *   (HEARTBEAT/RECONNECT_COUNT added Oct 6, 2026: SCADA reads OpenPLC, which keeps
 *    answering with stale values if this ESP32 drops, so only a counter this board
 *    increments itself proves it is alive. Frozen = lost; resumes with RC+1 = WiFi
 *    drop; both back at 0 = reboot.)
 *
 * Hardware (see each component's Wiring.md under Firmware/BSH-Elevator):
 *   Floor 1 button -> GPIO 4   (INPUT_PULLUP)
 *   Floor 2 button -> GPIO 16  (INPUT_PULLUP)
 *   Floor 3 button -> GPIO 17  (INPUT_PULLUP)
 *   OLED SDA       -> GPIO 21
 *   OLED SCL       -> GPIO 22
 *   L293D ENABLE   -> GPIO 18  (PWM, motor speed)
 *   L293D DIRA     -> GPIO 19
 *   L293D DIRB     -> GPIO 5
 *   Debug LED (opt)-> GPIO 23 (mirrors ENABLE), GPIO 25 (mirrors DIRA), GPIO 27 (mirrors DIRB)
 *
 * Libraries needed (Library Manager):
 *   - ModbusIP_ESP8266      (works on ESP32 despite the name)
 *   - Adafruit SSD1306 + Adafruit GFX Library
 *
 * Board: "ESP32 Dev Module"
 * ============================================================================
 */

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------------------------------------------------------------------
// WiFi configuration
// ---------------------------------------------------------------------------
const char* ssid     = "SCT5";
const char* password = "S3n!0rC@p";

// ---------------------------------------------------------------------------
// Static IP -- must match Piranga_Workflow_v2.md Section 6 (Network/IP map)
// ---------------------------------------------------------------------------
IPAddress ip(192, 168, 50, 11);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus register addresses (per BSH_Systems/01_BSH-Elevator.md)
// ---------------------------------------------------------------------------
const int DI_ELEVATOR_MOVING    = 0;
const int DI_ELEVATOR_FAULT     = 1;
const int COIL_CALL_FLOOR_1     = 0;
const int COIL_CALL_FLOOR_2     = 1;
const int COIL_CALL_FLOOR_3     = 2;
const int COIL_ELEVATOR_DISABLE = 3;
const int IREG_CURRENT_FLOOR    = 0;
const int IREG_HEARTBEAT        = 1;
const int IREG_RECONNECT_COUNT  = 2;

// ---------- Pins ----------
const int BTN_FLOOR[3]   = {4, 16, 17};
const int PIN_ENABLE      = 18;
const int PIN_DIRA        = 19;
const int PIN_DIRB        = 5;
const int PIN_DBG_ENABLE  = 23; // mirrors PIN_ENABLE
const int PIN_DBG_DIRA    = 25; // mirrors PIN_DIRA
const int PIN_DBG_DIRB    = 27; // mirrors PIN_DIRB

// ---------- Timing / tuning ----------
// FLOOR_TRAVEL_MS halved on 2026-09-30 (was 6000) -- per-floor travel time
// for this demo; still not measured/calibrated against the real cab
// mechanism (see L293D_Motor_Driver/Wiring.md's target-time method).
const unsigned long DOOR_DELAY_MS          = 3000;
const unsigned long FLOOR_TRAVEL_MS        = 1680;  // time to travel ONE floor -- multi-floor
                                                      // moves scale by distance, see expectedTravelMs below
const unsigned long ARRIVED_DISPLAY_MS     = 2000;
const int           MOTOR_SPEED            = 200;    // 0-255 PWM duty
const int           LEDC_FREQ_HZ           = 1000;
const int           LEDC_RESOLUTION        = 8;       // 8-bit -> 0-255 duty range
const unsigned long FAULT_TIMEOUT_MARGIN_MS = 4000;
const unsigned long DEBOUNCE_MS            = 50;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Debounce state ----------
struct ButtonState {
  bool lastReading;
  bool stable;
  unsigned long lastChangeMs;
};
ButtonState btnState[3];

// ---------- Elevator state machine ----------
enum ElevatorState { ELEV_IDLE, ELEV_DOOR_CLOSING, ELEV_MOVING, ELEV_ARRIVED };
ElevatorState state = ELEV_IDLE;

int currentFloor = 1;       // 1-3, physical starting position -- set the car at floor 1 before power-on
int targetFloor = -1;
unsigned long stateEnteredAt = 0;
unsigned long expectedTravelMs = 0;
bool faulted = false;

ModbusIP mb;
uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
unsigned long lastReconnectTry = 0;
bool wifiWasUp = false;

// ---------- Helpers ----------
bool readDebounced(int idx) {
  bool raw = (digitalRead(BTN_FLOOR[idx]) == LOW); // pressed = LOW (INPUT_PULLUP)
  unsigned long now = millis();
  if (raw != btnState[idx].lastReading) {
    btnState[idx].lastChangeMs = now;
    btnState[idx].lastReading = raw;
  }
  if ((now - btnState[idx].lastChangeMs) > DEBOUNCE_MS) {
    if (btnState[idx].stable != raw) {
      btnState[idx].stable = raw;
      return raw; // return true only on the debounced press edge
    }
  }
  return false;
}

void lcdPrint(const String &line1, const String &line2) {
  // Text size 1 = 6px/char, so up to ~21 characters fit on a 128px-wide screen
  // without wrapping.
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 16);
  display.println(line2);
  display.display();
}

void motorStop() {
  ledcWrite(PIN_ENABLE, 0);
  digitalWrite(PIN_DIRA, LOW);
  digitalWrite(PIN_DIRB, LOW);
  // mirror to debug LEDs
  ledcWrite(PIN_DBG_ENABLE, 0);
  digitalWrite(PIN_DBG_DIRA, LOW);
  digitalWrite(PIN_DBG_DIRB, LOW);
}

void motorRunToward(int target) {
  bool up = (target > currentFloor);
  digitalWrite(PIN_DIRA, up ? HIGH : LOW);
  digitalWrite(PIN_DIRB, up ? LOW : HIGH);
  ledcWrite(PIN_ENABLE, MOTOR_SPEED);
  // mirror to debug LEDs
  digitalWrite(PIN_DBG_DIRA, up ? HIGH : LOW);
  digitalWrite(PIN_DBG_DIRB, up ? LOW : HIGH);
  ledcWrite(PIN_DBG_ENABLE, MOTOR_SPEED);
}

// Kick off a move if the elevator is free to accept one. A request while
// already moving, disabled, or faulted is simply dropped -- no queueing, per
// "Stability Over Complexity."
void requestFloor(int floorNum) {
  if (faulted) return;
  if (mb.Coil(COIL_ELEVATOR_DISABLE)) return;
  if (state != ELEV_IDLE) return;
  if (floorNum == currentFloor) {
    lcdPrint("Floor " + String(floorNum), "Already here");
    return;
  }

  targetFloor = floorNum;
  state = ELEV_DOOR_CLOSING;
  stateEnteredAt = millis();
  lcdPrint("Floor " + String(floorNum), "Door closing..");
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(BTN_FLOOR[i], INPUT_PULLUP);
    btnState[i] = {false, false, 0};
  }

  pinMode(PIN_DIRA, OUTPUT);
  pinMode(PIN_DIRB, OUTPUT);
  pinMode(PIN_DBG_DIRA, OUTPUT);
  pinMode(PIN_DBG_DIRB, OUTPUT);

  ledcAttach(PIN_ENABLE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  ledcAttach(PIN_DBG_ENABLE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  motorStop();

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found at 0x3C -- check wiring/address (try 0x3D).");
    while (true) { delay(1000); } // halt so it's obvious the screen didn't init
  }
  lcdPrint("Elevator Ready", "Floor " + String(currentFloor));

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
    lcdPrint("WiFi connected", WiFi.localIP().toString());
    delay(1000);
  } else {
    Serial.println("\nWiFi FAILED to connect - check SSID/password and AP range.");
    Serial.println("Local buttons still work; remote calls/monitoring will not.");
    lcdPrint("WiFi FAILED", "Local mode only");
    delay(1500);
  }

  mb.server();
  mb.addIsts(DI_ELEVATOR_MOVING, false);
  mb.addIsts(DI_ELEVATOR_FAULT, false);
  mb.addCoil(COIL_CALL_FLOOR_1, false);
  mb.addCoil(COIL_CALL_FLOOR_2, false);
  mb.addCoil(COIL_CALL_FLOOR_3, false);
  mb.addCoil(COIL_ELEVATOR_DISABLE, false);
  mb.addIreg(IREG_CURRENT_FLOOR, currentFloor);
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  wifiWasUp = (WiFi.status() == WL_CONNECTED);

  lcdPrint("Elevator Ready", "Floor " + String(currentFloor));

  Serial.println("BSH-Elevator node ready (Phase B: WiFi + Modbus).");
}

void loop() {
  mb.task();  // ALWAYS call first -- services Modbus TCP requests

  // --- WiFi link tracking + heartbeat (non-blocking) ---
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
      Serial.println("WiFi link lost.");
    }
    wifiWasUp = wifiUp;
    if (t - lastHeartbeatTick >= 1000) {
      lastHeartbeatTick = t;
      heartbeat++;
      mb.Ireg(IREG_HEARTBEAT, heartbeat);
    }
  }

  // --- local button polling ---
  for (int i = 0; i < 3; i++) {
    if (readDebounced(i)) {
      requestFloor(i + 1);
    }
  }

  // --- remote "call floor" coils -- momentary, like a physical button: honor
  // a rising edge, then clear the coil ourselves so the SCADA/attacker side
  // doesn't have to write it back to 0 for the model to make sense. ---
  static bool c1Last = false, c2Last = false, c3Last = false;
  bool c1 = mb.Coil(COIL_CALL_FLOOR_1);
  bool c2 = mb.Coil(COIL_CALL_FLOOR_2);
  bool c3 = mb.Coil(COIL_CALL_FLOOR_3);
  if (c1 && !c1Last) { requestFloor(1); mb.Coil(COIL_CALL_FLOOR_1, false); }
  if (c2 && !c2Last) { requestFloor(2); mb.Coil(COIL_CALL_FLOOR_2, false); }
  if (c3 && !c3Last) { requestFloor(3); mb.Coil(COIL_CALL_FLOOR_3, false); }
  c1Last = c1; c2Last = c2; c3Last = c3;

  unsigned long now = millis();

  switch (state) {
    case ELEV_IDLE:
      // nothing to do, waiting for a request
      break;

    case ELEV_DOOR_CLOSING:
      if (now - stateEnteredAt >= DOOR_DELAY_MS) {
        int floorsToTravel = abs(targetFloor - currentFloor);
        expectedTravelMs = (unsigned long)floorsToTravel * FLOOR_TRAVEL_MS;
        state = ELEV_MOVING;
        stateEnteredAt = now;
        motorRunToward(targetFloor);
        lcdPrint("Moving to " + String(targetFloor), (targetFloor > currentFloor) ? "Going UP" : "Going DOWN");
      }
      break;

    case ELEV_MOVING: {
      unsigned long travelSoFar = now - stateEnteredAt;
      bool normalTimeout = travelSoFar >= expectedTravelMs;

      if (normalTimeout) {
        motorStop();
        currentFloor = targetFloor;
        mb.Ireg(IREG_CURRENT_FLOOR, currentFloor);
        state = ELEV_ARRIVED;
        stateEnteredAt = now;
        lcdPrint("Arrived Floor " + String(currentFloor), "Door opening..");
      } else if (!faulted && travelSoFar > (expectedTravelMs + FAULT_TIMEOUT_MARGIN_MS)) {
        // safety net -- should not normally trigger since normalTimeout fires first
        motorStop();
        faulted = true;
        lcdPrint("FAULT", "Motor timeout");
        Serial.println("ELEVATOR_FAULT latched: movement exceeded expected time.");
      }
      break;
    }

    case ELEV_ARRIVED:
      if (now - stateEnteredAt >= ARRIVED_DISPLAY_MS) {
        state = ELEV_IDLE;
        lcdPrint("Floor " + String(currentFloor), "Ready");
      }
      break;
  }

  // Once latched, ELEVATOR_FAULT stays set until the board is reset/power-
  // cycled (represents "needs a technician") -- it deliberately does not
  // self-clear, so it can't be mistaken for the remotely-toggleable
  // ELEVATOR_DISABLE coil.
  mb.Ists(DI_ELEVATOR_FAULT, faulted);
  mb.Ists(DI_ELEVATOR_MOVING, (state == ELEV_MOVING));
}
