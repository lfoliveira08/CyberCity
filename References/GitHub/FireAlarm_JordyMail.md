# JordyMail / Fire-and-Smoke-Detection-ESP32

- **Link:** https://github.com/JordyMail/Fire-and-Smoke-Detection-ESP32
- **System:** Fire Alarm (evaluated for, not used)
- **Status:** **Rejected** — Sep 15, 2026
- **License:** not recorded; irrelevant, since nothing was taken
- **Checked:** not independently verified; written from `References/INDEX.md` and `Piranga_Workflow_v2.md` Section 14

## What it is

An ESP32 fire/smoke detection project using a DHT22, MQ7 and MQ135 gas sensors
and a dust sensor, publishing over MQTT.

## Why it was rejected

It depends on a **public cloud MQTT broker.** Same reasoning that removed the
Blynk transport from the `bugsNburgers` reference: this project's ten nodes all
speak direct Modbus TCP to their own dedicated OpenPLC instance with no bridge
anywhere, and a security testbed built to demonstrate resilience should not have
an outside dependency in the middle of it.

Its sensor mix was also a poorer fit than `bugsNburgers`'s for a *fire alarm*
specifically — MQ135 is tuned for general air-quality gases and only detects
smoke as a side effect, which is the same reason MQ-2 was chosen over MQ-135 when
the Fire Alarm upgrade was executed.

## Why the note exists anyway

So nobody re-evaluates it. Two candidate fire-alarm projects were reviewed on
Sep 15, 2026; this is the one that lost, and the reason is architectural rather
than technical — it would still lose today.
