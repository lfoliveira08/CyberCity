# Logs index -- newest first

Rules and template: `README.md`. History before Oct 15, 2026: `Piranga_Workflow_v2.md` Section 14.

| Date | File | Summary |
|---|---|---|
| 2026-10-20 | `2026-10-20.md` | BSH-FireAlarm port 502 open again; first Modbus read from the Kali VM of every `PUB_*` address, heartbeat rose 101 -> 131 (PLC polling the ESP32 live); folder `Kali/` created for attack scripts and reports (not in use yet); BSH-Door PLC built in the Editor by Luis (8 rungs), `Program.st` regenerated, uploaded to 192.168.50.113 and tested on real hardware (slave ID 0 and address block verified, Kali heartbeat read 599 -> 604, functional tests passed); a held open command keeps the gate open and blocks the RFID reader; BSH-Elevator PLC built in the Editor by Luis (9 rungs, call-floor rungs use a 300 ms `TP` pulse, task `T#20ms`), compiled on 192.168.50.111 after a `Program.st` fix (located and unlocated variables need separate VAR blocks), Kali read test passed (heartbeat 84 -> 90), 3 functional tests passed; ESP32 timeouts after the first start cleared by a reboot (cause unknown); a detached motor lead stopped the car and was repaired |
| 2026-10-19 | `2026-10-19.md` | Ignition license confirmed; "license pending" updated in the current docs; license details and the install still to record; BSH-FireAlarm PLC program uploaded and verified on real hardware (slave ID 0 proven, 3 functional tests passed); Kali VM 102 built and given static IP 192.168.50.40 (guest agent, ISO detach and SSH from the laptop confirmed) |
| 2026-10-15 | `2026-10-15.md` | Created the Logs/ folder and registered it in the project docs; evaluated OpenPLC Runtime v4 and decided to stay on v3 through the Dec 11 demo |
