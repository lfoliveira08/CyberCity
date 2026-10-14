// FH-HVAC RGB LED bench test -- single component, no network.
// R/G/B -> GPIO 16/17/18 through 220 ohm, common cathode -> GND.
const int PINS[3] = {16, 17, 18};
const char* NAMES[6] = {"red", "green", "blue", "yellow (R+G)", "cyan (G+B)", "white"};
const bool MASK[6][3] = {{1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,1,1}};
int idx = 0;
unsigned long last = 0;
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) pinMode(PINS[i], OUTPUT);
  Serial.println("RGB bench test: steps every 1.5 s");
}
void loop() {
  if (millis() - last >= 1500) {
    last = millis();
    for (int i = 0; i < 3; i++) digitalWrite(PINS[i], MASK[idx][i] ? HIGH : LOW);
    Serial.println(NAMES[idx]);
    idx = (idx + 1) % 6;
  }
}
