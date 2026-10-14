# ESP32Servo

- **Link:** installed through the Arduino Library Manager; this project's docs do not record an upstream repo URL. Record it here once you install it and can read it off the Library Manager entry
- **System:** Door (BSH and FH) — the SG90 gate arm
- **Status:** **Adopted**
- **License:** not yet recorded
- **Version installed:** not yet recorded
- **Checked:** not independently verified; written from `References/INDEX.md`

## What it is

A servo library that works on the ESP32. It exists in this project's stack for a
specific reason worth remembering: **the standard Arduino `Servo.h` does not
support the ESP32.** The ESP32 generates servo PWM through its LEDC peripheral
rather than the AVR timer code `Servo.h` assumes, so the stock library will not
compile or will not behave.

## What we use it for

Driving the SG90 micro servo that acts as the gate arm on the Door node
(GPIO 13). Pins and wiring: `BSH_Systems/03_BSH-Door.md` and
`BSH_Sensors/06_SG90_Servo.md`.

## Things to watch

- The SG90 needs its own external 5V supply, not the ESP32's rail — stall current
  is a real brownout risk, and a browning-out ESP32 looks like a crashed or
  unresponsive Modbus slave, which is a confusing way to learn this.
- The Door node is built third in the current order, so this will be bring-up
  alongside the RC522 — two new-to-the-node parts at once. Wire and test them
  separately first (`Firmware/BSH_Door_Bringup_Checklist.md`).
