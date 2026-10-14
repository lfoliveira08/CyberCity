# BSH-Elevator — System Report

**Status: Phase A complete** (standalone hardware integration test passing).
**Phase B (WiFi + Modbus TCP) bench-confirmed 2026-10-07** at 192.168.50.11 (see
`ESP32_Core_Modbus/Build_Log.md`; the sketch in `BSH_Elevator_Node/` is now the
Phase B version, and this report keeps the Phase A record). Phase C
(OpenPLC/SCADA) has not started (update Oct 14, 2026: Phase C started Oct 13; all 8 OpenPLC containers are running, none configured yet) — see `Firmware/WORKFLOW.md`.

Produced per the process in `Firmware/WORKFLOW.md` Section 4: written once
a system's Phase A is done, before moving on to the next system (BSH-Door,
per the build order).

---

## 1. How the system works

BSH-Elevator represents a 3-floor elevator cab in the BSH building — one of
eight node builds in CyberUIW (4 systems per building, BSH and FH; this count was 10 when this report was written, before Backup Power was dropped on Oct 5, 2026).
Three floor-call buttons let a person locally request the cab; a DC motor
(via an L293D driver) moves it; an OLED shows live status text.

**State machine** (`ElevatorState`): `ELEV_IDLE` → `ELEV_DOOR_CLOSING` →
`ELEV_MOVING` → `ELEV_ARRIVED` → back to `ELEV_IDLE`. All timing is
`millis()`-based, non-blocking — pressing a button never freezes the loop,
matching the same pattern as `BSH_FireAlarm_Node.ino`. New floor requests
are ignored while a request is already in progress (mirrors the Wokwi
sim's behavior).

Once Phase B (WiFi/Modbus) is added, a remote `CALL_FLOOR_1/2/3` coil write
will call the same `requestFloor()` entry point a local button press does,
`ELEVATOR_DISABLE` will gate both local and remote requests, and
`ELEVATOR_MOVING`/`ELEVATOR_FAULT` discrete inputs plus a `CURRENT_FLOOR`
input register will expose live state to the hub. None of that Modbus layer
exists in the current Phase A firmware — see Section 4 for the target
schema.

**Known simplification, not yet addressed:** `FLOOR_TRAVEL_MS` is a single
fixed duration applied to *every* floor request, regardless of how many
floors away the target is — moving from Floor 1 to Floor 3 currently takes
the same time as Floor 1 to Floor 2. This matches the Wokwi sim's behavior
and hasn't caused a problem on the bench (all three floors are adjacent
enough that it isn't visually obvious), but it's a known gap between "time
elapsed" and "actual distance traveled" worth flagging for anyone tuning
timing later.

## 2. The code

Current firmware
(`Firmware/BSH-Elevator/BSH_Elevator_Node/BSH_Elevator_Node.ino`) is the
Phase A standalone integration test sketch — no WiFi, no Modbus. It is
standing in as this node's firmware for now; the WiFi+Modbus version will
replace it when Phase B starts (see `Firmware/WORKFLOW.md` Section 5). Full
sketch, as confirmed working on the bench:

```cpp
/*
 * BSH-Elevator -- Real Bench Combined Sketch (Phase A)
 * ESP32 + 3 floor buttons + I2C SSD1306 OLED + L293D motor driver + optional debug LEDs
 *
 * STATUS (2026-09-25): Phase A hardware bring-up confirmed working end to
 * end on the real bench -- all 3 floor buttons, the OLED status display,
 * and the L293D/motor (both directions) all work together in this one
 * non-blocking state machine. This is currently standing in as the node's
 * firmware; the WiFi+Modbus version will replace it when Phase B starts
 * (see Firmware/WORKFLOW.md Section 5). Full writeup:
 * Firmware/BSH-Elevator/SYSTEM_REPORT.md
 *
 * Pinout (matches each component's Wiring.md under Firmware/BSH-Elevator and Wiring_Diagram.svg):
 *   Floor 1 button -> GPIO 4   (INPUT_PULLUP)
 *   Floor 2 button -> GPIO 16  (INPUT_PULLUP)
 *   Floor 3 button -> GPIO 17  (INPUT_PULLUP)
 *   OLED SDA       -> GPIO 21
 *   OLED SCL       -> GPIO 22
 *   L293D ENABLE   -> GPIO 18  (PWM, motor speed)
 *   L293D DIRA     -> GPIO 19
 *   L293D DIRB     -> GPIO 5
 *   Debug LED (opt)-> GPIO 23 (mirrors ENABLE), GPIO 25 (mirrors DIRA), GPIO 27 (mirrors DIRB)
 *
 * Libraries needed (Library Manager): "Adafruit SSD1306" + "Adafruit GFX Library"
 * Assumes a 128x64 SSD1306 at I2C address 0x3C -- the most common size/address combo.
 * If yours is 128x32, change SCREEN_HEIGHT below to 32. If the screen stays blank,
 * try address 0x3D instead of 0x3C.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Pins ----------
const int BTN_FLOOR[3]   = {4, 16, 17};
const int PIN_ENABLE      = 18;
const int PIN_DIRA        = 19;
const int PIN_DIRB        = 5;
const int PIN_DBG_ENABLE  = 23; // mirrors PIN_ENABLE
const int PIN_DBG_DIRA    = 25; // mirrors PIN_DIRA
const int PIN_DBG_DIRB    = 27; // mirrors PIN_DIRB

// ---------- Timing / tuning (starting values -- same as the working Wokwi sim) ----------
const unsigned long DOOR_DELAY_MS          = 3000;
const unsigned long FLOOR_TRAVEL_MS        = 6000;  // time to travel one floor -- tune this per the
                                                      // target-time calibration procedure in
                                                      // L293D_Motor_Driver/Wiring.md once the real
                                                      // motor/gearbox load is on the bench
const unsigned long ARRIVED_DISPLAY_MS     = 2000;
const int           MOTOR_SPEED            = 200;    // 0-255 PWM duty -- tune alongside FLOOR_TRAVEL_MS
const int           LEDC_FREQ_HZ           = 1000;
const int           LEDC_RESOLUTION        = 8;       // 8-bit -> 0-255 duty range
const unsigned long FAULT_TIMEOUT_MARGIN_MS = 4000;
const unsigned long DEBOUNCE_MS            = 50;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Debounce state ----------
struct ButtonState {
  bool lastReading;
  bool stable;
  unsigned long lastChangeMs;
};
ButtonState btnState[3];

// ---------- Elevator state machine ----------
enum ElevatorState { ELEV_IDLE, ELEV_DOOR_CLOSING, ELEV_MOVING, ELEV_ARRIVED };
ElevatorState state = ELEV_IDLE;

int currentFloor = 1;       // 1-3, physical starting position -- set the car at floor 1 before power-on
int targetFloor = -1;
unsigned long stateEnteredAt = 0;
bool faulted = false;

// ---------- Helpers ----------
bool readDebounced(int idx) {
  bool raw = (digitalRead(BTN_FLOOR[idx]) == LOW); // pressed = LOW (INPUT_PULLUP)
  unsigned long now = millis();
  if (raw != btnState[idx].lastReading) {
    btnState[idx].lastChangeMs = now;
    btnState[idx].lastReading = raw;
  }
  if ((now - btnState[idx].lastChangeMs) > DEBOUNCE_MS) {
    if (btnState[idx].stable != raw) {
      btnState[idx].stable = raw;
      return raw; // return true only on the debounced press edge
    }
  }
  return false;
}

void lcdPrint(const String &line1, const String &line2) {
  // Text size 1 = 6px/char, so up to ~21 characters fit on a 128px-wide screen
  // without wrapping. Our longest messages (e.g. "Moving to 3") fit comfortably,
  // so both lines stay put instead of wrapping into each other.
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 16);
  display.println(line2);
  display.display();
}

void motorStop() {
  ledcWrite(PIN_ENABLE, 0);
  digitalWrite(PIN_DIRA, LOW);
  digitalWrite(PIN_DIRB, LOW);
  // mirror to debug LEDs
  ledcWrite(PIN_DBG_ENABLE, 0);
  digitalWrite(PIN_DBG_DIRA, LOW);
  digitalWrite(PIN_DBG_DIRB, LOW);
}

void motorRunToward(int target) {
  bool up = (target > currentFloor);
  digitalWrite(PIN_DIRA, up ? HIGH : LOW);
  digitalWrite(PIN_DIRB, up ? LOW : HIGH);
  ledcWrite(PIN_ENABLE, MOTOR_SPEED);
  // mirror to debug LEDs
  digitalWrite(PIN_DBG_DIRA, up ? HIGH : LOW);
  digitalWrite(PIN_DBG_DIRB, up ? LOW : HIGH);
  ledcWrite(PIN_DBG_ENABLE, MOTOR_SPEED);
}

void requestFloor(int floorNum) {
  if (faulted) return;
  if (floorNum == currentFloor && state == ELEV_IDLE) {
    lcdPrint("Floor " + String(floorNum), "Already here");
    return;
  }
  if (state == ELEV_IDLE) {
    targetFloor = floorNum;
    state = ELEV_DOOR_CLOSING;
    stateEnteredAt = millis();
    lcdPrint("Floor " + String(floorNum), "Door closing..");
  }
  // ignore new requests while already moving/door-closing/arrived -- matches Wokwi sim behavior
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(BTN_FLOOR[i], INPUT_PULLUP);
    btnState[i] = {false, false, 0};
  }

  pinMode(PIN_DIRA, OUTPUT);
  pinMode(PIN_DIRB, OUTPUT);
  pinMode(PIN_DBG_DIRA, OUTPUT);
  pinMode(PIN_DBG_DIRB, OUTPUT);

  ledcAttach(PIN_ENABLE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  ledcAttach(PIN_DBG_ENABLE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  motorStop();

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found at 0x3C -- check wiring/address (try 0x3D).");
    while (true) { delay(1000); } // halt so it's obvious the screen didn't init
  }
  lcdPrint("Elevator Ready", "Floor " + String(currentFloor));

  Serial.println("BSH-Elevator bench node ready.");
}

void loop() {
  // --- button polling ---
  for (int i = 0; i < 3; i++) {
    if (readDebounced(i)) {
      requestFloor(i + 1);
    }
  }

  unsigned long now = millis();

  switch (state) {
    case ELEV_IDLE:
      // nothing to do, waiting for a request
      break;

    case ELEV_DOOR_CLOSING:
      if (now - stateEnteredAt >= DOOR_DELAY_MS) {
        state = ELEV_MOVING;
        stateEnteredAt = now;
        motorRunToward(targetFloor);
        lcdPrint("Moving to " + String(targetFloor), (targetFloor > currentFloor) ? "Going UP" : "Going DOWN");
      }
      break;

    case ELEV_MOVING: {
      unsigned long travelSoFar = now - stateEnteredAt;
      bool normalTimeout = travelSoFar >= FLOOR_TRAVEL_MS;

      if (normalTimeout) {
        motorStop();
        currentFloor = targetFloor;
        state = ELEV_ARRIVED;
        stateEnteredAt = now;
        lcdPrint("Arrived Floor " + String(currentFloor), "Door opening..");
      } else if (!faulted && travelSoFar > (FLOOR_TRAVEL_MS + FAULT_TIMEOUT_MARGIN_MS)) {
        // safety net -- should not normally trigger since normalTimeout fires first
        motorStop();
        faulted = true;
        lcdPrint("FAULT", "Motor timeout");
      }
      break;
    }

    case ELEV_ARRIVED:
      if (now - stateEnteredAt >= ARRIVED_DISPLAY_MS) {
        state = ELEV_IDLE;
        lcdPrint("Floor " + String(currentFloor), "Ready");
      }
      break;
  }
}
```

## 3. Wiring (consolidated across all three components)

| Component | Part | Pin | ESP32 pin | Notes |
|---|---|---|---|---|
| Floor 1 button | Momentary push button | Signal leg | GPIO 4 | `INPUT_PULLUP`, active LOW. Other leg -> GND. |
| Floor 2 button | Momentary push button | Signal leg | GPIO 16 | `INPUT_PULLUP`, active LOW. Other leg -> GND. |
| Floor 3 button | Momentary push button | Signal leg | GPIO 17 | `INPUT_PULLUP`, active LOW. Other leg -> GND. |
| Status display | **128x64 I2C SSD1306 OLED** (substituted for the originally-documented 16x2 LCD — see `I2C_LCD_Display/Wiring.md`) | SDA | GPIO 21 | I2C address **0x3C** (0x3D if your board ships at the alternate address) |
| | | SCL | GPIO 22 | |
| | | VCC | 5V (or 3.3V, module-dependent) | |
| | | GND | Common ground | |
| L293D motor driver | L293D dual H-bridge IC | Pin 1 (Enable 1) | GPIO 18 | PWM (LEDC, 1kHz/8-bit) |
| | | Pin 2 (In 1 / DIRA) | GPIO 19 | |
| | | Pin 7 (In 2 / DIRB) | GPIO 5 | |
| | | Pin 16 (+V, logic) | ESP32 5V | |
| | | Pin 8 (+Vmotor) | External 4.5-6V supply (+) | Never from the ESP32's own rail |
| | | Pins 4, 5, 12, 13 (0V) | Common ground | Ties ESP32 GND + external supply GND together |
| | | Pins 9, 10, 15 (Enable 2, In 3, In 4) | Tied to GND | Unused second H-bridge channel |
| | | Pins 11, 14 (Out 3, Out 4) | Unconnected | Unused second H-bridge channel |
| | | Pins 3, 6 (Out 1, Out 2) | Motor's two leads | DG01D-A130 48:1 gearmotor — leads are not polarized, either order works (swap to reverse spin direction) |
| Debug LEDs (optional bench aid) | 3x LED + 220Ω resistor | Mirrors ENABLE | GPIO 23 | Independent pins, not tapped off the L293D lines — mirrored in firmware |
| | | Mirrors DIRA | GPIO 25 | |
| | | Mirrors DIRB | GPIO 27 | |

Full visual reference: `Firmware/BSH-Elevator/Wiring_Diagram.svg`.

## 4. Schema — target Modbus register map

Phase A has no Modbus layer yet (see `Firmware/WORKFLOW.md` Section 5). This
is the register map `BSH_Elevator_Node.ino`'s real WiFi/Modbus version will
expose once Phase B starts:

| Register | Type | Index | Source | Meaning |
|---|---|---|---|---|
| `CALL_FLOOR_1` | Coil | Coil0 | Central hub (write) | Momentary/self-clearing — same entry point as pressing the Floor 1 button locally |
| `CALL_FLOOR_2` | Coil | Coil1 | Central hub (write) | Same, for Floor 2 |
| `CALL_FLOOR_3` | Coil | Coil2 | Central hub (write) | Same, for Floor 3 |
| `ELEVATOR_DISABLE` | Coil | Coil3 | Central hub (write) | While set, ignores both local button presses and remote floor calls |
| `ELEVATOR_MOVING` | Discrete Input | DI0 | Motor state machine | 1 while in `ELEV_DOOR_CLOSING` or `ELEV_MOVING` |
| `ELEVATOR_FAULT` | Discrete Input | DI1 | Motor timeout safety net | 1 once a stall/timeout fault latches — does **not** self-clear (represents "needs a technician"), distinct from `ELEVATOR_DISABLE` which does |
| `CURRENT_FLOOR` | Input Register | IR0 | State machine | 1-3, updates on arrival |

## 5. Troubleshooting / Lessons Learned

Consolidated from each component's `Build_Log.md`.

### Resolved / confirmed during Phase A

- **Push buttons** worked first try — `INPUT_PULLUP` + 50ms software
  debounce on GPIO 4/16/17 needed no external resistors and no rework.
- **OLED text overlap** — `setTextSize(2)` on the status line looked fine
  in isolation but wrapped and collided with the second line for longer
  strings ("Elevator Ready", "Moving to 3"). Fixed by using `setTextSize(1)`
  for both lines (fits up to ~21 chars/line at 128px wide), lines at y=0
  and y=16.
- **Debug LED (green/DIRA) not lighting** — turned out to be a bad/loose
  jumper wire, not a code or GPIO problem; confirmed by the fact the
  identically-coded red (DIRB) LED worked correctly.
- **Motor not spinning at all** — debug LEDs toggling correctly proved the
  code/signal generation was right, but they're wired straight to the ESP32
  and never touch the L293D, so they didn't validate the driver chip's own
  power/wiring. The real-world checklist that unblocks this class of
  problem: common ground between the external motor supply and ESP32/L293D
  GND, chip orientation (pin 1 alignment), voltage measured directly at the
  chip's pin 8 (not just at the battery), and OUT1/OUT2 seating.
- **DG01D-A130 motor leads are not polarized** — either lead can go to
  either L293D output; swap them (or the DIRA/DIRB firmware mapping) to
  reverse spin direction, no damage risk.
- **Stray `*/` inside a header comment's file-path example**
  (`Firmware/BSH-Elevator/*/Wiring.md`) silently closed the C-style block
  comment early, corrupting everything after it into real (broken) code —
  produced a long, confusing cascade of unrelated compiler errors. Avoid
  wildcard-style paths inside `/* ... */` comments.
- **Non-ASCII characters (em dashes) in `.ino` comments** failed to compile
  on this Windows Arduino IDE setup ("extended character is not valid in an
  identifier") — the toolchain wasn't reading the file as UTF-8. Stick to
  plain ASCII in Arduino source comments going forward.
- **ESP32/L293D pin conflict avoided** — the user's specific board briefly
  couldn't use GPIO 5/18/19 (which double as this ESP32's default VSPI
  pins); an alternate mapping (13/14/26) was prepared but ultimately not
  needed — the original 18/19/5 mapping (matching the Elegoo DC-motor
  tutorial being followed) worked once the actual wiring issues above were
  fixed.

### Open — not yet done

- **`MOTOR_SPEED` (200) and `FLOOR_TRAVEL_MS` (6000ms) are still Wokwi-sim
  placeholder values**, not yet calibrated against the real DG01D-A130 +
  cab mechanism. `L293D_Motor_Driver/Wiring.md`'s "Speed / Timing
  Calibration" section has the target-time tuning procedure (measured
  floor spacing: 119.705mm) — still needs to be run on the bench.
- **`FLOOR_TRAVEL_MS` doesn't scale with distance** — see Section 1's
  "Known simplification" note. Every floor request takes the same fixed
  duration regardless of how many floors away the target is.
- **Wokwi simulation still models the 16x2 LCD**, not the OLED actually
  used on the bench — see Section 6. Not yet reconciled.

## 6. Simulation (Phase A0)

`Firmware/BSH-Elevator/Wokwi/` — built and run before any real hardware was
touched, per `Firmware/WORKFLOW.md` Section 7, using `BSH-FireAlarm/Wokwi/`
as the template. Confirmed working by the user on 2026-09-23.

- **Stand-ins used:** L293D + DC motor has no native Wokwi part — modeled
  as 3 LEDs (green=DIRA, red=DIRB, yellow=ENABLE) on the same GPIO pins the
  real L293D control lines use.
- **Simulated vs. bench:** the Wokwi sketch uses a 16x2 `wokwi-lcd1602`
  part (`LiquidCrystal_I2C`, address 0x27). The real bench build
  substituted a 128x64 SSD1306 OLED instead (`Adafruit_SSD1306`, address
  0x3C) — see Section 5's open item. The bench also added three dedicated
  debug LEDs (GPIO 23/25/27) mirroring the L293D control lines, which the
  Wokwi sim doesn't have (its 3 LEDs *are* the L293D stand-in, not a
  separate debug layer).
- **Differences found:** none in state-machine behavior — every scenario in
  the Wokwi `README.md` matched real bench behavior once the wiring issues
  in Section 5 were fixed.
