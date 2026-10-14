// FH-HVAC relay bench test -- single component, no network.
// Relay IN -> GPIO 26. Watch the relay's onboard LED and listen for the click.
// Serial output says what the SKETCH commands; compare with what the relay does.
// If the relay is ON when the sketch says OFF, flip RELAY_ACTIVE_LOW.
const int PIN_RELAY = 26;
const bool RELAY_ACTIVE_LOW = true;   // untested default -- change after this test
bool on = false;
unsigned long last = 0;
void setRelay(bool v) { digitalWrite(PIN_RELAY, (v != RELAY_ACTIVE_LOW) ? HIGH : LOW); }
void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY, OUTPUT);
  setRelay(false);
  Serial.println("Relay bench test: toggles every 2 s");
}
void loop() {
  if (millis() - last >= 2000) {
    last = millis();
    on = !on;
    setRelay(on);
    Serial.println(on ? "sketch says: relay ON" : "sketch says: relay OFF");
  }
}
