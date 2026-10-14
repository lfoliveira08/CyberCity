# References — Index

Every outside source the project uses: books, official guides, datasheets,
GitHub repos, and web articles. **Read this file first.** It says what each
source is for, which system it helps, and whether the project adopted it.

Folders are organized by *type* (Books, Guides, Datasheets, GitHub). The
"By system" section below groups the same sources by *system*, so you can
find everything for the system you're working on in one place.

**Status key:**
- **Adopted**: the project uses it directly (a library we install, a design we followed)
- **Ideas only**: we learned from it but didn't copy its code or design
- **Background**: theory and reference reading
- **Rejected**: evaluated and not used (the reason is noted, so nobody re-evaluates it)
- **Under evaluation**: a candidate for a decision that has not been made yet (added Oct 11, 2026 for the SCADA/HMI choice)

**Copyright rule:** `Books/` holds published, copyrighted textbooks. Keep
them in this local folder only. Don't upload them to the Claude project, a
shared Drive, or anywhere public. Everything else is fine to share (check
each guide's license page to be sure).

Last updated: Oct 11, 2026

---

## Books/ (local only)

| File | What it is | Most useful parts | Status |
|---|---|---|---|
| `White_Making_Embedded_Systems_2E_2024.pdf` | Elecia White, *Making Embedded Systems*, 2nd ed. (O'Reilly, 2024) | Ch. 2 Creating a System Architecture · Ch. 4 Inputs, Outputs, and Timers · Ch. 5 Interrupts · Ch. 6 Managing the Flow of Activity (state machines) · Ch. 7 Communicating with Peripherals · Ch. 9 Getting into Trouble (debugging) · Ch. 10 Building Connected Devices | Background: the theory behind our firmware patterns (non-blocking `millis()` loops, debouncing, watchdogs, state machines) |
| `Dunn_Intro_Instrumentation_Sensors_Process_Control.pdf` | William Dunn, *Introduction to Instrumentation, Sensors, and Process Control* | Ch. 3 R/L/C circuits and filters · Ch. 4 Amplifiers · chapters on Temperature, Position/Force/Light, Humidity and Other Sensors · Regulators, Valves, and Motors · **Programmable Logic Controllers** · Signal Conditioning and Transmission · Process Control | Background: how sensors work, calibration and thresholds; the PLC chapter supports the OpenPLC ladder-logic work (Phase C) |

## Guides/ (shareable)

| File | What it is | Most useful parts | Status |
|---|---|---|---|
| `Espressif_ESP-IDF_Guide_v5.3.2.pdf` | Espressif's official ESP-IDF Programming Guide, v5.3.2 (3,321 pages) | 2.6 Peripherals API (GPIO, ADC, LEDC/PWM, SPI, I2C) · 2.5 Networking APIs (Wi-Fi) · 4.x API Guides · **5 Security Guides** (secure boot, flash encryption) | Background: the final word on ESP32 hardware behavior (ADC range, GPIO 34–39 having no pull-ups, PWM channels). **Note:** written for ESP-IDF, not Arduino. Our sketches use the Arduino core, which runs on top of ESP-IDF, so the hardware facts apply but the code examples don't paste straight in |
| `IOT-OPEN-EU.pdf` | IOT-OPEN.EU open course book (multi-university EU project: ITMO, Riga TU, Silesian UT, Tallinn UT, U. Messina) | Ch. 4 IoT Hardware Overview · Ch. 5 IoT Communication and Networking · **Ch. 7 IoT Security and Privacy** · Ch. 8 Energy Consumption | Background: networking theory for Phase B, and security theory for the attack scenarios |

## Datasheets/ (shareable)

Empty for now. One PDF per part, named `<Part>_datasheet.pdf`. Priority
datasheets to add, since the docs keep flagging questions only they can
answer:

- [ ] MQ-2 and **MQ-9** (the bench uses an MQ-9): AO voltage range, heater warm-up
- [ ] MFRC522 / RC522: 3.3V-only supply, SPI timing
- [ ] HC-SR04: ECHO output voltage (the reason for the 1kΩ/2kΩ divider)
- [ ] L293D: motor supply (VCC2), current limits, thermal notes
- [ ] SG90 servo: stall current (brownout risk)
- [ ] ESP32-WROOM-32 / DevKitC V4: pinout, strapping pins, ADC1 vs ADC2
- [ ] **DHT22 / AM2302** (FH-HVAC): pull-up value, 2s minimum sample interval,
      accuracy at the temperatures the demo room will actually be at
- [ ] **SSD1306 OLED** (BSH-Elevator bench): I2C address variants (0x3C vs 0x3D)

## GitHub/ (notes on repos, not copies of the code)

**Every reference gets its own `.md` note in this folder** (what it is, what we
took, what differs, what to watch) — libraries included. That rule changed on
Sep 26, 2026: libraries used to be listed in the table without a note, which
left the things the project actually depends on every day as the only
undocumented references. We don't clone repos here — the notes are the record.

Notes are written from this project's own records unless they say otherwise.
Where a note says a detail is unverified — a license, a version, a commit count
— it means exactly that: confirm it against the repo before relying on it, and
then edit the note.

| Repo | System | What it is | Status | Note |
|---|---|---|---|---|
| [emelianov/modbus-esp8266](https://github.com/emelianov/modbus-esp8266) (`ModbusIP_ESP8266`) | All 10 nodes | Modbus TCP slave library for ESP32/ESP8266 | **Adopted** (Phase B backhaul) | `AllNodes_emelianov_modbus-esp8266.md` |
| [miguelbalboa/rfid](https://github.com/miguelbalboa/rfid) (`MFRC522`) | Door | RC522 RFID reader library | **Adopted** | `Door_miguelbalboa_rfid.md` |
| [silahcen001/DALI-ESP32](https://github.com/silahcen001/DALI-ESP32) (`DaliController`) | PowerBox | DALI master library | **Rejected** — Sep 28, 2026, dropped in favor of a plain relay (Power Box no longer uses any field-bus library) | `PowerBox_silahcen001_DALI-ESP32.md` |
| `ESP32Servo` (Arduino Library Manager) | Door | Servo library (standard `Servo.h` doesn't support the ESP32) | **Adopted** | `Door_ESP32Servo.md` |
| `LiquidCrystal_I2C` (Arduino Library Manager) | Elevator | I2C 16×2 LCD library | **Superseded on the bench** — still used by the Wokwi simulation, but the real build uses an SSD1306 OLED (Sep 25, 2026) | `Elevator_LiquidCrystal_I2C.md` |
| [bugsNburgers/IOT-Fire-Alarm-System](https://github.com/bugsNburgers/IOT-Fire-Alarm-System) | FireAlarm | Flame + MQ-2 + DHT11 fire alarm over Wi-Fi/Blynk | **Ideas only**: sensor choice and wiring kept, cloud transport replaced with Modbus (see `Piranga_Workflow_v2.md` §14, Sep 15) | `FireAlarm_bugsNburgers.md` |
| [JordyMail/Fire-and-Smoke-Detection-ESP32](https://github.com/JordyMail/Fire-and-Smoke-Detection-ESP32) | FireAlarm | DHT22/MQ7/MQ135/dust sensor over MQTT | **Rejected**: depends on a public cloud broker (§14, Sep 15) | `FireAlarm_JordyMail.md` |
| [monkeyboard/Wiegand-Protocol-Library-for-Arduino](https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino) | Door | Wiegand reader library | **Rejected**: the reader was swapped to the RC522 (§14, Sep 15) | `Door_monkeyboard_Wiegand.md` |
| `bacnet-stack-esp32`, Chipkin `ESP32-BACnetServerExample` | Elevator, FireAlarm | BACnet on ESP32 | **Rejected** for now: the BACnet backhaul isn't used (`Alternate_Design_BACnet_MQTT_Option.md`) | `ElevatorFireAlarm_BACnet_ESP32.md` |
| `Adafruit SSD1306` + `Adafruit GFX` (Arduino Library Manager) | Elevator (BSH) | 128x64 I2C OLED driver — **what the working bench firmware actually uses**, substituted for the 16x2 LCD on Sep 25, 2026 | **Adopted** | `Elevator_Adafruit_SSD1306.md` |
| `Adafruit DHT sensor library` + `Adafruit Unified Sensor` (Arduino Library Manager) | HVAC (FH) | DHT22/AM2302 temperature-humidity driver | **Installed** (Library Manager) for FH-HVAC; used with a DHT11 on the bench, DHT22 in Wokwi | version not recorded (Luis does not know); headers used: WiFi.h, ModbusIP_ESP8266.h, DHT.h |
| [KaziRifatAlMuin/Automated-Elevator](https://github.com/KaziRifatAlMuin/Automated-Elevator) | Elevator | 2-floor ESP32 elevator with current-sensing stall detection | **Ideas only**. Note: `GitHub/Elevator_KaziRifatAlMuin.md` | `Elevator_KaziRifatAlMuin.md` |
| [thiagoralves/OpenPLC_v3](https://github.com/thiagoralves/OpenPLC_v3) | All 8 nodes | OpenPLC Runtime: web UI, Modbus server, Modbus master for slave devices | **Adopted** (planned, Phase C; nothing installed yet) | `AllNodes_thiagoralves_OpenPLC_v3.md` |
| [frangoteam/FUXA](https://github.com/frangoteam/FUXA) | SCADA/HMI | Web-based SCADA/HMI editor, MIT | **Under evaluation** (fallback #2) | `SCADA_frangoteam_FUXA.md` |
| [Rapid SCADA](https://rapidscada.org/) | SCADA/HMI | Open-source SCADA platform, free Community edition | **Under evaluation** (fallback #1 if Ignition is not licensed) | `SCADA_RapidScada.md` |
| [ScadaBR/ScadaBR](https://github.com/ScadaBR/ScadaBR) | SCADA/HMI | Open-source Java SCADA, last release 2021 | Not chosen (weak candidate) | `SCADA_ScadaBR.md` |

## Papers/ (annotated bibliography)

Research papers and surveys from the professor's reading list (IEEE, ASEE,
ASME). Full list, status and rules are in `Papers/INDEX_Papers.md`. Added
Sep 30, 2026. All entries are **To read**: they came from search results, so
authors and years are deliberately not recorded until someone opens the paper.
Paywalled papers stay as links only (same rule as `Books/`).

- OpenPLC / Modbus TCP / ICS testbeds (6 entries)
- Education and testbed design (4 entries)
- Mechanical and manufacturing, ASME (3 entries)

## Web articles and tools

| Link | System | What it is | Status |
|---|---|---|---|
| [Electropeak: calibrate and use the MQ-9 with Arduino](https://www.hackster.io/electropeak/how-to-calibrate-use-mq9-gas-sensor-w-arduino-e93cb1) | FireAlarm | MQ-9 calibration how-to | Background (the bench uses an MQ-9) |
| [Wokwi docs: supported parts](https://docs.wokwi.com/getting-started/supported-hardware) | All (Phase A0) | Which parts Wokwi simulates natively | Adopted (Phase A0, `Firmware/WORKFLOW.md` §7) |
| [DG01D-A130 gearmotor datasheet (SparkFun-hosted)](https://cdn.sparkfun.com/datasheets/Robotics/DG01D.pdf) | Elevator | 48:1 gearmotor spec sheet -- voltage, no-load RPM/current, torque | **Adopted** (confirmed motor in hand, Sep 24, 2026 -- see `Firmware/BSH-Elevator/L293D_Motor_Driver/Wiring.md`) |
| [OpenPLC Editor docs: Modbus addressing](https://edge.autonomylogic.com/docs/openplc-editor/communication/modbus/addressing) | All (Phase C) | How Modbus coils, discrete inputs and registers map to `%QX`, `%IX`, `%IW`, `%QW` on the PLC's own Modbus server. Does not state where slave-device data appears | **Adopted** (basis of `PLC_Logic/Templates/Address_Mapping.md`) |
| [Ignition Maker Edition docs](https://docs.inductiveautomation.com/docs/8.3/other-editions/ignition-maker-edition.md) | SCADA/HMI | Free license for non-commercial and personal educational use; 10,000 tags, 10 Perspective sessions; includes Perspective, Modbus and OPC UA drivers; educational institutions are told to contact sales for classroom/senior-project options | **Adopted** (professor approved Oct 12, 2026; company license pending) |
| [Configuring openPLC and Rapid SCADA 6 Modbus Integration](https://industrialmonitordirect.com/blogs/knowledgebase/configuring-openplc-and-rapid-scada-6-modbus-integration) | SCADA/HMI | Third-party step-by-step guide to polling OpenPLC from Rapid SCADA 6 | Background (third-party, unverified; see `GitHub/SCADA_RapidScada.md`) |

---

## By system

Use this section when you start work on a system.

**FireAlarm**
- White Ch. 4 (inputs, debouncing), Ch. 6 (non-blocking loop)
- Dunn: Humidity and Other Sensors (gas sensing), Signal Conditioning
- ESP-IDF 2.6: ADC (why clean air already reads near the top on GPIO 34 with the MQ board on 5V)
- bugsNburgers repo (ideas only) · Electropeak MQ-9 article
- Datasheets: MQ-2, MQ-9

**Elevator** (BSH only — Phase A complete 2026-09-25)
- White Ch. 6 (state machines), Ch. 9 (stall and fault handling)
- Dunn: Regulators, Valves, and Motors · Position, Force, and Light
- ESP-IDF 2.6: LEDC (PWM motor speed), I2C (display)
- `Adafruit SSD1306` + `Adafruit GFX` (the bench build) · `LiquidCrystal_I2C`
  (the Wokwi simulation only) · KaziRifatAlMuin/Automated-Elevator (ideas only)
- Datasheets: L293D, DG01D-A130 gearmotor (adopted, linked below), SSD1306
- Note: Founder's Hall no longer has an elevator — FH slot 1 became HVAC on
  Sep 26, 2026, so nothing in this group applies to FH any more

**Door**
- White Ch. 7 (SPI peripherals)
- ESP-IDF 2.6: SPI, GPIO (GPIO 34 has no pull-up)
- IOT-OPEN Ch. 7 (access-control security → Scenario 3, RFID clone/replay)
- `miguelbalboa/rfid` · `ESP32Servo`
- Datasheets: RC522, SG90

**PowerBox**
- Dunn: Signal Conditioning and Transmission
- No field-bus library any more — plain relay for lights and fan (DALI dropped Sep 28, 2026; see `PowerBox_silahcen001_DALI-ESP32.md`)

**HVAC** (FH only — newest system; Phase B bench-confirmed 2026-10-07)
- Dunn: Temperature sensors · Process Control (setpoint and feedback loops)
- White Ch. 4 (inputs and timers), Ch. 6 (non-blocking loop — the DHT sensor's read
  is blocking and must be called on a timer)
- `Adafruit DHT sensor library` (planned) · sensor sheet:
  `FH_Sensors/17_DHT22_Temp_Humidity_Sensor.md`
- Datasheets: DHT22 / AM2302

**Networking and OpenPLC (Phases B/C, all systems)**
- IOT-OPEN Ch. 5 (networking), Ch. 7 (security)
- Dunn: Programmable Logic Controllers, Process Control
- ESP-IDF 2.5 (Wi-Fi), 5 (Security Guides)
- `emelianov/modbus-esp8266`
- Papers: `Papers/INDEX_Papers.md`, section A (OpenPLC / Modbus TCP testbeds) and
  section B (educational CPS testbeds)
- `thiagoralves/OpenPLC_v3` · OpenPLC Editor docs, Modbus addressing
- The OpenPLC programs themselves: `PLC_Logic/` (Templates/Address_Mapping.md)

**SCADA/HMI** (Phase C; Ignition chosen Oct 12, 2026, company license pending)
- Chosen: Ignition Maker Edition. Fallbacks: Rapid SCADA, then FUXA. Not chosen: ScadaBR, Node-RED (earlier plan). Full options table: `Piranga_Workflow_v2.md` Section 10
- Dunn: Programmable Logic Controllers, Process Control

---

## How to add a source

1. Put the file in the right folder (or, for a repo, add a row and a note in `GitHub/`).
2. Add a row above, including its **status**, which system it helps, and the chapters or files that matter.
3. **Write the note file** and put its name in the row's Note column. Libraries
   are not exempt (rule changed Sep 26, 2026).
4. For a library, note the version you installed. Libraries change, and several
   have incompatible forks — `silahcen001/DALI-ESP32` is the one already known to
   bite this project.
