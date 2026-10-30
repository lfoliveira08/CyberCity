/*
  ============================================================================
  BSH-PowerBox -- power rails (Wokwi simulation, Phase A0)
  ============================================================================
  Opened as a proposal Oct 27, 2026; lights/fan removed and adopted as the
  real design Oct 28, 2026 (see BSH_Systems/04_BSH-PowerBox.md). This is now
  the as-built Phase A0 simulation for this node. The identical working copy
  used during the prototype bench testing is Prototype_PowerRails/Wokwi/.

  Idea: the PowerBox no longer drives a fan or lights. Three relay channels
  switch power to the other three BSH nodes. In the simulation each downstream
  node is only an LED: lit = that node has power. On the bench the relay's NO
  contact feeds a node's 5V pin (node USB unplugged) -- the "5V-pin method",
  decided Oct 28, 2026.

  Pins (rail-to-pin map by Luis, Oct 28, 2026):
    GPIO 25  Rail 1 relay IN   -> Elevator
    GPIO 26  Rail 2 relay IN   -> FireAlarm
    GPIO 27  Rail 3 relay IN   -> Door
  25/26/27 are not strapping, flash or input-only pins per
  References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md.

  Contact: NO (normally open), decided Oct 27, 2026: when the PowerBox is off,
  the other systems are off too ("fail-dark"). Relay de-energised = rail CUT.
  Energising the coil POWERS the rail, so rail on = coil on and the coil draws
  current while the rail is on.
  Decided but NOT shown here (no WiFi/Modbus): rails turn off on comms loss.
  Open (decision 8): how rails come on after power-up; RAILS_ON_AT_BOOT below
  is only the sketch's default.

  Controls (Serial Monitor at 115200, type the character + Enter):
    '1'  toggle rail 1 (Elevator)   '2' rail 2 (FireAlarm)   '3' rail 3 (Door)
    'a'  cut all three rails        'r' restore all three rails

  Real relay polarity: Wokwi's relay energises on HIGH. Each real board must
  be tested on its own; do not carry a result from one board to another.
  ============================================================================
*/

const int PIN_RAIL_RELAY[3] = {25, 26, 27};
const char* RAIL_NAME[3]    = {"Elevator", "FireAlarm", "Door"};

const bool RELAY_ACTIVE_LOW = false;  // Wokwi: energises on HIGH; test each real board
const bool RAIL_USES_NC     = false;  // NO contact: coil energised = rail POWERED
const bool RAILS_ON_AT_BOOT = true;   // sim default only; real rule is decision 8, open

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

const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-PowerBox PROTOTYPE -- power rails (no WiFi/Modbus) ===");

  for (int i = 0; i < 3; i++) {
    pinMode(PIN_RAIL_RELAY[i], OUTPUT);
    applyRail(i);
  }

  Serial.println("'1' '2' '3' toggle a rail | 'a' cut all | 'r' restore all");
  Serial.println("Boot: rails per RAILS_ON_AT_BOOT (NO contact: coil on = powered).");
  Serial.println();
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '3') {
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
    for (int i = 0; i < 3; i++) {
      Serial.print(RAIL_NAME[i]);
      Serial.print("=");
      Serial.print(railOn[i] ? "powered" : "CUT");
      Serial.print("  ");
    }
    Serial.println();
  }
}
