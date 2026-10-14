/*
  ============================================================================
  BSH-Perimeter -- PIR Motion Sensor Bench Test (Phase A, this component only)
  ============================================================================
  Purpose: bench-test the PIR motion sensor in isolation, per the checklist
  in Firmware/BSH-Perimeter/PIR_Motion_Sensor/Build_Log.md. No WiFi, no
  Modbus, no ultrasonic sensor -- just this one input, watched over Serial,
  so a bad reading or a wiring mistake gets caught before it's tangled up
  with anything else.

  This is a bench aid, not a file that gets committed to the repo -- the
  project convention (Firmware/WORKFLOW.md Section 3) keeps each component
  subfolder to exactly Wiring.md + Build_Log.md. The real, committed
  firmware for this node is Firmware/BSH_Perimeter_Node.ino (full WiFi +
  Modbus version, Phase B/C) and, once Phase A's two sensors are both
  confirmed, a combined standalone integration sketch modeled on
  BSH_FireAlarm_Node.ino's current Phase A version.

  Wiring (see Firmware/BSH-Perimeter/PIR_Motion_Sensor/Wiring.md):
    OUT -> GPIO 32 (direct digital input, no divider needed)
    VCC -> 5V
    GND -> common ground

  Before trusting any result:
    - Trigger-mode jumper set to REPEATABLE (H) -- retriggerable output
      stays HIGH while motion continues, instead of pulsing once
    - Give the module ~30-60s after power-on to settle before judging
      readings (it free-runs briefly at boot)
    - Sensitivity / hold-time trimmers at a reasonable starting point

  What this sketch does:
    - Reads PIN_PIR every loop iteration (matches the real firmware's
      "raw every loop, no smoothing" approach -- a PIR output is already a
      clean digital trigger)
    - Prints a line only when the state changes (HIGH<->LOW), with a
      timestamp, so it's easy to see how long a trigger holds
    - Also prints a quiet heartbeat once a second so you can tell the
      sketch is still running even with no motion

  Board: "ESP32 Dev Module"
  No libraries needed beyond the ESP32 core.
  ============================================================================
*/

const int PIN_PIR = 32;

bool lastState = false;
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL_MS = 1000;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);

  Serial.println();
  Serial.println("=== BSH-Perimeter PIR Bench Test ===");
  Serial.println("Waiting ~30-60s for the module to settle before trusting readings...");
  Serial.println("Confirm the trigger-mode jumper is set to REPEATABLE (H).");
  Serial.println();
}

void loop() {
  unsigned long now = millis();
  bool motion = (digitalRead(PIN_PIR) == HIGH);

  if (motion != lastState) {
    lastState = motion;
    Serial.print("[");
    Serial.print(now / 1000.0, 1);
    Serial.print("s] MOTION_DETECTED -> ");
    Serial.println(motion ? "TRUE (triggered)" : "false (clear)");
  }

  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeat = now;
    Serial.print(".  current state: ");
    Serial.println(motion ? "TRIGGERED" : "clear");
  }
}
