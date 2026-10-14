// FH-HVAC DHT11 bench test -- single component, no network.
// DATA -> GPIO 4, VCC -> 3.3V, GND -> GND. Library: "DHT sensor library" (Adafruit).
#include <DHT.h>
DHT dht(4, DHT11);   // real bench part is a DHT11 (whole degrees)
unsigned long last = 0;
void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("DHT11 bench test: one read every 2 s");
}
void loop() {
  if (millis() - last >= 2000) {
    last = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("READ FAILED (nan) -- check DATA wire, pull-up, power");
    } else {
      Serial.print("T=");  Serial.print(t, 1);
      Serial.print(" C  RH="); Serial.print(h, 0);
      Serial.println(" %");
    }
  }
}
