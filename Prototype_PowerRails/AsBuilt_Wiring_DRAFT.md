# As-built wiring — DRAFT (new BSH-PowerBox, breadboard build)

**Draft, Oct 28, 2026, for Luis to correct. Source: Luis's reports and one bench photo, not a measurement.** It becomes `Firmware/BSH-PowerBox/Schematic/Connections.md` (Step 3 of `ADOPTION_CHECKLIST.md`) only after Luis confirms it. Labels: **Luis said** = he told me; **photo** = I read it from his bench photo, unconfirmed; **assumed** = my guess; **unknown** = not recorded.

## The board

| Item | Value | Source |
|---|---|---|
| Board | ESP32 DevKit on a breadboard (the former spare) | Luis said |
| Pin count | unknown (30-pin like the real nodes is assumed) | unknown |
| USB connector | micro-USB | photo |
| Powered from | USB cable to the laptop during tests | photo |
| IP | 192.168.50.14 (the old PowerBox address, on purpose) | Luis said |
| Firmware | `Firmware/PowerRails_Node/PowerRails_Node.ino`, flashed by Luis | Luis said |

## Per rail

| | Rail 1, Elevator | Rail 2, FireAlarm | Rail 3, Door |
|---|---|---|---|
| ESP32 pin to relay IN | GPIO 25 | GPIO 26 | GPIO 27 | 
| Relay VCC | assumed ESP32 5V/VIN rail | assumed same | assumed same |
| Relay GND | assumed ESP32 GND rail | assumed same | assumed same |
| Relay COM | **ESP32 VIN pin** (Luis said) | same | same |
| Relay NO | the node's 5V pin on its expansion board (Luis said "VIN and GND" for the real nodes) | same | same |
| Node GND | joined to the PowerBox GND (without it the node does not power up; Luis said) | same | same |
| Relay contact | NO, so off means no power (decided Oct 27) | same | same |
| Relay board polarity | active-HIGH assumed, not confirmed per board | unknown | unknown |

Relay boards: Songle SRD-05VDC-SL-C type, as in Luis's photo (assumed all three are the same). Rails to nodes: 25 = Elevator, 26 = FireAlarm, 27 = Door (Luis, Oct 28).

## Electrical points to check (acceptance, Step 7)

- COM comes from the **VIN pin**, so all three nodes draw through the DevKit's own 5V path and the USB source behind it. That path's limit is not known (not verified: whether the DevKit has a diode or fuse there) and no current was measured. In the final build the PowerBox's USB comes from a hub or strip, not the laptop.
- Each relay coil is about 70 mA (`BSH_Sensors/09_Relay_Module.md`), from the same 5V source: about 210 mA for three.
- The motor and servo supplies of the Elevator and Door are separate and are **not** switched (`BSH_Sensors/01_L293D_Motor_Driver.md`, `06_SG90_Servo.md`; how the bench feeds them was reported as power banks).
- A node's USB cable and its 5V pin must never be connected at the same time (back-feed unverified).
- Relay GPIOs 25, 26, 27 are ordinary pins per `References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md`; boot-time pin behaviour was not tested.

## Still to fill in (Luis)

- [ ] Pins per side on the DevKit (15 per side is the 30-pin board; 19 per side is the 38-pin board; general figures)
- [ ] Is relay VCC/GND on the same VIN and GND rail as COM?
- [ ] Which relay board feeds which node, and its polarity if tested
- [ ] The pin label on each real node's expansion board that the NO wire goes to
