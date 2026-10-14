/*
  ============================================================================
  BSH-Elevator -- Push Buttons Bench Test (Phase A, this component only)
  ============================================================================
  Purpose: bench-test the 3 floor-call buttons in isolation, per the
  checklist in Firmware/BSH-Elevator/Push_Buttons/Build_Log.md. No LCD, no
  motor, no WiFi, no Modbus -- just these three inputs, watched over Serial,
  so a wiring mistake or a debounce problem gets caught before it's tangled
  up with the rest of the state machine.

  This is a bench aid, not a file that gets committed to the repo -- the
  project convention (Firmware/WORKFLOW.md Section 3) keeps each component
  subfolder to exactly Wiring.md + Build_Log.md. The real, committed
  firmware for this node is Firmware/BSH_Elevator_Node.ino (full WiFi +
  Modbus version, Phase B/C) and, once Phase A's three components are all
  confirmed, a combined standalone integration sketch modeled on
  BSH_FireAlarm_Node.ino's current Phase A version -- same debounce
  approach as this test, just wired into the full door-close/moving/arrived
  state machine instead of a bare Serial print.

  Wiring (see Firmware/BSH-Elevator/Push_Buttons/Wiring.md):
    Floor 1 -> GPIO 4,  other leg -> GND
    Floor 2 -> GPIO 16, other leg -> GND
    Floor 3 -> GPIO 17, other leg -> GND
    INPUT_PULLUP in firmware -- no external resistor needed on any of the
    three (none of these pins are in the input-only 34-39 range that lacks
    an internal pull-up).

  What this sketch does:
    - Reads all three buttons every loop iteration with the same ~50ms
      debounce struct the real firmware uses
    - Prints a line only on a debounced rising edge (press), with a
      timestamp -- a single physical press should print exactly once per
      button, which is the debounce check called out in the Build_Log
      checklist
    - Also prints a quiet heartbeat once a second with the live raw state
      of all three, so you can tell the sketch is still running and can
      spot a floating/stuck pin even between presses

  Board: "ESP32 Dev Module"
  No libraries needed beyond the ESP32 core.
  ============================================================================
*/

const int PIN_BTN_FLOOR_1 = 4;
const int PIN_BTN_FLOOR_2 = 16;
const int PIN_BTN_FLOOR_3 = 17;

const unsigned long DEBOUNCE_MS = 50;
struct ButtonState {
  bool rawLast = false;
  bool debounced = false;
  unsigned long lastChange = 0;
};
ButtonState btn1, btn2, btn3;

bool readDebounced(int pin, ButtonState &s, unsigned long now) {
  bool raw = (digitalRead(pin) == LOW);   // active LOW
  if (raw != s.rawLast) {
    s.lastChange = now;
    s.rawLast = raw;
  }
  if (now - s.lastChange > DEBOUNCE_MS) {
    s.debounced = raw;
  }
  return s.debounced;
}

unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL_MS = 1000;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN_FLOOR_1, INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR_2, INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR_3, INPUT_PULLUP);

  Serial.println();
  Serial.println("=== BSH-Elevator Push Buttons Bench Test ===");
  Serial.println("Press each floor button once. Each press should print exactly");
  Serial.println("one line below -- if a single press prints twice, debounce or");
  Serial.println("wiring needs a look before moving on to the LCD.");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  bool p1 = readDebounced(PIN_BTN_FLOOR_1, btn1, now);
  bool p2 = readDebounced(PIN_BTN_FLOOR_2, btn2, now);
  bool p3 = readDebounced(PIN_BTN_FLOOR_3, btn3, now);

  static bool p1Last = false, p2Last = false, p3Last = false;
  if (p1 && !p1Last) {
    Serial.print("[");
    Serial.print(now / 1000.0, 1);
    Serial.println("s] Floor 1 button PRESSED");
  }
  if (p2 && !p2Last) {
    Serial.print("[");
    Serial.print(now / 1000.0, 1);
    Serial.println("s] Floor 2 button PRESSED");
  }
  if (p3 && !p3Last) {
    Serial.print("[");
    Serial.print(now / 1000.0, 1);
    Serial.println("s] Floor 3 button PRESSED");
  }
  p1Last = p1; p2Last = p2; p3Last = p3;

  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeat = now;
    Serial.print(".  raw state -- Floor1: ");
    Serial.print(digitalRead(PIN_BTN_FLOOR_1) == LOW ? "LOW(pressed)" : "HIGH(released)");
    Serial.print("  Floor2: ");
    Serial.print(digitalRead(PIN_BTN_FLOOR_2) == LOW ? "LOW(pressed)" : "HIGH(released)");
    Serial.print("  Floor3: ");
    Serial.println(digitalRead(PIN_BTN_FLOOR_3) == LOW ? "LOW(pressed)" : "HIGH(released)");
  }
}
