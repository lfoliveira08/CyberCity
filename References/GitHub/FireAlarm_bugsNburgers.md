# bugsNburgers / IOT-Fire-Alarm-System

- **Link:** https://github.com/bugsNburgers/IOT-Fire-Alarm-System
- **System:** Fire Alarm (BSH and FH)
- **Status:** **Ideas only** — sensor selection and wiring kept, its entire software layer rebuilt
- **License:** not yet recorded. Since the project took its *wiring* rather than its code, this matters less here — but check before copying any source
- **Checked:** not independently verified; written from `References/INDEX.md` and the Sep 15, 2026 entries in `Piranga_Workflow_v2.md` Section 14

## What it is

An ESP32 fire-alarm project: flame sensor + MQ-2 gas/smoke sensor + DHT11,
reporting over WiFi to the Blynk cloud, plus a USB-serial webcam capture script.

## What we took

The **sensor choice and the GPIO wiring**, essentially unchanged: flame sensor on
GPIO 18 (active LOW), MQ-2 analog out on GPIO 34. Wiring does not depend on what
is polling the ESP32, so there was no reason to redo it.

This is what turned BSH/FH-FireAlarm from a bare pull-station buzzer into a real
detection node (`FLAME_DETECTED` DI1, `GAS_SMOKE_ALERT` DI2, `GAS_RAW_VALUE` IR0).

## What we rejected, and why

- **The Blynk cloud transport.** Every other node in this project speaks direct
  Modbus TCP to its own OpenPLC. Routing one node through an external cloud broker
  would break that uniformity, and it is a poor fit for a testbed whose whole
  point is demonstrating resilience without outside infrastructure.
- **The webcam/serial capture piece**, which assumes a permanently tethered
  desktop. A sensor node embedded in a 3D-printed building cannot provide that.
- **Its fixed magic-number gas threshold** (`GAS_THRESHOLD = 1500`, with comments
  inconsistently mentioning 2150). A threshold tuned on one sensor in one room is
  fragile across units, rooms and humidity. Replaced with a boot-time clean-air
  baseline plus `GAS_ALERT_MARGIN` — same 20-sample approach, relative instead of
  absolute. The margin (400) is still a placeholder pending bench calibration,
  and the "baseline is computed once, at boot" limitation carries over: **power
  the node on in clean air.**

## A gap in the reference worth remembering

It says nothing about the MQ-2's output voltage. Many MQ-2 breakout boards run
their heater at 5V and can swing AOUT near that rail, while the ESP32's ADC pins
are 3.3V tolerant — so copying its wiring literally can damage GPIO 34. Measure
AOUT with a multimeter first and add the 1k/2k divider if it exceeds 3.3V; see
`BSH_Sensors/16_MQ-2_Gas_Smoke_Sensor.md`. This is exactly the kind of omission
that gets copied without noticing.
