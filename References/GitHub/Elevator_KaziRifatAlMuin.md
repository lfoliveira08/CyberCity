# KaziRifatAlMuin / Automated-Elevator

- **Link:** https://github.com/KaziRifatAlMuin/Automated-Elevator
- **System:** Elevator (BSH and FH)
- **Status:** Ideas only
- **License:** none listed, so there's no permission to copy the code. Take ideas and write our own code.
- **Checked:** Sep 23, 2026 (8 commits on `main` at that time)

## What it is

A 2-floor automatic elevator on an **ESP32** using the **Arduino framework**
(same as ours). Main file: `Automatic_Elevator_ESP32.ino`.

Hardware:
- DC motor with an H-bridge driver, PWM speed control
- MG995 servo for the door
- 2× HC-SR04 ultrasonic sensors (detect someone waiting at each floor)
- ACS712 30A current sensor (overcurrent protection)
- IR obstacle sensor (stops the door closing on something)
- Green/red LEDs, 2 buzzers, optional push switch
- Local web page with `/status` and `/control?floor=<1|2>`

## Ideas worth borrowing

1. **Current sensing to detect a stuck motor.** Our `ELEVATOR_FAULT` uses a
   stall *timeout* (the cab takes too long to arrive). Measuring motor
   current catches a jam immediately and is a real-world safety feature.
   It's a possible upgrade, but it adds a part (ACS712) and an analog pin
   (use ADC1).
2. **Door obstacle detection with an IR sensor.** Our elevator has no door
   sensor. It's worth considering, and the IR part is already in the Door
   node's parts list.
3. **An unauthenticated control endpoint (`/control?floor=`) as a security
   lesson.** Anyone on the network can call the elevator. That's the same
   weakness as our Scenario 5 (Elevator Lockout) through an unauthenticated
   Modbus write. It's a good side-by-side for the presentation.

## Where it differs from our design (don't copy these)

| Theirs | Ours |
|---|---|
| 2 floors | 3 floors |
| Servo door (MG995) | No door actuator in the BOM |
| Ultrasonic presence detection | 3 floor-call push buttons |
| LEDs and buzzers for status | I2C 16×2 LCD |
| Web API over Wi-Fi | Modbus TCP to OpenPLC (Phase B) |
| Their own pin numbers | Our pins: `BSH_Systems/01_BSH-Elevator.md` |
