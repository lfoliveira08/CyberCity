/*
  ============================================================================
  BSH-PowerBox -- Phase A Step 3: Buttons Alone
  ============================================================================
  Purpose: confirm each button reads cleanly and debounces properly, with
  no relay involved yet -- isolates button wiring/behavior from relay
  behavior, so a problem in one step doesn't get confused with the other.

  Wiring: lights button one leg -> GPIO 4, other leg -> GND.
  fan button one leg -> GPIO 16, other leg -> GND. No external resistor
  needed -- firmware uses INPUT_PULLUP. See
  Firmware/BSH-PowerBox/Wiring docs / BSH_Sensors/02_Push_Button.md.

  Usage: open Serial Monitor at 115200 baud. Press each button once and
  confirm exactly ONE "pressed" line prints per physical press -- if you
  see two or more lines from a single press, the button is bouncing worse
  than expected; increase BUTTON_DEBOUNCE_MS and re-test.
  ============================================================================
*/

const int PIN_LIGHTS_BUTTON = 4;
const int PIN_FAN_BUTTON    = 16;

const unsigned long BUTTON_DEBOUNCE_MS = 50;

int lightsButtonLastReading = HIGH;
int lightsButtonStableState = HIGH;
unsigned long lightsButtonLastChangeTime = 0;

int fanButtonLastReading = HIGH;
int fanButtonStableState = HIGH;
unsigned long fanButtonLastChangeTime = 0;

bool readButtonEdge(int pin, int &lastReading, int &stableState, unsigned long &lastChangeTime) {
  int reading = digitalRead(pin);
  if (reading != lastReading) {
    lastChangeTime = millis();
  }
  bool pressedEdge = false;
  if ((millis() - lastChangeTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) {  // active LOW: this is a press
        pressedEdge = true;
      }
    }
  }
  lastReading = reading;
  return pressedEdge;
}

unsigned long lightsPressCount = 0;
unsigned long fanPressCount = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Phase A Step 3: Buttons Alone ===");
  Serial.println("Press each button once at a time and watch for exactly one line per press.");

  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FAN_BUTTON, INPUT_PULLUP);
}

void loop() {
  if (readButtonEdge(PIN_LIGHTS_BUTTON, lightsButtonLastReading, lightsButtonStableState, lightsButtonLastChangeTime)) {
    lightsPressCount++;
    Serial.print("Lights button pressed (count: ");
    Serial.print(lightsPressCount);
    Serial.println(")");
  }
  if (readButtonEdge(PIN_FAN_BUTTON, fanButtonLastReading, fanButtonStableState, fanButtonLastChangeTime)) {
    fanPressCount++;
    Serial.print("Fan button pressed (count: ");
    Serial.print(fanPressCount);
    Serial.println(")");
  }
}
