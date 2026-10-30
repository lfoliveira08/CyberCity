/*
  ============================================================================
  BSH-PowerBox -- Wokwi Simulation (Phase A0)
  ============================================================================
  Purpose: learn how the Power Box system works -- both relay channels, the
  two local override buttons, how they're wired, and how the commanded
  on/off logic behaves -- in the Wokwi simulator, before (or alongside) the
  real bench build. Rule: Firmware/WORKFLOW.md Section 7.

  DESIGN (Sep 28, 2026): this node used to use DALI for dimmable lighting.
  DALI was dropped in favor of a plain relay for lights, the same pattern
  as the fan -- see BSH_Systems/04_BSH-PowerBox.md Notes. Both channels are
  native Wokwi parts now (wokwi-relay-module), so unlike the old DALI
  stand-in, this simulation proves the whole node's logic, not just teaches
  it.

  LOCAL OVERRIDE BUTTONS (Sep 29, 2026): added a real local push-button per
  channel, like an actual room light switch, alongside the Serial "hub
  command" stand-in below -- see BSH_PowerBox_Node.ino's header and the
  Sep 29, 2026 entry in Piranga_Workflow_v2.md Section 14 for the full
  rationale. Here in Phase A0 there's no Modbus coil to fight with, so the
  button and the Serial command both just flip the same lightsOn/fanOn
  boolean directly -- the real firmware's more careful "hub regains
  authority only on a fresh coil write" logic is a Phase B/C concern, not
  needed here.

  This is standalone Phase A0 logic only -- no WiFi, no Modbus, no
  libraries beyond the ESP32 core. It mirrors the relay-driving and button
  logic that BSH_PowerBox_Node.ino uses once Phase B adds Modbus: same pin
  map, same setRelay()/button-debounce handling. Instead of a Modbus coil,
  the "hub command" here comes from the Serial Monitor.

  Parts in diagram.json (same GPIOs as BSH_Systems/04_BSH-PowerBox.md):
    GPIO 13  Lights relay IN     (wokwi-relay-module + wokwi-led "light")
    GPIO 15  Fan relay IN        (wokwi-relay-module + wokwi-led "fan")
    GPIO 4   Lights button       (wokwi-pushbutton, INPUT_PULLUP)
    GPIO 16  Fan button          (wokwi-pushbutton, INPUT_PULLUP)

  Controls:
    Serial Monitor -- type 'l' + Enter to toggle lights, 'f' + Enter for fan
                       (stands in for a hub/Modbus command)
    Click the on-screen buttons -- same toggle, but models the local
                       physical override switch

  GPIO NOTE (Sep 28, 2026): lights was originally on GPIO 12, carried over
  from the old DALI TX pin. GPIO 12 is an ESP32 strapping pin (MTDI) that
  samples flash voltage at reset. Moved to GPIO 13. See
  BSH_Systems/04_BSH-PowerBox.md Notes and this folder's README for what
  that turned out NOT to fully explain (the real cause was Wokwi's relay
  polarity, below).
  ============================================================================
*/

// ---------------------------------------------------------------------------
// GPIO pin map -- identical to the full BSH-PowerBox node firmware
// ---------------------------------------------------------------------------
const int PIN_LIGHTS_RELAY  = 13;
const int PIN_FAN_RELAY     = 15;
const int PIN_LIGHTS_BUTTON = 4;
const int PIN_FAN_BUTTON    = 16;

// Most cheap real relay modules trigger active-LOW (LOW = energized), but
// Wokwi's simulated wokwi-relay-module part turned out to energize on HIGH
// instead -- both channels tested inverted in Phase A0 (external LED AND
// the relay's own onboard LED1), once both were checked carefully. Lights
// was caught first; fan was actually wrong the whole time too, just not
// noticed until lights was fixed and fan was compared against it again.
// Set both to active-HIGH to match what Wokwi's part actually does.
// IMPORTANT: this is a Wokwi-simulator quirk, not necessarily true of the
// real relay hardware -- when you move to the physical bench build, put
// this back to RELAY_ACTIVE_LOW = true (active-LOW) for both channels
// unless bench testing shows otherwise. Don't carry "false" over blindly.
const bool RELAY_LIGHTS_ACTIVE_LOW = false;  // Wokwi-only, energizes on HIGH
const bool RELAY_FAN_ACTIVE_LOW    = false;  // Wokwi-only, energizes on HIGH

// ---------------------------------------------------------------------------
// Commanded/actual state -- toggled from Serial (hub stand-in) or a local
// button (physical override); a Modbus coil write once Phase B firmware is
// running for real
// ---------------------------------------------------------------------------
bool lightsOn = false;
bool fanOn    = false;

void setRelay(int pin, bool on, bool activeLow) {
  bool level = activeLow ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

// ---------------------------------------------------------------------------
// Button debounce (non-blocking, ~50ms) -- returns true once per press
// (falling edge only; button is wired INPUT_PULLUP so LOW == pressed)
//
// Plain per-button variables instead of a shared struct: the Arduino/Wokwi
// build auto-generates function prototypes and inserts them near the top
// of the file, ahead of any struct you define -- so a function taking a
// custom struct by reference (e.g. ButtonState &btn) fails to compile with
// "'ButtonState' was not declared in this scope", even though the struct
// IS defined earlier in this same file. Passing plain int/unsigned long by
// reference instead sidesteps that bug entirely.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Status print throttle (once a second)
// ---------------------------------------------------------------------------
const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-PowerBox -- Wokwi Simulation (Phase A0, no WiFi/Modbus) ===");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  pinMode(PIN_FAN_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false, RELAY_LIGHTS_ACTIVE_LOW);  // boot state: lights off
  setRelay(PIN_FAN_RELAY, false, RELAY_FAN_ACTIVE_LOW);        // boot state: fan off

  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FAN_BUTTON, INPUT_PULLUP);

  Serial.println("Type 'l' + Enter to toggle lights, 'f' + Enter to toggle fan,");
  Serial.println("or click the lights/fan button parts in the diagram.");
  Serial.println("Both channels boot OFF -- a dark, idle room is this node's safe default.");
  Serial.println();
}

void loop() {
  // -------------------------------------------------------------------
  // Local button input -- checked every loop (not gated behind the print
  // throttle) so the millis()-based debounce timing is accurate
  // -------------------------------------------------------------------
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

  // -------------------------------------------------------------------
  // Serial command input -- stands in for a hub/Modbus coil write until
  // Phase B firmware exists
  // -------------------------------------------------------------------
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
    // ignore newline/other characters
  }

  // -------------------------------------------------------------------
  // Status print, once a second
  // -------------------------------------------------------------------
  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("lights=");
    Serial.print(lightsOn ? "ON" : "off");
    Serial.print("  fan=");
    Serial.println(fanOn ? "ON" : "off");
  }
}
