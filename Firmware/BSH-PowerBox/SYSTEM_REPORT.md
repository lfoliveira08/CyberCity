# BSH-PowerBox -- System Report

**Status: Phase A complete** (2026-10-05) -- both relay channels and both local
override buttons bench-confirmed together in the standalone integration sketch.
**Phase B (WiFi + Modbus TCP) bench-confirmed 2026-10-06** at 192.168.50.14: the
Phase B sketch sits at `BSH_PowerBox_Node/BSH_PowerBox_Node.ino` (see
`ESP32_Core_Modbus/Build_Log.md`). Phase C has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

Produced per `Firmware/WORKFLOW.md` Section 4.

---

## 1. How the system works

BSH-PowerBox switches building lighting and a fan. Each channel is one
dedicated 1-channel relay module (split from a shared 2-channel module on
Sep 30, 2026) driving its own load, plus one local override push button, like
a real room switch. Both channels boot OFF -- a dark, idle room is this node's
safe default.

Two input sources act on each channel: the local button and (from Phase B on)
the hub's Modbus coil. Whichever acted most recently wins. A button press
takes effect immediately; the hub regains control only on a fresh coil write,
not by the coil still holding its old value. Status always reports the real
physical state, so SCADA can see when a button has made reality differ from
the last command.

In Phase A there is no hub, so the Serial commands `l` and `f` stand in for
coil writes and both sources flip the same variable.

## 2. The code

Phase A sketch as confirmed on the bench (source: the combined test, with
relay polarity set to the bench result):

```cpp
/*
  ============================================================================
  BSH-PowerBox -- Phase A Step 4: Combined Standalone Integration Sketch
  ============================================================================
  Purpose: both relay channels + both buttons together, no WiFi, no Modbus.
  This is the real Phase A "everything at once" test before Phase B adds
  networking. Once this behaves correctly, Phase B swaps Serial commands
  for real Modbus coils (that's BSH_PowerBox_Node.ino, already written).

  Wiring: same as before --
    GPIO 13 -> Lights relay IN (3 LEDs in parallel on the switched side)
    GPIO 15 -> Fan relay IN
    GPIO 4  -> Lights override button (INPUT_PULLUP, other leg to GND)
    GPIO 16 -> Fan override button (INPUT_PULLUP, other leg to GND)

  BEFORE FLASHING THIS: set RELAY_LIGHTS_ACTIVE_LOW / RELAY_FAN_ACTIVE_LOW
  below based on what Phase A Steps 1 and 2 actually found on your bench --
  don't assume both channels match each other, and don't carry over the
  Wokwi sim's active-HIGH finding (that was simulator-only).

  Controls (Serial Monitor, 115200 baud):
    l / L -- toggle lights (stands in for a future Modbus coil write)
    f / F -- toggle fan (same)
  Or press the physical buttons -- same effect, models the local override
  switch. In this standalone sketch there's no "hub" to conflict with, so
  both input sources just flip the same variable directly (the more
  careful "hub regains authority only on a fresh coil write" logic lives
  in BSH_PowerBox_Node.ino once Phase B adds Modbus -- see its header
  comment for the full authority rule).
  ============================================================================
*/

const int PIN_LIGHTS_RELAY  = 13;
const int PIN_FAN_RELAY     = 15;
const int PIN_LIGHTS_BUTTON = 4;
const int PIN_FAN_BUTTON    = 16;

// Set these from what Phase A Steps 1 & 2 actually found on the bench.
const bool RELAY_LIGHTS_ACTIVE_LOW = false;  // bench-confirmed Oct 5, 2026
const bool RELAY_FAN_ACTIVE_LOW    = false;  // bench-confirmed Oct 5, 2026

bool lightsOn = false;
bool fanOn    = false;

void setRelay(int pin, bool on, bool activeLow) {
  bool level = activeLow ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
}

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
      if (stableState == LOW) {
        pressedEdge = true;
      }
    }
  }
  lastReading = reading;
  return pressedEdge;
}

const unsigned long PRINT_INTERVAL_MS = 1000;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-PowerBox -- Phase A Combined Standalone Test ===");

  pinMode(PIN_LIGHTS_RELAY, OUTPUT);
  pinMode(PIN_FAN_RELAY, OUTPUT);
  setRelay(PIN_LIGHTS_RELAY, false, RELAY_LIGHTS_ACTIVE_LOW);
  setRelay(PIN_FAN_RELAY, false, RELAY_FAN_ACTIVE_LOW);

  pinMode(PIN_LIGHTS_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FAN_BUTTON, INPUT_PULLUP);

  Serial.println("Type 'l' + Enter to toggle lights, 'f' + Enter to toggle fan,");
  Serial.println("or press the physical buttons. Both channels boot OFF.");
  Serial.println();
}

void loop() {
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
  }

  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("lights=");
    Serial.print(lightsOn ? "ON" : "off");
    Serial.print("  fan=");
    Serial.println(fanOn ? "ON" : "off");
  }
}
```

The sketch above is the Phase A record only. The current firmware at
`BSH_PowerBox_Node/BSH_PowerBox_Node.ino` is the Phase B version.

## 3. The wiring

| Pin | Function | Notes |
|---|---|---|
| GPIO 13 | Lights relay IN | Moved from GPIO 12 (strapping pin). Active-HIGH on the bench board |
| GPIO 15 | Fan relay IN | Active-HIGH on the bench board. GPIO 15 is also a strapping pin -- no boot problems seen |
| GPIO 4 | Lights override button | INPUT_PULLUP, other leg to GND |
| GPIO 16 | Fan override button | INPUT_PULLUP, other leg to GND |

Each relay board has its own VCC/GND. Per-component detail: `Lights_Relay/Wiring.md`, `Fan_Relay/Wiring.md`.

## 4. Schema (Modbus register map)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Coil | Coil0 | LIGHTS_ON_CMD | Lights on/off command |
| Coil | Coil1 | FAN_CMD | Fan on/off command |
| Discrete Input | DI0 | LIGHTS_STATUS | Actual physical lights state |
| Discrete Input | DI1 | FAN_STATUS | Actual physical fan state |

IP 192.168.50.14. Dedicated PLC: `OpenPLC-BSH-PowerBox` at 192.168.50.114.

## 5. Troubleshooting

- **Relay polarity belongs to the physical board, not the channel.** Both new
  1-channel boards were verified independently as active-HIGH on Oct 5, 2026.
  If a load is on when the log says OFF, flip `RELAY_LIGHTS_ACTIVE_LOW` /
  `RELAY_FAN_ACTIVE_LOW`.
- **No Serial output at all for a button or relay command** is most likely the
  wrong sketch flashed (an old lights-only sketch did exactly this on Oct 5),
  not a hardware fault.
- **A custom struct passed by reference to a function can break Arduino's
  auto-prototype generation** ("not declared in this scope"). The button
  debounce uses plain per-button variables for this reason.
- **Do not trust a partial fix.** The GPIO 12 to 13 move did not cure the
  Wokwi inversion; the cause was the simulator's relay polarity. Check the
  relay's own onboard LED, not only the external load.
- **Check the channel you assume is fine.** Fan was inverted in Wokwi the
  whole time and unnoticed until compared against lights.

Full detail: `Lights_Relay/Build_Log.md`, `Fan_Relay/Build_Log.md`.

## 6. Simulation

`Wokwi/` (built Sep 28-29, 2026). Stand-ins: none, all parts are native
(`wokwi-relay-module`, `wokwi-led`, `wokwi-pushbutton`). The simulation got
relay polarity wrong versus the bench for the first board (Wokwi energizes on
HIGH; the real boards happened to also be active-HIGH, but verified
separately, not carried over). Wokwi does not model real relay-module pull-ups
or load current.
