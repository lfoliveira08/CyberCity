/*
  ============================================================================
  BSH-PowerBox -- Phase A Step 2: Lights Relay Alone (3 LEDs, one per floor)
  ============================================================================
  Purpose: confirm the lights relay channel switches all 3 floor LEDs
  together cleanly, and find out whether your real relay module is
  active-LOW or active-HIGH -- don't assume it matches the Wokwi sim
  (which needed active-HIGH, a simulator-only quirk; most real modules
  are active-LOW).

  Wiring: relay IN1 -> GPIO 13, relay VCC -> 5V, relay GND -> ESP32 GND,
  all 3 LEDs in parallel (each with its own resistor) on the channel-1
  NO/COM side. See Firmware/BSH-PowerBox/Lights_Relay/Wiring.md.

  Usage: open Serial Monitor at 115200 baud, type 'l' + Enter to toggle.
  If the LEDs turn ON when the log says OFF (or vice versa), flip
  RELAY_ACTIVE_LOW below and re-flash. Also check all 3 LEDs are roughly
  equally bright -- a big mismatch usually means a wiring/resistor issue
  on that one branch, not a polarity problem.
  ============================================================================
*/

const int PIN_LIGHTS_RELAY = 13;

// Start with the common real-hardware assumption (active-LOW). Flip this
// if the LEDs behave backwards from what the Serial log says.
const bool RELAY_ACTIVE_LOW = true;

bool lightsOn = false;

void setRelay(int pin, bool on) {
  bool level = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Phase A Step 2: Lights Relay Alone (3 LEDs) ===");
  Serial.println("Type 'l' + Enter to toggle the lights.");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false);  // boot state: off
  Serial.println("Booted OFF.");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'l' || c == 'L') {
      lightsOn = !lightsOn;
      setRelay(PIN_LIGHTS_RELAY, lightsOn);
      Serial.print("Lights command: ");
      Serial.println(lightsOn ? "ON" : "OFF");
      Serial.println(lightsOn
        ? "-> All 3 LEDs should be lit now, roughly evenly. If not, check RELAY_ACTIVE_LOW / wiring."
        : "-> All 3 LEDs should be dark now. If any are still lit, flip RELAY_ACTIVE_LOW.");
    }
  }
}
