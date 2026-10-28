/*
  ============================================================================
  BSH-PowerBox -- PROPOSAL: power rails (Wokwi simulation, Phase A0 style)
  ============================================================================
  DRAFT, opened Oct 27, 2026. NOT the as-built node. The as-built simulation
  is Firmware/BSH-PowerBox/Wokwi/ (lights + fan) and stays untouched. Nothing here is on the
  bench or in the real firmware; Luis has not approved the idea yet.

  Idea: the PowerBox no longer drives a fan. Three relay channels switch the
  USB +5V of the other three BSH nodes (Elevator, FireAlarm, Door). In this
  simulation each downstream node is only an LED: lit = that node has power.

  Pins (SIM-ONLY placeholders for the three rails; not chosen for the bench):
    GPIO 13  Lights relay IN   (unchanged from the as-built node)
    GPIO 4   Lights button     (unchanged, INPUT_PULLUP)
    GPIO 25  Rail 1 relay IN   -> Elevator
    GPIO 26  Rail 2 relay IN   -> FireAlarm
    GPIO 27  Rail 3 relay IN   -> Door
  25/26/27 are not strapping, flash or input-only pins per
  References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md.

  Contact: the rails use the relay's NO (normally open) contact, flipped from
  NC on Oct 27, 2026 at Luis's decision: when the PowerBox is off, the other
  systems are off too ("fail-dark"). Relay de-energised = rail CUT. Energising
  the coil POWERS the rail, so rail on = coil on and the coil draws current
  while the rail is on.
  Decided but NOT shown here (no WiFi/Modbus in the sim): rails turn off on
  comms loss. Open (decision 8): how rails come on after power-up;
  RAILS_ON_AT_BOOT below is only the sim's default.

  Controls (Serial Monitor, type the character + Enter):
    'l'  toggle lights
    '1'  toggle rail 1 (Elevator)   '2' rail 2 (FireAlarm)   '3' rail 3 (Door)
    'a'  cut all three rails        'r' restore all three rails
    Or click the lights button part.

  Real relay polarity: Wokwi's relay energises on HIGH; BSH-PowerBox's real
  boards are also active-HIGH (bench-confirmed Oct 5, 2026) but test every new
  board on its own, do not carry this over.
  ============================================================================
*/

const int PIN_LIGHTS_RELAY  = 13;
const int PIN_LIGHTS_BUTTON = 4;
const int PIN_RAIL_RELAY[3] = {25, 26, 27};
const char* RAIL_NAME[3]    = {"Elevator", "FireAlarm", "Door"};

const bool RELAY_ACTIVE_LOW = false;  // Wokwi: energises on HIGH
const bool RAIL_USES_NC     = false;  // NO contact: coil energised = rail POWERED
const bool RAILS_ON_AT_BOOT = true;   // sim default only; real rule is decision 8, open

bool lightsOn = false;
bool railOn[3] = {RAILS_ON_AT_BOOT, RAILS_ON_AT_BOOT, RAILS_ON_AT_BOOT};

void setRelay(int pin, bool energised) {
  bool level = RELAY_ACTIVE_LOW ? !energised : energised;
  digitalWrite(pin, level ? HIGH : LOW);
}

// Rail ON with the NC contact = coil off; with the NO contact = coil on.
void applyRail(int i) {
  bool energised = RAIL_USES_NC ? !railOn[i] : railOn[i];
  setRelay(PIN_RAIL_RELAY[i], energised);
}

void setRail(int i, bool on) {
  railOn[i] = on;
  applyRail(i);
  Serial.print("Rail ");
  Serial.print(i + 1);
  Serial.print(" (");
  Serial.print(RAIL_NAME[i]);
  Serial.print("): ");
  Serial.println(on ? "POWERED" : "CUT");
}

// Button debounce, same plain-variable style as the as-built sketch.
const unsigned long BUTTON_DEBOUNCE_MS = 50;
int lightsButtonLastReading = HIGH;
int lightsButtonStableState = HIGH;
unsigned long lightsButtonLastChangeTime = 0;

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
  Serial.println("=== BSH-PowerBox PROPOSAL -- power rails (Wokwi, no WiFi/Modbus) ===");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false);
  for (int i = 0; i < 3; i++) {
    pinMode(PIN_RAIL_RELAY[i], OUTPUT);
    applyRail(i);
  }
  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);

  Serial.println("'l' lights | '1' '2' '3' toggle a rail | 'a' cut all | 'r' restore all");
  Serial.println("Boot: lights off, rails per RAILS_ON_AT_BOOT (NO contact: coil on = powered).");
  Serial.println();
}

void loop() {
  if (readButtonEdge(PIN_LIGHTS_BUTTON, lightsButtonLastReading, lightsButtonStableState, lightsButtonLastChangeTime)) {
    lightsOn = !lightsOn;
    setRelay(PIN_LIGHTS_RELAY, lightsOn);
    Serial.print("Lights button pressed -> ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'l' || c == 'L') {
      lightsOn = !lightsOn;
      setRelay(PIN_LIGHTS_RELAY, lightsOn);
      Serial.print("Lights command: ");
      Serial.println(lightsOn ? "ON" : "OFF");
    } else if (c >= '1' && c <= '3') {
      int i = c - '1';
      setRail(i, !railOn[i]);
    } else if (c == 'a' || c == 'A') {
      for (int i = 0; i < 3; i++) setRail(i, false);
    } else if (c == 'r' || c == 'R') {
      for (int i = 0; i < 3; i++) setRail(i, true);
    }
  }

  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("lights=");
    Serial.print(lightsOn ? "ON" : "off");
    for (int i = 0; i < 3; i++) {
      Serial.print("  ");
      Serial.print(RAIL_NAME[i]);
      Serial.print("=");
      Serial.print(railOn[i] ? "powered" : "CUT");
    }
    Serial.println();
  }
}
