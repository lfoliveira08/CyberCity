/*
  ============================================================================
  BSH-PowerBox -- Phase A Step 1: Fan Relay Alone
  ============================================================================
  Purpose: confirm the fan relay channel switches cleanly on its own, and
  find out whether your real relay module is active-LOW or active-HIGH --
  don't assume it matches the Wokwi sim (which needed active-HIGH, a
  simulator-only quirk; most real modules are active-LOW).

  Wiring: relay IN2 -> GPIO 15, relay VCC -> 5V, relay GND -> ESP32 GND,
  fan on the channel-2 NO/COM side. See Firmware/BSH-PowerBox/Fan_Relay/Wiring.md.

  Usage: open Serial Monitor at 115200 baud, type 'f' + Enter to toggle.
  If the fan turns ON when the log says OFF (or vice versa), flip
  RELAY_ACTIVE_LOW below and re-flash.
  ============================================================================
*/

const int PIN_FAN_RELAY = 15;

// Start with the common real-hardware assumption (active-LOW). Flip this
// if the fan behaves backwards from what the Serial log says.
const bool RELAY_ACTIVE_LOW = false;

bool fanOn = false;

void setRelay(int pin, bool on) {
  bool level = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Phase A Step 1: Fan Relay Alone ===");
  Serial.println("Type 'f' + Enter to toggle the fan.");

  pinMode(PIN_FAN_RELAY, OUTPUT);
  setRelay(PIN_FAN_RELAY, false);  // boot state: off
  Serial.println("Booted OFF.");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'f' || c == 'F') {
      fanOn = !fanOn;
      setRelay(PIN_FAN_RELAY, fanOn);
      Serial.print("Fan command: ");
      Serial.println(fanOn ? "ON" : "OFF");
      Serial.println(fanOn
        ? "-> Fan should be spinning now. If it's NOT, flip RELAY_ACTIVE_LOW."
        : "-> Fan should be stopped now. If it's still spinning, flip RELAY_ACTIVE_LOW.");
    }
  }
}
