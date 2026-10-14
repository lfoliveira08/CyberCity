# FH-HVAC -- System Report (Phase A)

Written 2026-10-07 per `Firmware/WORKFLOW.md` Section 4.

> **Status: Phase A bench-confirmed 2026-10-07; Phase B (WiFi + Modbus) since flashed and
> bench-confirmed, including the sensor-unplug fault test and the WiFi-cut fail-off (see the
> decisions log and `ESP32_Core_Modbus/Build_Log.md`).** This report is the Phase A record. This report
> keeps the Phase A sketch in full below; `FH_HVAC_Node/FH_HVAC_Node.ino` is replaced in place by
> the Phase B (WiFi + Modbus) version, so this report is the record of what Phase A was.

## 1. How the system works
Founder's Hall slot 1. A zone air-conditioning unit: a DHT11 reads temperature and humidity every
2 seconds; when the temperature is at least 0.5 C above the setpoint a relay switches a small DC motor
with a fan blade on, and switches it off when the temperature falls to 0.5 C below the setpoint
(hysteresis, so it does not chatter). An RGB LED shows state. `HVAC_ENABLE` forces the unit off.

- **Scaling:** temperature and setpoint are integer tenths of a degree C (24.0 C = 240); humidity is whole %.
  The DHT11 reads whole degrees, so readings are multiples of 10 and the effective hysteresis is one full degree.
- **Fault:** 3 consecutive bad reads (NaN or outside 0-50 C) latch `HVAC_FAULT`: relay forced open, LED red,
  reading shown as `---`, never 0. Cleared only by a reset.
- **Fail direction:** fail-off (decided 2026-10-07). Sensor fault or `HVAC_ENABLE` clear opens the relay;
  in Phase B a WiFi outage longer than 10 s does too.
- **LED:** green idle, blue cooling, yellow disabled, red fault.
- **Phase A stand-ins for Modbus:** Serial `e` toggles `HVAC_ENABLE`; `+` / `-` change the setpoint by 1.0 C.
- **Power:** the motor runs from the ESP32 board's 5V pin through the relay, with a 1N4007 flyback diode across it.

## 2. The code (Phase A, as bench-confirmed)

```cpp
/*
 * FH-HVAC Node -- Phase A: standalone integration test (no WiFi, no Modbus)
 * ============================================================================
 * STATUS (2026-10-07): written for Phase A0 (Wokwi) and Phase A (bench).
 * Not yet bench-confirmed. Phase B (WiFi + Modbus) has NOT started; when it does
 * this file is replaced in place (path never changes, Firmware/WORKFLOW.md S8).
 *
 * Role: Founder's Hall zone HVAC unit. Reads a DHT11 (DHT22 in Wokwi), compares the temperature
 * to a setpoint, and drives a relay (compressor/fan stand-in) plus an RGB status
 * LED. Design doc: FH_Systems/01_FH-HVAC.md
 *
 * Design decisions (settled 2026-10-07, see FH_HVAC_Bringup_Checklist.md S0):
 *   - SCALING: temperatures are integer TENTHS of a degree C (23.4 C -> 234).
 *     TEMP_C and TARGET_TEMP_C use the SAME scale. Humidity is whole percent.
 *   - FAIL DIRECTION: fail-OFF. A sensor fault, or (Phase B) loss of comms,
 *     forces the relay open. Same direction as ELEVATOR_DISABLE.
 *
 * Phase B register map this sketch is laid out for (not implemented yet):
 *   IR0 TEMP_C (tenths)   IR1 HUMIDITY_PCT   IR2 HEARTBEAT   IR3 RECONNECT_COUNT
 *   DI0 COMPRESSOR_RUNNING   DI1 HVAC_FAULT
 *   HR0 TARGET_TEMP_C (tenths)   Coil0 HVAC_ENABLE
 *   In this sketch: hvacEnable == Coil0, targetTenths == HR0,
 *   relayOn == DI0, faulted == DI1, tempTenths == IR0, humidityPct == IR1.
 *
 * Hardware (FH_Systems/01_FH-HVAC.md):
 *   DHT11 DATA -> GPIO 4  (3-pin module: pull-up already fitted, no resistor needed)
 *   Relay IN   -> GPIO 26
 *   RGB R/G/B  -> GPIO 16 / 17 / 18 (220 ohm each, common cathode to GND)
 *
 * LED colours: green = idle, blue = cooling, yellow = disabled, red = FAULT.
 *
 * Serial commands (115200 baud, Phase A stand-ins for the Modbus writes):
 *   e = toggle HVAC_ENABLE     + = setpoint +1.0 C     - = setpoint -1.0 C
 *   f = (Wokwi only) simulate a failed sensor read
 *
 * Libraries: "DHT sensor library" + "Adafruit Unified Sensor" (Adafruit).
 * Board: "ESP32 Dev Module"
 * ASCII only in this file (Windows Arduino IDE rejects non-ASCII).
 * ============================================================================
 */

#include <DHT.h>

#define WOKWI_SIM 0   // 1 in Firmware/FH-HVAC/Wokwi/sketch.ino, 0 on real hardware

// ---------- Pins ----------
const int PIN_DHT   = 4;
const int PIN_RELAY = 26;
const int PIN_LED_R = 16;
const int PIN_LED_G = 17;
const int PIN_LED_B = 18;

// ---------- Relay polarity ----------
// Bench-confirmed 2026-10-07: the keyestudio relay module on this node is
// active-HIGH (relay energizes when the pin is HIGH). Wokwi's relay is too.
const bool RELAY_ACTIVE_LOW = false;

// ---------- Tuning ----------
const unsigned long SAMPLE_INTERVAL_MS = 2000;  // DHT22 max ~0.5 Hz (DHT11 1 Hz) -- never faster
const int  FAULT_AFTER_FAILS = 3;               // consecutive bad reads before latching
const int  HYSTERESIS_TENTHS = 5;               // +/-0.5 C around the setpoint
const int  TARGET_MIN_TENTHS = 100;             // 10.0 C
const int  TARGET_MAX_TENTHS = 350;             // 35.0 C

// ---------- Sensor model ----------
// Real bench part is a DHT11 (3-pin module): whole-degree resolution, +/-2 C,
// 0-50 C. Wokwi only offers the DHT22, so the simulation uses that one.
// Readings are still stored as tenths; a DHT11 simply always gives multiples of 10.
#if WOKWI_SIM
#define DHT_MODEL DHT22
const int  TEMP_VALID_MIN_TENTHS = -400;        // DHT22 spec range
const int  TEMP_VALID_MAX_TENTHS = 800;
#else
#define DHT_MODEL DHT11
const int  TEMP_VALID_MIN_TENTHS = 0;           // DHT11 spec range
const int  TEMP_VALID_MAX_TENTHS = 500;
#endif

DHT dht(PIN_DHT, DHT_MODEL);

// ---------- State ----------
int  tempTenths   = 0;      // only meaningful when haveReading is true
int  humidityPct  = 0;
bool haveReading  = false;  // false until the first good read -- never report a fake 0
int  targetTenths = 240;    // 24.0 C default setpoint (HR0 in Phase B)
bool hvacEnable   = true;   // Coil0 in Phase B
bool cooling      = false;  // relay on (DI0 in Phase B)
bool faulted      = false;  // latched until reset (DI1 in Phase B)
int  failCount    = 0;
unsigned long lastSampleMs = 0;
#if WOKWI_SIM
bool simFail = false;
#endif

// ---------- Helpers ----------
void setRelay(bool on) {
  digitalWrite(PIN_RELAY, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r ? HIGH : LOW);
  digitalWrite(PIN_LED_G, g ? HIGH : LOW);
  digitalWrite(PIN_LED_B, b ? HIGH : LOW);
}

void printTenths(int v) {
  if (v < 0) { Serial.print('-'); v = -v; }
  Serial.print(v / 10);
  Serial.print('.');
  Serial.print(v % 10);
}

void updateOutputs() {
  if (faulted || !hvacEnable || !haveReading) {
    cooling = false;                       // fail-off
  } else if (!cooling && tempTenths >= targetTenths + HYSTERESIS_TENTHS) {
    cooling = true;
  } else if (cooling && tempTenths <= targetTenths - HYSTERESIS_TENTHS) {
    cooling = false;
  }
  setRelay(cooling);

  if (faulted)         setRGB(true,  false, false);  // red
  else if (!hvacEnable) setRGB(true,  true,  false);  // yellow
  else if (cooling)    setRGB(false, false, true);   // blue
  else                 setRGB(false, true,  false);  // green
}

void sampleSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
#if WOKWI_SIM
  if (simFail) { t = NAN; h = NAN; }
#endif
  bool ok = !isnan(t) && !isnan(h);
  int tt = 0;
  if (ok) {
    tt = (int)lroundf(t * 10.0f);
    if (tt < TEMP_VALID_MIN_TENTHS || tt > TEMP_VALID_MAX_TENTHS) ok = false;
  }

  if (ok) {
    failCount = 0;
    tempTenths = tt;
    humidityPct = (int)lroundf(h);
    haveReading = true;
  } else {
    failCount++;
    if (failCount >= FAULT_AFTER_FAILS && !faulted) {
      faulted = true;
      Serial.println("HVAC_FAULT latched: sensor read failed repeatedly. Relay forced open. Reset to clear.");
    }
  }

  updateOutputs();

  Serial.print("T=");
  if (ok) printTenths(tempTenths); else Serial.print("---");
  Serial.print("C RH=");
  if (ok) Serial.print(humidityPct); else Serial.print("--");
  Serial.print("% target=");
  printTenths(targetTenths);
  Serial.print("C enable=");  Serial.print(hvacEnable ? 1 : 0);
  Serial.print(" cooling=");  Serial.print(cooling ? 1 : 0);
  Serial.print(" fault=");    Serial.print(faulted ? 1 : 0);
  Serial.print(" fails=");    Serial.println(failCount);
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'e') {
      hvacEnable = !hvacEnable;
      Serial.print("HVAC_ENABLE -> "); Serial.println(hvacEnable ? 1 : 0);
      updateOutputs();
    } else if (c == '+' || c == '-') {
      targetTenths += (c == '+') ? 10 : -10;
      if (targetTenths > TARGET_MAX_TENTHS) targetTenths = TARGET_MAX_TENTHS;
      if (targetTenths < TARGET_MIN_TENTHS) targetTenths = TARGET_MIN_TENTHS;
      Serial.print("TARGET_TEMP_C -> "); printTenths(targetTenths); Serial.println("C");
      updateOutputs();
    }
#if WOKWI_SIM
    else if (c == 'f') {
      simFail = !simFail;
      Serial.print("sim: sensor read failure -> "); Serial.println(simFail ? "ON" : "OFF");
    }
#endif
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  setRelay(false);          // known-off before anything else
  setRGB(false, false, false);
  dht.begin();
  updateOutputs();
  lastSampleMs = millis();  // first sample after one full interval (sensor warm-up)
  Serial.println("FH-HVAC Phase A standalone test. Commands: e=enable toggle, +/- = setpoint"
#if WOKWI_SIM
                 ", f=fail read"
#endif
                 );
}

void loop() {
  handleSerial();
  unsigned long now = millis();
  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;
    sampleSensor();
  }
}
```

## 3. The wiring
| Part | Part pin | ESP32 pin | Notes |
|---|---|---|---|
| DHT11 (keyestudio 3-pin module) | G | GND | pull-up already on the module |
| | V | 3V3 | |
| | S (data) | GPIO 4 (D4) | |
| Relay module (keyestudio, 1-channel) | G | GND | |
| | V | 5V | |
| | S (signal) | GPIO 26 (D26) | **active-HIGH**, bench-confirmed |
| Relay terminals | COM | 5V | motor supply = the board's 5V pin |
| | NO | motor lead A | NC unused |
| Motor + fan blade | lead B | GND | leads are not polarized |
| 1N4007 flyback diode | across the motor | | stripe (cathode) toward lead A (the NO side) |
| RGB LED (common cathode) | R / G / B | GPIO 16 / 17 / 18 | 220 ohm each |
| | common (long leg) | GND | |

Picture: `Relay_Module/Fan_Wiring.svg`.

## 4. Schema (target Modbus register map for Phase B)
| Type | Address | Name | Notes |
|---|---|---|---|
| Input Register | IR0 | TEMP_C | tenths of a degree C |
| Input Register | IR1 | HUMIDITY_PCT | whole % |
| Input Register | IR2 | HEARTBEAT | +1 per second |
| Input Register | IR3 | RECONNECT_COUNT | WiFi rejoins since boot |
| Discrete Input | DI0 | COMPRESSOR_RUNNING | relay state |
| Discrete Input | DI1 | HVAC_FAULT | latched |
| Holding Register | HR0 | TARGET_TEMP_C | tenths of a degree C, master-written |
| Coil | Coil0 | HVAC_ENABLE | master-written, default set |

OpenPLC counts: DI 2, coils 1, IR 4, HR 1. Static IP 192.168.50.21, OpenPLC container 192.168.50.121.

## 5. Troubleshooting (lessons from this build)
- **`DHT.h: No such file or directory`:** the Adafruit "DHT sensor library" (and "Adafruit Unified Sensor") are not installed. Install both from the Library Manager.
- **Relay/fan behaves in reverse:** the keyestudio relay module is active-HIGH, the opposite of the common assumption. `RELAY_ACTIVE_LOW = false`. A polarity result belongs to the physical board it was tested on.
- **DHT11 instead of the DHT22 in the design docs:** the part in hand was a DHT11. Wiring is identical; whole-degree resolution, +/-2 C, 0-50 C. Firmware picks the model with the `WOKWI_SIM` define (Wokwi has only the DHT22).
- **Cooling never starts in the simulation:** it needs the temperature at least 0.5 C above the setpoint. Raising the setpoint (instead of lowering it) correctly never starts cooling.
- **Brushed DC motor on a relay:** the flyback diode is required (stripe toward the positive/NO side) to absorb the opening spike. If the board resets whenever the motor switches, move the motor to its own supply (2-3 AA cells, grounds joined).
- **Do not power the motor from the board's DC jack.**

## 6. Simulation
`Firmware/FH-HVAC/Wokwi/` (diagram, sketch, README). All parts native except the fan: Wokwi has no DC motor,
diode or battery part, so a cyan LED on the relay's NO/COM stands in for the fan motor. Sim-only differences:
DHT22 instead of DHT11, Serial `f` injects a failed read, no flyback diode / supply. Wokwi cannot show a real
`HVAC_FAULT` (it never fails a read), so the real fault path is proven by unplugging the sensor on the bench.
Phase B is not simulated: a simulated ESP32 cannot be reached by the hub's OpenPLC container.
