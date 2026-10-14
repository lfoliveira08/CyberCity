/*
  ============================================================================
  Project CyberUIW -- BSH-Elevator Node (PHASE B DRAFT: WiFi + Modbus)
  ============================================================================
  STATUS (2026-09-26): NOT THE CURRENT FIRMWARE. This is the WiFi+Modbus
  version, written 2026-09-14, that had been living only in the Claude
  project and not on local disk. Copied here so it is openable in the
  Arduino IDE. Em dashes were replaced with "--" on the way in (the project's
  Windows Arduino IDE rejects non-ASCII characters in comments); nothing else
  was changed.

  The node's CURRENT firmware is ../BSH_Elevator_Node/BSH_Elevator_Node.ino,
  the bench-confirmed Phase A sketch (2026-09-25). This draft lives in its own
  folder because the Arduino IDE compiles every .ino in a folder together as
  one sketch -- two sketches in one folder will not build.

  *** THE TWO VERSIONS ARE NOT A SUPERSET OF EACH OTHER. *** Before Phase B
  starts, port both directions:
    - This draft has, and the bench sketch lacks: travel time that scales
      with distance (expectedTravelMs = floorsToTravel * FLOOR_TRAVEL_MS).
      That is exactly the "Known simplification" flagged in
      ../SYSTEM_REPORT.md Section 1 -- this draft already solves it.
    - The bench sketch has, and this draft lacks: the 128x64 SSD1306 OLED
      (Adafruit_SSD1306, address 0x3C) that physically replaced the 16x2 LCD
      on 2026-09-25, and the three debug LEDs on GPIO 23/25/27 that mirror
      the L293D control lines. This draft still drives LiquidCrystal_I2C at
      0x27, which is NOT what is wired on the bench.
  Also unresolved here: MOTOR_SPEED (200) and FLOOR_TRAVEL_MS (6000) are
  still Wokwi placeholder values, never calibrated against the real
  DG01D-A130 gearmotor. See ../L293D_Motor_Driver/Wiring.md.

  Role: Simulates a 3-floor elevator inside the BSH building. Runs locally
  off button presses and/or remote "call floor" commands, reporting its
  state through its own dedicated mini-PLC.

  NOTE ON THE BUILD ORDER IN THE ORIGINAL HEADER: this file was written when
  the order was Fire Alarm -> Power Box -> Perimeter/Motion -> Elevator ->
  Door. The order changed on 2026-09-23 to Fire Alarm -> Elevator -> Door ->
  Power Box -> Perimeter/Motion. Firmware/WORKFLOW.md Section 1 is
  authoritative.

  Architecture: this ESP32 is a plain Modbus TCP SLAVE. OpenPLC-BSH-Elevator
  polls it directly at 192.168.1.11:502. No bridge, no other protocol on the
  backhaul side. (See BSH_Systems/01_BSH-Elevator.md and Section 5/6
  of Piranga_Workflow_v2.md.)

  REWRITE NOTE: this replaces the original standalone prototype
  (`Elevator_Motor.ino`, no Modbus, no networking). That prototype used
  delay() for the door-close wait and the full multi-second travel time,
  which would freeze mb.task() for seconds at a time and make this ESP32
  look like a dead Modbus slave to OpenPLC. This version keeps the same
  motor/button/LCD behavior but rebuilds the movement sequence as a
  non-blocking state machine driven by millis(), per the project's
  Universal Firmware Template. It also drops the prototype's 7-segment
  display, which isn't part of the documented BOM/pin map for this node
  (BSH_Systems/01_BSH-Elevator.md lists only the L293D driver, 3 buttons,
  and the I2C LCD) and its GPIOs aren't reserved anywhere in the docs.

  Modbus Register Map (per BSH_Systems/01_BSH-Elevator.md):
    Discrete Input  DI0   = ELEVATOR_MOVING    (slave->master)
    Discrete Input  DI1   = ELEVATOR_FAULT     (slave->master)
    Coil            Coil0 = CALL_FLOOR_1       (master->slave, momentary)
    Coil            Coil1 = CALL_FLOOR_2       (master->slave, momentary)
    Coil            Coil2 = CALL_FLOOR_3       (master->slave, momentary)
    Coil            Coil3 = ELEVATOR_DISABLE   (master->slave, level/lockout)
    Input Register  IR0   = CURRENT_FLOOR      (slave->master, 1-3)

  Hardware (see BSH_Sensors/01_L293D_Motor_Driver.md, 02_Push_Button.md,
  03_I2C_LCD_Display.md):
    GPIO 18 -> Motor ENABLE (PWM)
    GPIO 19 -> Motor DIRA
    GPIO 5  -> Motor DIRB
    GPIO 4  -> Floor button 1 (INPUT_PULLUP, active LOW)
    GPIO 16 -> Floor button 2 (INPUT_PULLUP, active LOW)
    GPIO 17 -> Floor button 3 (INPUT_PULLUP, active LOW)
    GPIO 21 -> LCD SDA
    GPIO 22 -> LCD SCL

  Libraries required (Arduino Library Manager):
    - ModbusIP_ESP8266     (works on ESP32 despite the name)
    - LiquidCrystal_I2C
  Board: "ESP32 Dev Module"

  IMPORTANT: fill in your real WiFi SSID/password below before flashing.
  Never commit real credentials to git or the shared Drive folder.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------------------------------------------------------------------
// WiFi configuration -- TODO: fill in before flashing
// ---------------------------------------------------------------------------
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ---------------------------------------------------------------------------
// Static IP -- must match Section 6 (Network / IP & PLC Map) of the workflow
// ---------------------------------------------------------------------------
IPAddress ip(192, 168, 1, 11);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus register addresses (per BSH_Systems/01_BSH-Elevator.md)
// ---------------------------------------------------------------------------
const int DI_ELEVATOR_MOVING = 0;
const int DI_ELEVATOR_FAULT  = 1;
const int COIL_CALL_FLOOR_1  = 0;
const int COIL_CALL_FLOOR_2  = 1;
const int COIL_CALL_FLOOR_3  = 2;
const int COIL_ELEVATOR_DISABLE = 3;
const int IREG_CURRENT_FLOOR = 0;

// ---------------------------------------------------------------------------
// GPIO pin map
// ---------------------------------------------------------------------------
const int PIN_MOTOR_ENABLE = 18;
const int PIN_MOTOR_DIRA   = 19;
const int PIN_MOTOR_DIRB   = 5;
const int PIN_BTN_FLOOR_1  = 4;
const int PIN_BTN_FLOOR_2  = 16;
const int PIN_BTN_FLOOR_3  = 17;
const int PIN_LCD_SDA      = 21;
const int PIN_LCD_SCL      = 22;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------------------------------------------------------------------
// Timing constants (carried over from the original Elevator_Motor.ino
// prototype's values, now driven non-blockingly)
// ---------------------------------------------------------------------------
const unsigned long DOOR_DELAY_MS       = 3000;   // doors "closing" before travel
const unsigned long FLOOR_TRAVEL_MS     = 6000;   // per floor of travel
const unsigned long ARRIVED_DISPLAY_MS  = 2000;   // how long "Arrived!" stays up
const int MOTOR_SPEED = 200;  // 0-255 PWM duty
const unsigned long LEDC_FREQ_HZ = 1000;
const int LEDC_RESOLUTION = 8;

// A move that takes meaningfully longer than its expected travel time is
// treated as a stall/fault rather than trusted blindly -- this is what makes
// ELEVATOR_FAULT a genuine, independently-detected condition rather than
// just a flag someone could set remotely (that's what ELEVATOR_DISABLE is
// for; see the system doc's "Notes" on keeping the two distinguishable).
const unsigned long FAULT_TIMEOUT_MARGIN_MS = 4000;

// ---------------------------------------------------------------------------
// Debounce (buttons)
// ---------------------------------------------------------------------------
const unsigned long DEBOUNCE_MS = 50;
struct ButtonState {
  bool rawLast = false;
  bool debounced = false;
  unsigned long lastChange = 0;
};
ButtonState btn1, btn2, btn3;

bool readDebounced(int pin, ButtonState &s, unsigned long now) {
  bool raw = (digitalRead(pin) == LOW);
  if (raw != s.rawLast) {
    s.lastChange = now;
    s.rawLast = raw;
  }
  if (now - s.lastChange > DEBOUNCE_MS) {
    s.debounced = raw;
  }
  return s.debounced;
}

// ---------------------------------------------------------------------------
// Elevator state machine
// ---------------------------------------------------------------------------
enum ElevatorState { ELEV_IDLE, ELEV_DOOR_CLOSING, ELEV_MOVING, ELEV_ARRIVED };
ElevatorState state = ELEV_IDLE;

int currentFloor = 1;
int targetFloor  = 1;
unsigned long stateEnteredAt = 0;
unsigned long expectedTravelMs = 0;
bool faulted  = false;

ModbusIP mb;

void lcdPrint(const String &line1, const String &line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void motorStop() {
  ledcWrite(PIN_MOTOR_ENABLE, 0);
  digitalWrite(PIN_MOTOR_DIRA, LOW);
  digitalWrite(PIN_MOTOR_DIRB, LOW);
}

void motorRunToward(int fromFloor, int toFloor) {
  if (toFloor > fromFloor) {
    digitalWrite(PIN_MOTOR_DIRA, HIGH);
    digitalWrite(PIN_MOTOR_DIRB, LOW);
  } else {
    digitalWrite(PIN_MOTOR_DIRA, LOW);
    digitalWrite(PIN_MOTOR_DIRB, HIGH);
  }
  ledcWrite(PIN_MOTOR_ENABLE, MOTOR_SPEED);
}

// Kick off a move if the elevator is free to accept one. A request while
// already moving, disabled, or faulted is simply dropped -- no queueing, per
// "Stability Over Complexity."
void requestFloor(int floor) {
  if (faulted) return;
  if (mb.Coil(COIL_ELEVATOR_DISABLE)) return;
  if (state != ELEV_IDLE) return;
  if (floor == currentFloor) {
    lcdPrint("Already on", "Floor " + String(currentFloor));
    return;
  }

  targetFloor = floor;
  state = ELEV_DOOR_CLOSING;
  stateEnteredAt = millis();
  lcdPrint("Floor " + String(targetFloor) + " requested", "Doors closing...");
}

void setup() {
  Serial.begin(115200);

  Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcdPrint("Elevator Ready", "Floor 1");

  pinMode(PIN_MOTOR_DIRA, OUTPUT);
  pinMode(PIN_MOTOR_DIRB, OUTPUT);
  digitalWrite(PIN_MOTOR_DIRA, LOW);
  digitalWrite(PIN_MOTOR_DIRB, LOW);
  ledcAttach(PIN_MOTOR_ENABLE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  ledcWrite(PIN_MOTOR_ENABLE, 0);

  pinMode(PIN_BTN_FLOOR_1, INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR_2, INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR_3, INPUT_PULLUP);

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
    Serial.println("Local buttons still work; remote calls/monitoring will not.");
  }

  mb.server();
  mb.addIsts(DI_ELEVATOR_MOVING, false);
  mb.addIsts(DI_ELEVATOR_FAULT, false);
  mb.addCoil(COIL_CALL_FLOOR_1, false);
  mb.addCoil(COIL_CALL_FLOOR_2, false);
  mb.addCoil(COIL_CALL_FLOOR_3, false);
  mb.addCoil(COIL_ELEVATOR_DISABLE, false);
  mb.addIreg(IREG_CURRENT_FLOOR, currentFloor);
}

void loop() {
  mb.task();  // ALWAYS call first - services Modbus TCP requests

  unsigned long now = millis();

  // -------------------------------------------------------------------
  // Local button inputs
  // -------------------------------------------------------------------
  bool p1 = readDebounced(PIN_BTN_FLOOR_1, btn1, now);
  bool p2 = readDebounced(PIN_BTN_FLOOR_2, btn2, now);
  bool p3 = readDebounced(PIN_BTN_FLOOR_3, btn3, now);
  static bool p1Last = false, p2Last = false, p3Last = false;
  if (p1 && !p1Last) requestFloor(1);
  if (p2 && !p2Last) requestFloor(2);
  if (p3 && !p3Last) requestFloor(3);
  p1Last = p1; p2Last = p2; p3Last = p3;

  // -------------------------------------------------------------------
  // Remote "call floor" coils -- momentary, like a physical button: honor
  // a rising edge, then clear the coil ourselves so the SCADA/attacker
  // side doesn't have to write it back to 0 for the model to make sense.
  // -------------------------------------------------------------------
  static bool c1Last = false, c2Last = false, c3Last = false;
  bool c1 = mb.Coil(COIL_CALL_FLOOR_1);
  bool c2 = mb.Coil(COIL_CALL_FLOOR_2);
  bool c3 = mb.Coil(COIL_CALL_FLOOR_3);
  if (c1 && !c1Last) { requestFloor(1); mb.Coil(COIL_CALL_FLOOR_1, false); }
  if (c2 && !c2Last) { requestFloor(2); mb.Coil(COIL_CALL_FLOOR_2, false); }
  if (c3 && !c3Last) { requestFloor(3); mb.Coil(COIL_CALL_FLOOR_3, false); }
  c1Last = c1; c2Last = c2; c3Last = c3;

  // -------------------------------------------------------------------
  // State machine
  // -------------------------------------------------------------------
  switch (state) {
    case ELEV_IDLE:
      // Nothing to do; requestFloor() above drives transitions out of IDLE.
      break;

    case ELEV_DOOR_CLOSING:
      if (now - stateEnteredAt >= DOOR_DELAY_MS) {
        int floorsToTravel = abs(targetFloor - currentFloor);
        expectedTravelMs = (unsigned long)floorsToTravel * FLOOR_TRAVEL_MS;
        motorRunToward(currentFloor, targetFloor);
        state = ELEV_MOVING;
        stateEnteredAt = now;
        lcdPrint(targetFloor > currentFloor ? "Going up..." : "Going down...",
                 "To Floor " + String(targetFloor));
      }
      break;

    case ELEV_MOVING: {
      unsigned long elapsed = now - stateEnteredAt;
      if (elapsed >= expectedTravelMs) {
        motorStop();
        currentFloor = targetFloor;
        mb.Ireg(IREG_CURRENT_FLOOR, currentFloor);
        state = ELEV_ARRIVED;
        stateEnteredAt = now;
        lcdPrint("Arrived!", "Floor " + String(currentFloor));
      }
      // Stall/fault detection for this state lives in the dedicated check
      // below (runs every loop, independent of this normal-completion
      // path) so it still catches a jam even if this branch never fires.
      break;
    }

    case ELEV_ARRIVED:
      if (now - stateEnteredAt >= ARRIVED_DISPLAY_MS) {
        state = ELEV_IDLE;
        lcdPrint("Elevator Ready", "Floor " + String(currentFloor));
      }
      break;
  }

  // -------------------------------------------------------------------
  // Fault detection -- independent stall watchdog on the MOVING state.
  // If we are still in ELEV_MOVING well past when we should have
  // finished, something is wrong with the motor/mechanism (or the model
  // physically jammed) - stop driving it and latch a fault rather than
  // pretending we know where the cab ended up.
  // -------------------------------------------------------------------
  if (state == ELEV_MOVING && !faulted) {
    if (now - stateEnteredAt > expectedTravelMs + FAULT_TIMEOUT_MARGIN_MS) {
      motorStop();
      faulted = true;
      lcdPrint("FAULT", "Check elevator");
      Serial.println("ELEVATOR_FAULT latched: movement exceeded expected time.");
    }
  }
  // Once latched, ELEVATOR_FAULT stays set until the board is reset/power-
  // cycled (representing "needs a technician") - it deliberately does not
  // self-clear, so it can't be mistaken for the remotely-toggleable
  // ELEVATOR_DISABLE coil.
  mb.Ists(DI_ELEVATOR_FAULT, faulted);

  bool moving = (state == ELEV_MOVING);
  mb.Ists(DI_ELEVATOR_MOVING, moving);
}
