# BSH-Door -- Wokwi Simulation (Phase A0)

A simulation of the whole BSH-Door system: same GPIO pins, same logic as the
Phase A bench sketch (`../BSH_Door_Node/BSH_Door_Node.ino`). Use it to learn how
each part behaves, how it is wired and how the gate reacts, without touching
real hardware.

Rule: `Firmware/WORKFLOW.md` Section 7.

**Status: built and run Oct 9, 2026.** Luis ran it and it looked fine; the
individual scenarios were not itemized. A0 is logged in each component's
`Build_Log.md` (the "Simulated in Wokwi first" line) and in the status tracker. The parts and wiring follow the sensors actually used on the
bench: RC522 over SPI, SG90 servo, common-cathode RGB LED, IR module and reed
module (S/V/G).

## Files

| File | What it is |
|---|---|
| `diagram.json` | The parts and every wire. Wokwi draws the wiring diagram from it |
| `sketch.ino` | The firmware. Same logic as the bench sketch; simulation-only differences sit behind `WOKWI_SIM` (see below) |
| `libraries.txt` | Libraries the sketch needs: `MFRC522`, `ESP32Servo` |
| `README.md` | This guide |

## How to open it

1. Go to **wokwi.com** and start a new **ESP32** project.
2. In the `sketch.ino` tab, delete everything and paste this folder's `sketch.ino`.
3. In the `diagram.json` tab, delete everything and paste this folder's `diagram.json`.
4. In the `libraries.txt` tab (create it with the "+" menu if it is missing), paste this folder's `libraries.txt`. This installs `MFRC522` and `ESP32Servo`.
5. Press the green **Play** button. The Serial Monitor opens underneath.
6. Optional: sign in and **Save** so the project gets its own link the team can reuse.

The part positions in `diagram.json` were chosen without seeing the screen. If
parts overlap, drag them apart; the wires stay connected.

## What is in the circuit

| GPIO | Part in Wokwi | Real part | Notes |
|---|---|---|---|
| 18 / 19 / 23 / 32 / 33 | `board-mfrc522` (SCK / MISO / MOSI / SDA / RST), 3.3V | RC522 RFID reader | Native part, six preset cards |
| 13 | Servo | SG90 servo | Powered from the board's 5V pin here; on the bench it needs an external 5V supply |
| 25 / 26 / 27 | RGB LED (common cathode) + 3x 220 ohm | RGB LED + 3x 220 ohm | Same wiring as the bench |
| 14 | **Slide switch (stand-in)** | IR obstacle sensor | Slide **LEFT = object present** (pin LOW), **RIGHT = clear** (pin HIGH) |
| 34 | **Slide switch + 10k pull-up (stand-in)** | Reed switch module (S/V/G) | Slide **LEFT = magnet near = door closed** (pin LOW), **RIGHT = magnet away = door open** (pin HIGH) |

## Scenarios to try

Start with nothing triggered: the LED should be **steady green** and the Serial
Monitor should print a line like this twice a second:

```
Reed raw=0 -> DoorPositionOpen=0   IR raw=1 -> IRDetected=0
```

| # | Do this | You should see |
|---|---|---|
| 1 | Nothing (normal operation) | Green LED, servo at 0 degrees, the line above repeating |
| 2 | Click the RFID reader, **tap the Blue card** (01:02:03:04). Keyboard: press `t` after selecting it | `Card UID: 01 02 03 04`, `Access granted`, LED turns **red**, servo sweeps to 90 degrees, holds about 2 s, returns to 0, LED back to green |
| 3 | Tap the **Red card** (AA:BB:CC:DD) | `Card UID: AA BB CC DD`, `Access denied`, LED red for 1 s, servo does not move |
| 4 | Slide the **IR switch LEFT** | `IR raw=0 -> IRDetected=1`. Nothing else reacts: the IR value is reported honestly and never opens the gate by itself |
| 5 | With no card tapped, slide the **reed switch RIGHT** | `Reed raw=1 -> DoorPositionOpen=1` while the servo never moved and no card was granted. This mismatch (door open, no access granted) is the signature SCADA will correlate in Phase B and C. The sketch itself deliberately does not judge it |
| 6 | Tap Blue, then also slide the reed RIGHT while the servo is open and back LEFT after it closes | Shows that `DoorPositionOpen` comes from the reed switch, not from the servo command. The simulator does not link them; you play the magnet |
| 7 | In `sketch.ino` set `IR_ACTIVE_LOW = false`, restart, slide the IR switch RIGHT | `IRDetected=1` now appears on the opposite position. This is exactly the flag to set on the bench after reading the real IR module's raw value |
| 8 | Same for `REED_LOW_MEANS_CLOSED = false` | `DoorPositionOpen` inverts. Same idea for the reed module |

Things to notice while you play:
- **Three independent values:** IR, door position and the access decision never
  feed each other locally. In Phase B they become `IR_DETECTED`,
  `DOOR_POSITION` and `ACCESS_GRANTED`, and the SCADA layer does the correlating.
- **Red means two things:** red shows while the gate is open or moving and also
  for 1 s on a denied card. Green means closed and idle.
- **Blocking code (Phase A only):** while the gate opens (about 2.5 s) and while
  a denied card is shown (1 s) the sensor line stops printing, because the
  sketch uses `delay()`. Phase B has to replace these with `millis()` timing so
  Modbus can keep running.
- **No re-read:** the sketch calls `PICC_HaltA()` after every read so one tap
  is one decision.

## Real bench differences

The simulation cannot show these, so they still have to be checked on the real bench:

- **RFID reader:** the real RC522 is a 3.3V-only part with limited range, and
  antenna placement inside the printed gate matters. The real authorized card
  is `B3 12 87 2D`, not Wokwi's Blue card; the sketch swaps it behind `WOKWI_SIM`.
- **IR polarity and range:** the stand-in is a slide switch, so the real
  module's polarity (`IR_ACTIVE_LOW`), sensitivity potentiometer and sunlight
  interference only show up on the bench. The default (`true`) has **not** been
  checked on the real module.
- **Reed module:** the stand-in uses an explicit external 10k pull-up because
  GPIO 34 has no internal pull-up. The real part is a 3-pin S/V/G module and it
  is **not yet confirmed** whether it has its own pull-up. Contact bounce and
  polarity (`REED_LOW_MEANS_CLOSED`) are also unchecked.
- **Servo power:** the simulated servo is powered from the board's 5V pin. The
  real SG90 needs an external 5V supply with a common ground, or it can brown
  out the ESP32. Wokwi does not model current draw.
- **Voltage:** Wokwi never warns about a 5V signal on a 3.3V pin.

## Troubleshooting

- **Compile error about `MFRC522.h` or `ESP32Servo.h`:** `libraries.txt` was not
  loaded. Paste it into the `libraries.txt` tab and run again.
- **Tapping a card does nothing:** the simulation must be running, and you tap
  from the reader's own control panel (or focus the panel and press `t`).
  Wokwi's panel layout can differ slightly from this description; see Wokwi's
  MFRC522 documentation.
- **Servo does not move:** check the servo's V+ wire goes to the board's 5V pin.
- **LED stuck on one colour:** check the three RGB legs and that the common
  pin goes to GND.
- **Alarm-style readings at boot:** the IR switch should start RIGHT and the
  reed switch LEFT. `"value": "1"` / `"value": "0"` in `diagram.json` set those.
