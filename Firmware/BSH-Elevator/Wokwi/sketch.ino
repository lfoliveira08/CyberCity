/*
  ============================================================================
  BSH-Elevator -- Wokwi Simulation (Phase A0)
  ============================================================================
  Purpose: learn how the Elevator system works -- the non-blocking state
  machine, the debounced floor buttons, the I2C LCD, and the L293D motor
  driver's ENABLE/DIRA/DIRB control lines -- in the Wokwi simulator, before
  (or alongside) the real bench build. Rule: Firmware/WORKFLOW.md Section 7.

  This mirrors the state-machine core of the full node firmware
  (Firmware/BSH-Elevator/BSH_Elevator_Node/BSH_Elevator_Node.ino, bench
  confirmed 2026-09-25 -- see Firmware/BSH-Elevator/SYSTEM_REPORT.md):
  same pin map, same debounce, same DOOR_CLOSING -> MOVING -> ARRIVED
  timing, same non-blocking millis() structure, same stall-timeout
  ELEVATOR_FAULT latch. WiFi and Modbus (ModbusIP_ESP8266) are Phase B/C
  and are NOT part of this sketch -- this is the no-WiFi/no-Modbus
  standalone integration test, same pattern as
  BSH-FireAlarm/BSH_FireAlarm_Node/BSH_FireAlarm_Node.ino.

  Two things this sketch can't get from the real node without Modbus:
    - ELEVATOR_DISABLE is a remote-lockout coil (Phase B). There's no local
      equivalent, so it isn't simulated here -- it starts existing once
      Modbus does.
    - A real motor stall (jammed cab, disconnected motor) can't happen to
      a simulated LED. So this sketch adds ONE simulation-only test hook,
      gated behind WOKWI_SIM: type 's' + Enter into the Serial Monitor
      while the cab is "moving" to simulate a stalled motor and watch
      ELEVATOR_FAULT latch. Real hardware doesn't need this -- a genuine
      stall triggers the same watchdog on its own.

  Parts in diagram.json (same GPIOs as BSH_Systems/01_BSH-Elevator.md):
    GPIO 4/16/17  floor buttons 1/2/3  (wokwi-pushbutton, INPUT_PULLUP, active LOW)
    GPIO 21/22    LCD SDA/SCL          (wokwi-lcd1602, "pins": "i2c")
    GPIO 19       Motor DIRA           (STAND-IN: green LED -- "going up")
    GPIO 5        Motor DIRB           (STAND-IN: red LED -- "going down")
    GPIO 18       Motor ENABLE (PWM)   (STAND-IN: yellow LED -- brightness = speed)

  No WiFi, no Modbus. Only extra library: LiquidCrystal_I2C (see libraries.txt).
  ============================================================================
*/

#define WOKWI_SIM 1   // 1 = running in Wokwi (adds the 's' stall test-hook), 0 = real hardware

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------------------------------------------------------------------
// GPIO pin map -- identical to BSH_Systems/01_BSH-Elevator.md
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
// Timing constants -- same values as the bench sketch / the original
// Elevator_Motor.ino prototype's numbers, now driven non-blockingly
// ---------------------------------------------------------------------------
const unsigned long DOOR_DELAY_MS      = 3000;  // doors "closing" before travel
const unsigned long FLOOR_TRAVEL_MS    = 6000;  // per floor of travel
const unsigned long ARRIVED_DISPLAY_MS = 2000;  // how long "Arrived!" stays up
const int MOTOR_SPEED = 200;                    // 0-255 PWM duty
const unsigned long LEDC_FREQ_HZ   = 1000;
const int LEDC_RESOLUTION = 8;

// A move that takes meaningfully longer than its expected travel time is
// treated as a stall/fault rather than trusted blindly.
const unsigned long FAULT_TIMEOUT_MARGIN_MS = 4000;

// ---------------------------------------------------------------------------
// Debounce (buttons) -- same struct/approach as the real firmware
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
// Elevator state machine -- identical states/transitions to the real firmware
// ---------------------------------------------------------------------------
enum ElevatorState { ELEV_IDLE, ELEV_DOOR_CLOSING, ELEV_MOVING, ELEV_ARRIVED };
ElevatorState state = ELEV_IDLE;

int currentFloor = 1;
int targetFloor  = 1;
unsigned long stateEnteredAt = 0;
unsigned long expectedTravelMs = 0;
bool faulted = false;

#if WOKWI_SIM
bool simulateStall = false;        // set true by the 's' Serial command -- see loop()
unsigned long stallSimulatedAt = 0;
#endif

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
// already moving or faulted is simply dropped -- no queueing, per "Stability
// Over Complexity." (ELEVATOR_DISABLE isn't checked here -- it's a Modbus
// coil that doesn't exist until Phase B.)
void requestFloor(int floor) {
  if (faulted) {
    lcdPrint("FAULT latched", "Reset to clear");
    return;
  }
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
  delay(300);
  Serial.println();
  Serial.println("=== BSH-Elevator -- Wokwi Simulation (Phase A0, no WiFi/Modbus) ===");

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

#if WOKWI_SIM
  Serial.println("Press Floor 1/2/3 to call the cab. Watch the LCD and the");
  Serial.println("DIRA/DIRB/ENABLE LEDs. While the cab is MOVING, type 's' + Enter");
  Serial.println("in this Serial Monitor to simulate a stalled motor and latch");
  Serial.println("ELEVATOR_FAULT (there's no real motor here to jam on its own).");
#endif
  Serial.println();
}

void loop() {
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

#if WOKWI_SIM
  // Simulation-only stall test hook -- see header. Harmless if unused.
  while (Serial.available()) {
    char c = Serial.read();
    if ((c == 's' || c == 'S') && state == ELEV_MOVING && !faulted && !simulateStall) {
      simulateStall = true;
      stallSimulatedAt = now;
      Serial.println(">>> Simulating a stalled motor. Waiting for the fault watchdog...");
    }
  }
#endif

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
#if WOKWI_SIM
      if (simulateStall) break;  // skip normal arrival; let the watchdog below catch it
#endif
      unsigned long elapsed = now - stateEnteredAt;
      if (elapsed >= expectedTravelMs) {
        motorStop();
        currentFloor = targetFloor;
        state = ELEV_ARRIVED;
        stateEnteredAt = now;
        lcdPrint("Arrived!", "Floor " + String(currentFloor));
      }
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
  // Latches; does not self-clear (representing "needs a technician"),
  // same as the real firmware -- this is deliberately NOT the same thing
  // as ELEVATOR_DISABLE (a remote lockout, Phase B/Modbus only). Keeping
  // the two visibly distinguishable is the point -- see the system doc's
  // Notes section.
  // -------------------------------------------------------------------
  if (state == ELEV_MOVING && !faulted) {
    bool normalTimeout = (now - stateEnteredAt > expectedTravelMs + FAULT_TIMEOUT_MARGIN_MS);
#if WOKWI_SIM
    bool simTimeout = simulateStall && (now - stallSimulatedAt > 2000);
#else
    bool simTimeout = false;
#endif
    if (normalTimeout || simTimeout) {
      motorStop();
      faulted = true;
      lcdPrint("FAULT", "Check elevator");
      Serial.println("ELEVATOR_FAULT latched: movement exceeded expected time.");
    }
  }
}
