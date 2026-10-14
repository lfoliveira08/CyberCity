/*
  ============================================================================
  BSH-PowerBox -- Phase A Step 4: Combined Standalone Integration Sketch
  ============================================================================
  Purpose: both relay channels + both buttons together, no WiFi, no Modbus.
  This is the real Phase A "everything at once" test before Phase B adds
  networking. Once this behaves correctly, Phase B swaps Serial commands
  for real Modbus coils (that's BSH_PowerBox_Node.ino, already written).

  Wiring: same as before --
    GPIO 13 -> Lights relay IN (3 LEDs in parallel on the switched side)
    GPIO 15 -> Fan relay IN
    GPIO 4  -> Lights override button (INPUT_PULLUP, other leg to GND)
    GPIO 16 -> Fan override button (INPUT_PULLUP, other leg to GND)

  BEFORE FLASHING THIS: set RELAY_LIGHTS_ACTIVE_LOW / RELAY_FAN_ACTIVE_LOW
  below based on what Phase A Steps 1 and 2 actually found on your bench --
  don't assume both channels match each other, and don't carry over the
  Wokwi sim's active-HIGH finding (that was simulator-only).

  Controls (Serial Monitor, 115200 baud):
    l / L -- toggle lights (stands in for a future Modbus coil write)
    f / F -- toggle fan (same)
  Or press the physical buttons -- same effect, models the local override
  switch. In this standalone sketch there's no "hub" to conflict with, so
  both input sources just flip the same variable directly (the more
  careful "hub regains authority only on a fresh coil write" logic lives
  in BSH_PowerBox_Node.ino once Phase B adds Modbus -- see its header
  comment for the full authority rule).
  ============================================================================
*/

const int PIN_LIGHTS_RELAY  = 13;
const int PIN_FAN_RELAY     = 15;
const int PIN_LIGHTS_BUTTON = 4;
const int PIN_FAN_BUTTON    = 16;

// Set these from what Phase A Steps 1 & 2 actually found on the bench.
const bool RELAY_LIGHTS_ACTIVE_LOW = true;
const bool RELAY_FAN_ACTIVE_LOW    = true;

bool lightsOn = false;
bool fanOn    = false;

void setRelay(int pin, bool on, bool activeLow) {
  bool level = activeLow ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

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
      if (stableState == LOW) {
        pressedEdge = true;
      }
    }
  }
  lastReading = reading;
  return pressedEdge;
}

const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-PowerBox -- Phase A Combined Standalone Test ===");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  pinMode(PIN_FAN_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false, RELAY_LIGHTS_ACTIVE_LOW);
  setRelay(PIN_FAN_RELAY, false, RELAY_FAN_ACTIVE_LOW);

  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FAN_BUTTON, INPUT_PULLUP);

  Serial.println("Type 'l' + Enter to toggle lights, 'f' + Enter to toggle fan,");
  Serial.println("or press the physical buttons. Both channels boot OFF.");
  Serial.println();
}

void loop() {
  if (readButtonEdge(PIN_LIGHTS_BUTTON, lightsButtonLastReading, lightsButtonStableState, lightsButtonLastChangeTime)) {
    lightsOn = !lightsOn;
    setRelay(PIN_LIGHTS_RELAY, lightsOn, RELAY_LIGHTS_ACTIVE_LOW);
    Serial.print("Lights button pressed -> ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }
  if (readButtonEdge(PIN_FAN_BUTTON, fanButtonLastReading, fanButtonStableState, fanButtonLastChangeTime)) {
    fanOn = !fanOn;
    setRelay(PIN_FAN_RELAY, fanOn, RELAY_FAN_ACTIVE_LOW);
    Serial.print("Fan button pressed -> ");
    Serial.println(fanOn ? "ON" : "OFF");
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'l' || c == 'L') {
      lightsOn = !lightsOn;
      setRelay(PIN_LIGHTS_RELAY, lightsOn, RELAY_LIGHTS_ACTIVE_LOW);
      Serial.print("Lights command: ");
      Serial.println(lightsOn ? "ON" : "OFF");
    } else if (c == 'f' || c == 'F') {
      fanOn = !fanOn;
      setRelay(PIN_FAN_RELAY, fanOn, RELAY_FAN_ACTIVE_LOW);
      Serial.print("Fan command: ");
      Serial.println(fanOn ? "ON" : "OFF");
    }
  }

  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("lights=");
    Serial.print(lightsOn ? "ON" : "off");
    Serial.print("  fan=");
    Serial.println(fanOn ? "ON" : "off");
  }
}
