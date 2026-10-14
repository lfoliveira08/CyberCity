/*
  ============================================================================
  BSH-Perimeter -- HC-SR04 Ultrasonic Sensor Bench Test (Phase A, this
  component only)
  ============================================================================
  Purpose: bench-test the ultrasonic ranger in isolation, per the checklist
  in Firmware/BSH-Perimeter/HC-SR04_Ultrasonic_Sensor/Build_Log.md. No
  WiFi, no Modbus, no PIR sensor -- just this one input, watched over
  Serial.

  *** STOP -- before connecting ECHO to the ESP32 ***
  ECHO is a 5V output. Do not wire it directly to a GPIO. Build the
  1kOhm/2kOhm voltage divider first (1k from ECHO to the GPIO, 2k from
  that same GPIO to GND) and VERIFY THE VOLTAGE AT THE GPIO SIDE WITH A
  MULTIMETER (~3.3V, not 5V) before ever plugging it into the ESP32. See
  Firmware/BSH-Perimeter/HC-SR04_Ultrasonic_Sensor/Wiring.md for the math
  and the full wiring table. This is the single step most likely to get
  skipped, and skipping it risks the GPIO.

  This is a bench aid, not a file that gets committed to the repo (see the
  note in the PIR bench-test sketch about Firmware/WORKFLOW.md Section 3).
  The real, committed firmware is Firmware/BSH_Perimeter_Node.ino
  (Phase B/C) and, later, a combined Phase A integration sketch.

  Wiring (see Wiring.md):
    TRIG -> GPIO 14 (direct, 3.3V-safe)
    ECHO -> GPIO 27 (THROUGH THE DIVIDER -- see above)
    VCC  -> 5V
    GND  -> common ground, shared with the divider's ground leg

  What this sketch does (mirrors readDistanceCm() in BSH_Perimeter_Node.ino
  exactly, so bench results transfer directly to the real firmware):
    - Pulses TRIG, reads ECHO with a bounded pulseIn() (25ms timeout -- this
      is short enough that a hung sensor can't freeze the sketch)
    - A timed-out or out-of-range (>400cm) reading reports as 9999
      (NO_ECHO_SENTINEL_CM), never a bare 0 -- 0 would look like "something
      is touching the sensor," which is not what a missing echo means
    - Samples once every 150ms, not every loop iteration
    - Also reports OBJECT_IN_RANGE against the same 50cm placeholder
      threshold used in the real firmware (OBJECT_RANGE_THRESHOLD_CM) --
      recalibrate that constant once the real printed perimeter geometry
      exists; don't leave it for the night before the demo

  Bench checks to run:
    - Hold an object at a few known distances, confirm the reported value
      is reasonably close
    - Point the sensor at nothing solid / move the object out of range,
      confirm it reports 9999, not 0 or garbage
    - Watch for wildly jumping readings (usually a divider wiring issue or
      the sensor mount vibrating)

  Board: "ESP32 Dev Module"
  No libraries needed beyond the ESP32 core.
  ============================================================================
*/

const int PIN_TRIG = 14;
const int PIN_ECHO  = 27;  // through the 1k/2k voltage divider - see above

const unsigned long SAMPLE_INTERVAL_MS = 150;
unsigned long lastSample = 0;

const unsigned long ECHO_TIMEOUT_US = 25000UL;   // ~4.3m round trip
const uint16_t NO_ECHO_SENTINEL_CM  = 9999;      // never a real reading

// Placeholder - same value as BSH_Perimeter_Node.ino, recalibrate later.
const int OBJECT_RANGE_THRESHOLD_CM = 50;

uint16_t readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return NO_ECHO_SENTINEL_CM;  // timed out - no valid echo
  }

  // Speed of sound ~343 m/s -> ~58.3us round-trip per cm.
  unsigned long cm = duration / 58UL;
  if (cm > 400UL) {
    return NO_ECHO_SENTINEL_CM;  // outside the sensor's rated range
  }
  return (uint16_t)cm;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.println();
  Serial.println("=== BSH-Perimeter HC-SR04 Bench Test ===");
  Serial.println("Confirm the ECHO divider was verified with a multimeter (~3.3V) BEFORE trusting this.");
  Serial.println();
}

void loop() {
  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;

    uint16_t distanceCm = readDistanceCm();
    bool inRange = (distanceCm != NO_ECHO_SENTINEL_CM) &&
                   ((int)distanceCm <= OBJECT_RANGE_THRESHOLD_CM);

    Serial.print("DISTANCE_CM = ");
    if (distanceCm == NO_ECHO_SENTINEL_CM) {
      Serial.print("9999 (no echo / out of range)");
    } else {
      Serial.print(distanceCm);
    }
    Serial.print("   OBJECT_IN_RANGE = ");
    Serial.println(inRange ? "TRUE" : "false");
  }
}
