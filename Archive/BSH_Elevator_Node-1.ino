/*
 * BSH-Elevator -- Real Bench Combined Sketch (Phase A)
 * ESP32 + 3 floor buttons + I2C SSD1306 OLED + L293D motor driver + optional debug LEDs
 *
 * Pinout (matches each component's Wiring.md under Firmware/BSH-Elevator and Wiring_Diagram.svg):
 *   Floor 1 button -> GPIO 4   (INPUT_PULLUP)
 *   Floor 2 button -> GPIO 16  (INPUT_PULLUP)
 *   Floor 3 button -> GPIO 17  (INPUT_PULLUP)
 *   OLED SDA       -> GPIO 21
 *   OLED SCL       -> GPIO 22
 *   L293D ENABLE   -> GPIO 13  (PWM, motor speed)
 *   L293D DIRA     -> GPIO 14
 *   L293D DIRB     -> GPIO 26
 *   Debug LED (opt)-> GPIO 23 (mirrors ENABLE), GPIO 25 (mirrors DIRA), GPIO 27 (mirrors DIRB)
 *
 * Libraries needed (Library Manager): "Adafruit SSD1306" + "Adafruit GFX Library"
 * Assumes a 128x64 SSD1306 at I2C address 0x3C -- the most common size/address combo.
 * If yours is 128x32, change SCREEN_HEIGHT below to 32. If the screen stays blank,
 * try address 0x3D instead of 0x3C.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Pins ----------
const int BTN_FLOOR[3]   = {4, 16, 17};
const int PIN_ENABLE      = 13;
const int PIN_DIRA        = 14;
const int PIN_DIRB        = 26;
const int PIN_DBG_ENABLE  = 23; // mirrors PIN_ENABLE
const int PIN_DBG_DIRA    = 25; // mirrors PIN_DIRA
const int PIN_DBG_DIRB    = 27; // mirrors PIN_DIRB

// ---------- Timing / tuning (starting values -- same as the working Wokwi sim) ----------
const unsigned long DOOR_DELAY_MS          = 3000;
const unsigned long FLOOR_TRAVEL_MS        = 6000;  // time to travel one floor -- tune this per the
                                                      // target-time calibration procedure in
                                                      // L293D_Motor_Driver/Wiring.md once the real
                                                      // motor/gearbox load is on the bench
const unsigned long ARRIVED_DISPLAY_MS     = 2000;
const int           MOTOR_SPEED            = 200;    // 0-255 PWM duty -- tune alongside FLOOR_TRAVEL_MS
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
bool faulted = false;

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
  // without wrapping. Our longest messages (e.g. "Moving to 3") fit comfortably,
  // so both lines stay put instead of wrapping into each other.
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

void requestFloor(int floorNum) {
  if (faulted) return;
  if (floorNum == currentFloor && state == ELEV_IDLE) {
    lcdPrint("Floor " + String(floorNum), "Already here");
    return;
  }
  if (state == ELEV_IDLE) {
    targetFloor = floorNum;
    state = ELEV_DOOR_CLOSING;
    stateEnteredAt = millis();
    lcdPrint("Floor " + String(floorNum), "Door closing..");
  }
  // ignore new requests while already moving/door-closing/arrived -- matches Wokwi sim behavior
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

  Serial.println("BSH-Elevator bench node ready.");
}

void loop() {
  // --- button polling ---
  for (int i = 0; i < 3; i++) {
    if (readDebounced(i)) {
      requestFloor(i + 1);
    }
  }

  unsigned long now = millis();

  switch (state) {
    case ELEV_IDLE:
      // nothing to do, waiting for a request
      break;

    case ELEV_DOOR_CLOSING:
      if (now - stateEnteredAt >= DOOR_DELAY_MS) {
        state = ELEV_MOVING;
        stateEnteredAt = now;
        motorRunToward(targetFloor);
        lcdPrint("Moving to " + String(targetFloor), (targetFloor > currentFloor) ? "Going UP" : "Going DOWN");
      }
      break;

    case ELEV_MOVING: {
      unsigned long travelSoFar = now - stateEnteredAt;
      bool normalTimeout = travelSoFar >= FLOOR_TRAVEL_MS;

      if (normalTimeout) {
        motorStop();
        currentFloor = targetFloor;
        state = ELEV_ARRIVED;
        stateEnteredAt = now;
        lcdPrint("Arrived Floor " + String(currentFloor), "Door opening..");
      } else if (!faulted && travelSoFar > (FLOOR_TRAVEL_MS + FAULT_TIMEOUT_MARGIN_MS)) {
        // safety net -- should not normally trigger since normalTimeout fires first
        motorStop();
        faulted = true;
        lcdPrint("FAULT", "Motor timeout");
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
}
