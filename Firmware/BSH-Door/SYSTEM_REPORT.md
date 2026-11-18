# BSH-Door -- System Report

**Status: Phase B bench-confirmed (Oct 9, 2026) at 192.168.50.13.** Phase A was reported fine by Luis the same day; no per-component Phase A results were itemized, so the Phase B tests below also served as the hardware check. This report was first written as a draft on Oct 9, 2026 and finalized the same day with the Phase B results. Phase C (OpenPLC + SCADA) has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet).

Produced per `Firmware/WORKFLOW.md` Section 4.

---

## 1. How the system works

BSH-Door is the gate and access-control node of building BSH. An RFID reader decides
locally who may open the gate; an IR sensor and a reed switch independently
report approach and the gate's actual position; an SG90 servo moves the gate
arm; an RGB LED shows the local state.

The node reports three honest, independent values and deliberately does not
judge "intrusion" itself:

- **IR_DETECTED** -- something is near the gate.
- **DOOR_POSITION** -- the reed switch says the gate is open or closed,
  independent of what the servo was told to do.
- **ACCESS_GRANTED** -- a card on the authorized list was accepted.

Correlating them (for example approach with no matching access, or the reed
reporting open when no card was granted) is the SCADA layer's job, per
`Piranga_Workflow_v2.md` Section 7, not something built into this ESP32.

**Local logic (Phase B sketch):** the gate is a non-blocking state machine
(closed, opening, hold, closing) so `mb.task()` runs on every pass of the loop.
An authorized card raises ACCESS_GRANTED for about 2 s and starts the gate:
servo 0 to 90 degrees in 5-degree steps every 25 ms, hold 2 s, then back to 0.
An unauthorized card flashes the LED red for 1 s and the servo does not move.
`PICC_HaltA()` runs after every read so one tap is one decision. The safe
default is closed: green LED, servo at 0 and detached.

**Remote commands (momentary):** a write of 1 to DOOR_OPEN_CMD opens the gate
with no card and leaves ACCESS_GRANTED false; a write of 1 to DOOR_CLOSE_CMD
sends an opening or holding gate back to closed. The node clears the coil
itself after consuming the write.

**Fail-safe:** if the WiFi link stays down longer than 5 s the gate is sent to
closed and any pending remote command is dropped. The local card reader keeps
working the whole time. This watches the WiFi link only, not a master that goes
silent while WiFi stays up.

**Connection monitoring:** HEARTBEAT counts up once a second and
RECONNECT_COUNT counts WiFi rejoins, so SCADA can tell a frozen node from a
healthy one.

## 2. The code

Phase B sketch of record (source: `BSH-Door/BSH_Door_Node/BSH_Door_Node.ino`),
bench-confirmed Oct 9, 2026. The Phase A standalone sketch it grew from is kept
in `Archive/Door_PhaseA_sketches/`.

```cpp
/*
  ============================================================================
  BSH-Door Node -- Phase B: WiFi + Modbus TCP slave
  ============================================================================
  STATUS (2026-10-09): Phase B bench-confirmed by Luis at 192.168.50.13 (remote
  open/close coils, card badge, ACCESS_GRANTED pulse, IR/reed, heartbeat/reconnect,
  WiFi watchdog). Results in ESP32_Core_Modbus/Build_Log.md.

  Architecture: plain Modbus TCP SLAVE at 192.168.50.13:502. OpenPLC-BSH-Door
  (192.168.50.113) will poll it directly in Phase C. No bridge.

  Modbus register map (per BSH_Systems/03_BSH-Door.md):
    DI0    IR_DETECTED      approach/presence (raw sensor, no judgement)
    DI1    DOOR_POSITION    1 = open, 0 = closed (from the reed switch)
    DI2    ACCESS_GRANTED   pulsed ~2 s after a valid card read
    Coil0  DOOR_OPEN_CMD    remote open (momentary: the node clears it)
    Coil1  DOOR_CLOSE_CMD   remote close (momentary: the node clears it)
    IR0    HEARTBEAT        +1 per second, wraps at 65535
    IR1    RECONNECT_COUNT  WiFi rejoins since boot; 0 after a reboot

  The three DIs are independent and honest. A card open raises ACCESS_GRANTED;
  a remote DOOR_OPEN_CMD does not. SCADA correlates them -- this node never
  decides what is an intrusion.

  Hardware (same pin map as Phase A, both buildings):
    GPIO 13 servo signal (servo on external 5V)
    GPIO 25/26/27 RGB LED R/G/B (green = closed, red = open/moving/denied)
    GPIO 14 IR obstacle sensor
    GPIO 34 reed switch module (input-only, no internal pull-up)
    GPIO 18/19/23 SPI SCK/MISO/MOSI, GPIO 32 RFID SS, GPIO 33 RFID RST
    RC522 is 3.3V ONLY.

  Gate logic is a non-blocking state machine (no delay()), so mb.task() runs
  every loop.  Open: servo 0 -> 90 in 5 degree steps every 25 ms, hold 2 s,
  then back to 0.  A close command during opening/hold sends it back to 0.

  FAIL-SAFE: if WiFi stays down longer than WATCHDOG_MS the gate is sent to
  closed and any pending remote command is dropped (a closed gate is the safe
  default). The local card reader keeps working the whole time. This watches
  the WiFi LINK, not a silent master.

  Libraries: ModbusIP_ESP8266, MFRC522, ESP32Servo. Board: ESP32 Dev Module.
  ASCII only in this file (no em dashes) -- Windows Arduino IDE rule.
  ============================================================================
*/

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ---------------------------------------------------------------------------
// WiFi + static IP (must match Piranga_Workflow_v2.md Section 6)
// ---------------------------------------------------------------------------
const char* ssid     = "SCT5";
const char* password = "YOUR_WIFI_PASSWORD";  // placeholder, set locally

IPAddress ip(192, 168, 50, 13);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Modbus addresses
// ---------------------------------------------------------------------------
const int DI_IR_DETECTED     = 0;
const int DI_DOOR_POSITION   = 1;
const int DI_ACCESS_GRANTED  = 2;
const int COIL_DOOR_OPEN_CMD  = 0;
const int COIL_DOOR_CLOSE_CMD = 1;
const int IREG_HEARTBEAT       = 0;
const int IREG_RECONNECT_COUNT = 1;

// ---------------------------------------------------------------------------
// Pins (project pin map, identical for BSH and FH)
// ---------------------------------------------------------------------------
#define PIN_SERVO      13
#define PIN_RGB_RED    25
#define PIN_RGB_GREEN  26
#define PIN_RGB_BLUE   27   // unused, wired for BOM consistency
#define PIN_IR_SENSOR  14
#define PIN_REED       34   // external pull-up to 3.3V required (module or 10k)
#define PIN_RFID_SS    32
#define PIN_RFID_RST   33
#define PIN_SPI_SCK    18
#define PIN_SPI_MISO   19
#define PIN_SPI_MOSI   23

// Polarity flags -- still at their defaults, verify on the bench from the raw
// Serial values (WORKFLOW Phase A open items).
const bool REED_LOW_MEANS_CLOSED = true;
const bool IR_ACTIVE_LOW = true;

// ---------------------------------------------------------------------------
// Authorized cards (this building's list)
// ---------------------------------------------------------------------------
byte AUTHORIZED_CARDS[][4] = {
  {0xB3, 0x12, 0x87, 0x2D}   // BSH
};
const int NUM_AUTHORIZED_CARDS = 1;

// ---------------------------------------------------------------------------
// Timing and servo
// ---------------------------------------------------------------------------
const int  GATE_CLOSED_ANGLE = 0;
const int  GATE_OPEN_ANGLE   = 90;
const int  GATE_STEP_DEG     = 5;
const unsigned long GATE_STEP_MS   = 25;
const unsigned long GATE_HOLD_MS   = 2000;
const unsigned long ACCESS_PULSE_MS = 2000;
const unsigned long DENIED_FLASH_MS = 1000;
const unsigned long WATCHDOG_MS     = 5000;
const unsigned long CARD_POLL_MS    = 50;
const unsigned long PRINT_INTERVAL_MS = 500;

enum GateState { GATE_CLOSED, GATE_OPENING, GATE_HOLD, GATE_CLOSING };
GateState gateState = GATE_CLOSED;
int gateAngle = GATE_CLOSED_ANGLE;
unsigned long lastStepTime = 0;
unsigned long holdStart = 0;

MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST);
Servo servo;
ModbusIP mb;

// Set from Modbus callbacks (run inside mb.task()), consumed in loop().
volatile bool openCmdPending  = false;
volatile bool closeCmdPending = false;

bool accessGrantedPulse = false;
unsigned long accessPulseStart = 0;
bool deniedFlash = false;
unsigned long deniedStart = 0;

uint16_t heartbeat = 0;
uint16_t reconnectCount = 0;
unsigned long lastHeartbeatTick = 0;
unsigned long wifiDownSince = 0;
bool wifiWasDown = false;
bool failSafeApplied = false;
unsigned long lastReconnectTry = 0;
unsigned long lastPrint = 0;
unsigned long lastCardPoll = 0;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
bool isAuthorized(byte *uid, byte size) {
  if (size != 4) return false;
  for (int i = 0; i < NUM_AUTHORIZED_CARDS; i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (uid[j] != AUTHORIZED_CARDS[i][j]) { match = false; break; }
    }
    if (match) return true;
  }
  return false;
}

bool readDoorPositionOpen() {
  bool raw = digitalRead(PIN_REED);
  return REED_LOW_MEANS_CLOSED ? (raw == HIGH) : (raw == LOW);
}

bool readIrDetected() {
  bool raw = digitalRead(PIN_IR_SENSOR);
  return IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);
}

void showClosed() {
  digitalWrite(PIN_RGB_GREEN, HIGH);
  digitalWrite(PIN_RGB_RED, LOW);
}

void showOpen() {
  digitalWrite(PIN_RGB_RED, HIGH);
  digitalWrite(PIN_RGB_GREEN, LOW);
}

// Modbus write callbacks. Momentary: only a write of 1 counts.
uint16_t onOpenCmdWrite(TRegister* reg, uint16_t val) {
  if (val != 0) openCmdPending = true;
  return val;
}
uint16_t onCloseCmdWrite(TRegister* reg, uint16_t val) {
  if (val != 0) closeCmdPending = true;
  return val;
}

void startOpening() {
  if (!servo.attached()) servo.attach(PIN_SERVO);
  gateState = GATE_OPENING;
  lastStepTime = millis();
}

void startClosing() {
  if (!servo.attached()) servo.attach(PIN_SERVO);
  gateState = GATE_CLOSING;
  lastStepTime = millis();
}

// Advance the gate one non-blocking step at a time.
void updateGate(unsigned long now) {
  switch (gateState) {
    case GATE_CLOSED:
      break;
    case GATE_OPENING:
      if (now - lastStepTime >= GATE_STEP_MS) {
        lastStepTime = now;
        gateAngle += GATE_STEP_DEG;
        if (gateAngle >= GATE_OPEN_ANGLE) {
          gateAngle = GATE_OPEN_ANGLE;
          gateState = GATE_HOLD;
          holdStart = now;
        }
        servo.write(gateAngle);
      }
      break;
    case GATE_HOLD:
      if (now - holdStart >= GATE_HOLD_MS) {
        startClosing();
      }
      break;
    case GATE_CLOSING:
      if (now - lastStepTime >= GATE_STEP_MS) {
        lastStepTime = now;
        gateAngle -= GATE_STEP_DEG;
        if (gateAngle <= GATE_CLOSED_ANGLE) {
          gateAngle = GATE_CLOSED_ANGLE;
          servo.write(gateAngle);
          servo.detach();
          gateState = GATE_CLOSED;
        } else {
          servo.write(gateAngle);
        }
      }
      break;
  }
}

void updateLed(unsigned long now) {
  if (gateState == GATE_CLOSED && !deniedFlash) showClosed();
  else showOpen();
}

const char* gateName() {
  switch (gateState) {
    case GATE_CLOSED:  return "closed";
    case GATE_OPENING: return "opening";
    case GATE_HOLD:    return "hold";
    default:           return "closing";
  }
}

// Non-blocking card poll. Only acts while the gate is closed and idle.
void pollCard(unsigned long now) {
  if (now - lastCardPoll < CARD_POLL_MS) return;
  lastCardPoll = now;
  if (gateState != GATE_CLOSED || deniedFlash) return;
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" ");
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (isAuthorized(mfrc522.uid.uidByte, mfrc522.uid.size)) {
    Serial.println(F("Access granted"));
    accessGrantedPulse = true;
    accessPulseStart = now;
    startOpening();
  } else {
    Serial.println(F("Access denied"));
    deniedFlash = true;
    deniedStart = now;
  }
  mfrc522.PICC_HaltA();
}

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== BSH-Door Node (Phase B: WiFi + Modbus) ===");

  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_SS);
  mfrc522.PCD_Init();

  servo.attach(PIN_SERVO);
  servo.write(GATE_CLOSED_ANGLE);
  delay(500);
  servo.detach();

  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  pinMode(PIN_REED, INPUT);        // NOT INPUT_PULLUP on GPIO 34
  pinMode(PIN_IR_SENSOR, INPUT);
  showClosed();

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected. IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED - check SSID/password and AP range.");
    Serial.println("Card reader still works locally; Modbus will not until WiFi joins.");
  }

  mb.server();
  mb.addIsts(DI_IR_DETECTED, false);
  mb.addIsts(DI_DOOR_POSITION, false);
  mb.addIsts(DI_ACCESS_GRANTED, false);
  mb.addCoil(COIL_DOOR_OPEN_CMD, false);
  mb.addCoil(COIL_DOOR_CLOSE_CMD, false);
  mb.addIreg(IREG_HEARTBEAT, 0);
  mb.addIreg(IREG_RECONNECT_COUNT, 0);
  mb.onSetCoil(COIL_DOOR_OPEN_CMD, onOpenCmdWrite);
  mb.onSetCoil(COIL_DOOR_CLOSE_CMD, onCloseCmdWrite);

  Serial.println("Ready. Gate boots closed. Badge a card...");
}

void loop() {
  mb.task();  // always first, every iteration
  unsigned long now = millis();

  // --- WiFi link watchdog + reconnect ---
  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  if (!wifiUp) {
    if (!wifiWasDown) {
      wifiWasDown = true;
      wifiDownSince = now;
      failSafeApplied = false;
      Serial.println("WiFi link lost.");
    }
    if (!failSafeApplied && (now - wifiDownSince) >= WATCHDOG_MS) {
      failSafeApplied = true;
      openCmdPending = false;
      closeCmdPending = false;
      if (gateState != GATE_CLOSED) startClosing();
      Serial.println("WATCHDOG: comm lost, gate sent to closed.");
    }
    if (now - lastReconnectTry >= 5000) {
      lastReconnectTry = now;
      WiFi.reconnect();
    }
  } else if (wifiWasDown) {
    wifiWasDown = false;
    Serial.println("WiFi link restored.");
    reconnectCount++;
    mb.Ireg(IREG_RECONNECT_COUNT, reconnectCount);
    // Drop stale command state so a held coil cannot silently re-arm.
    mb.Coil(COIL_DOOR_OPEN_CMD, false);
    mb.Coil(COIL_DOOR_CLOSE_CMD, false);
    openCmdPending = false;
    closeCmdPending = false;
  }

  // --- remote commands (momentary: consume the write, clear the coil) ---
  if (closeCmdPending) {
    closeCmdPending = false;
    mb.Coil(COIL_DOOR_CLOSE_CMD, false);
    if (gateState == GATE_OPENING || gateState == GATE_HOLD) {
      Serial.println("Hub wrote DOOR_CLOSE_CMD -> closing");
      startClosing();
    }
  }
  if (openCmdPending) {
    openCmdPending = false;
    mb.Coil(COIL_DOOR_OPEN_CMD, false);
    if (gateState == GATE_CLOSED || gateState == GATE_CLOSING) {
      Serial.println("Hub wrote DOOR_OPEN_CMD -> opening (no card)");
      startOpening();
    } else if (gateState == GATE_HOLD) {
      holdStart = now;  // extend the hold
    }
  }

  // --- local card reader ---
  pollCard(now);

  // --- gate motion, LED, pulses ---
  updateGate(now);
  if (deniedFlash && (now - deniedStart) >= DENIED_FLASH_MS) deniedFlash = false;
  if (accessGrantedPulse && (now - accessPulseStart) >= ACCESS_PULSE_MS) accessGrantedPulse = false;
  updateLed(now);

  // --- publish the three honest status bits ---
  mb.Ists(DI_IR_DETECTED, readIrDetected());
  mb.Ists(DI_DOOR_POSITION, readDoorPositionOpen());
  mb.Ists(DI_ACCESS_GRANTED, accessGrantedPulse);

  // Heartbeat: +1 every second, non-blocking
  if (now - lastHeartbeatTick >= 1000) {
    lastHeartbeatTick = now;
    heartbeat++;
    mb.Ireg(IREG_HEARTBEAT, heartbeat);
  }

  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    Serial.print("gate=");
    Serial.print(gateName());
    Serial.print("  DoorPositionOpen=");
    Serial.print(readDoorPositionOpen());
    Serial.print(" (raw ");
    Serial.print(digitalRead(PIN_REED));
    Serial.print(")  IRDetected=");
    Serial.print(readIrDetected());
    Serial.print(" (raw ");
    Serial.print(digitalRead(PIN_IR_SENSOR));
    Serial.print(")  access=");
    Serial.print(accessGrantedPulse);
    Serial.print("  hb=");
    Serial.print(heartbeat);
    Serial.print(" rc=");
    Serial.print(reconnectCount);
    Serial.print("  wifi=");
    Serial.println(wifiUp ? "up" : "DOWN");
  }
}
```

## 3. The wiring

| Component | Signal | ESP32 pin | Power / notes |
|---|---|---|---|
| RC522 RFID reader | SCK | GPIO 18 | VSPI default. **VCC to 3.3V only, never 5V** |
| | MISO | GPIO 19 | |
| | MOSI | GPIO 23 | |
| | SDA / SS | GPIO 32 | Chip select |
| | RST | GPIO 33 | |
| SG90 servo | Signal | GPIO 13 | **External 5V supply**, common ground with the ESP32 |
| RGB LED (common cathode) | Red | GPIO 25 | 220 ohm in series |
| | Green | GPIO 26 | 220 ohm in series |
| | Blue | GPIO 27 | 220 ohm in series; wired for BOM consistency, never turned on |
| IR obstacle sensor | Signal | GPIO 14 | VCC 3.3V. Polarity set by `IR_ACTIVE_LOW`; confirmed by Luis on the bench |
| Reed switch module (S/V/G) | S | GPIO 34 | Input-only pin, no internal pull-up. Pull-up confirmed working on the bench by Luis (no extra resistor reported needed) |

None of these is a strapping pin (GPIO 0, 2, 5, 12, 15). The IR sketch comment
names GPIO 4 as a fallback if a board has no GPIO 14; that would be a pin-map
change and has to be logged before it is used. There is no buzzer on this node.

Per-component detail: `RFID_RC522_Reader/`, `SG90_Servo/`, `RGB_LED/`,
`IR_Sensor/`, `Reed_Switch/` (each has `Wiring.md` and `Build_Log.md`).

## 4. Schema (Modbus register map, implemented and bench-confirmed)

| Type | Address | Name | Meaning |
|---|---|---|---|
| Discrete Input | DI0 | IR_DETECTED | Approach/presence detected |
| Discrete Input | DI1 | DOOR_POSITION | Actual open (1) / closed (0), from the reed switch |
| Discrete Input | DI2 | ACCESS_GRANTED | Valid card accepted locally (pulsed about 2 s) |
| Coil | Coil0 | DOOR_OPEN_CMD | Remote open command (momentary) |
| Coil | Coil1 | DOOR_CLOSE_CMD | Remote close command (momentary) |
| Input Register | IR0 | HEARTBEAT | Counts up once a second, wraps at 65535 |
| Input Register | IR1 | RECONNECT_COUNT | WiFi rejoins since boot |

IP 192.168.50.13. Dedicated PLC: `OpenPLC-BSH-Door` at 192.168.50.113. OpenPLC counts: DI 3, coil 2,
Input Register 2.

Design points that hold in the sketch: DOOR_POSITION comes from the reed switch,
never from the servo command; IR_DETECTED goes through `readIrDetected()` so the
polarity flag applies; DOOR_OPEN_CMD with no card opens the gate and leaves
ACCESS_GRANTED false. That contrast is the authorized-versus-injected-access
scenario.

## 5. Troubleshooting / Lessons Learned

### Found in the Oct 9, 2026 code review (not bench findings)

The first drafts of the Door sketches were adapted from a kit tutorial and
differed from the project pin map. All were corrected in the current sketch:

- **Pin conflicts.** A single LED on GPIO 19 and a buzzer on GPIO 18 pushed the
  RFID SCK/MISO to GPIO 16/17 (FH draft). The fix was to move the indicator to
  the planned RGB LED on 25/26 and return the RFID to 18/19/23. GPIO 16/17 would
  also have failed on a WROVER module (PSRAM pins).
- **IR on GPIO 27** collided with the RGB LED's blue channel; it is back on 14.
- **Wrong RFID module type** in the BSH draft (I2C module, `MFRC522_I2C`). The
  project uses the SPI RC522 with the standard `MFRC522` library.
- **Card compare by text** dropped leading zeros (a byte below 0x10 printed as
  one digit), so different cards could match. Replaced by a byte-array compare.
- **No `PICC_HaltA()`**, so a held card was re-read instantly. Added.
- **File naming:** sketches must be `<Bldg>_Door_Node.ino` in their node folder,
  with no `_PhaseA` suffix.

### Hardware cautions (from the component docs)

- RC522 is a 3.3V part. Check VCC before powering on.
- GPIO 34-39 have no internal pull resistors. `INPUT_PULLUP` there compiles and
  does nothing; a pull-up must be real.
- The SG90 needs an external 5V supply or it can brown out the ESP32.
- Never judge the IR or reed polarity from the simulation; read the real raw values.

### Phase B bench results (Oct 9, 2026, confirmed by Luis)

| Test | Result |
|---|---|
| Serial shows `Connected. IP address: 192.168.50.13` | Pass |
| Modbus TCP port 502 reachable from the laptop | Pass |
| HEARTBEAT counts about 1 per second, RECONNECT_COUNT 0 after boot | Pass |
| DOOR_OPEN_CMD opens the gate with no card, ACCESS_GRANTED stays 0 | Pass (the vulnerability this node demonstrates) |
| DOOR_CLOSE_CMD closes an opening or holding gate early | Pass |
| Card badge opens the gate and pulses ACCESS_GRANTED for about 2 s | Pass (card `B3 12 87 2D` accepted) |
| IR_DETECTED and DOOR_POSITION track their sensors | Pass |
| WiFi cut: watchdog fires, card still opens the gate offline | Pass |
| RECONNECT_COUNT reads 1 after recovery, both counters 0 after reset | Pass |

The laptop capture shows port 502 open, HEARTBEAT counting (99, then 127 to 186 across the run), RECONNECT_COUNT 0, and ACCESS_GRANTED reading 1 after a badge while it read 0 during remote opens. The rest is confirmed by Luis directly.

### Known limits and quirks

- The gate holds open only 2 s and closes by itself, so the 5 s watchdog cannot
  be seen closing an open gate. The offline card test is the meaningful check.
- The watchdog watches the WiFi link only, not a master that stops polling
  while WiFi stays up (SCADA watches HEARTBEAT instead).
- If WiFi fails to connect at boot and joins later, RECONNECT_COUNT reads 1
  even though the board never dropped a connection it had. Read it with the
  heartbeat when judging a "drop".
- `GATE_OPEN_ANGLE` (90 degrees) has not been checked against the physical gate
  arm in a logged test.
- Per-component standalone bench results were not itemized in the component
  `Build_Log.md` files; Luis reported the combined behavior fine.

## 6. Simulation

`Wokwi/` (built Oct 9, 2026 and run by Luis the same day, looked fine). Parts: `board-mfrc522` (native, six preset cards), `wokwi-servo` (native), `wokwi-rgb-led` with 3 x 220 ohm (native), plus two stand-ins: a slide switch for the IR sensor (LEFT = object, pin LOW) and a slide switch with a 10k pull-up for the reed module (LEFT = magnet near = closed, pin LOW). The sketch swaps the real card for Wokwi's Blue card (01:02:03:04) behind `WOKWI_SIM`; everything else is identical to the bench sketch.

The simulation cannot show: the RC522's real range and antenna placement, IR polarity and sensitivity, the reed module's pull-up and contact bounce, servo current draw (the real servo needs an external 5V supply), or voltage levels. The bench was tested on Oct 9, 2026 (Phase B); no differences from the simulation were reported.
