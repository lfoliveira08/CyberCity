# FH-HVAC -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **program running; functional tests reported as worked and the Kali read test passed (Oct 21, 2026)**. Open: the compile log and the Settings values were not shown (files drafted Oct 11, 2026; container started Oct 14; ladder built in the Editor, `Program.st` regenerated, program uploaded and run Oct 21).

## Phase C steps

- [x] Container `OpenPLC-FH-HVAC` running at 192.168.50.121; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [x] Modbus server answering on port 502 (proven Oct 21 by the Kali read). **Not shown:** the Settings page (Modbus checkbox, S7 and EtherNet/IP off, Hardware = Blank), Polling Period and Timeout.
- [x] ESP32 added as a slave device (192.168.50.21:502), generic TCP, slave ID 0, sizes from `Register_Map.md`. **Slave Devices listing shown Oct 21 and matches the draft:** DI `%IX100.0`-`%IX100.1`, DO `%QX100.0`-`%QX100.0`, AI `%IW100`-`%IW103`, AO `%QW100`-`%QW100`. **Slave ID 0 CONFIRMED by live polling Oct 21, 2026:** `PUB_HEARTBEAT` read 135 then 140 from Kali about 5 s apart.
- [x] Ladder built by Luis in the OpenPLC Editor (project `FH-HVAC-ESP32/`, saved Oct 21): 8 rungs R1-R8, task `T#20ms` (his Editor setting, cyclic, priority 1). Saved `main.ld` read and checked: 16 variables with the expected addresses; R1 contact to coil; R2 MOVE (`SCADA_TARGET_TEMP_C` to `TARGET_TEMP_C`, EN from the left rail, ENO to the right rail); R3-R4 contact to coil; R5-R8 MOVE. Every coil and MOVE output has one writer. Luis's R1 and R2 screenshot was checked; R3-R8 were checked in the saved file only.
- [x] `Program.st` regenerated from the saved project (replaces the Oct 11 hand draft, no backup kept), uploaded by Luis and run. **The compile log was not pasted.** The program started and polled the ESP32, so it compiled
- [x] Monitoring page values: `HEARTBEAT` counting and `RECONNECT_COUNT` 0 are confirmed through the Kali read (Luis did not paste Monitoring screenshots)
- [x] A Modbus client reads the `PUB_*` addresses (Oct 21, 2026), from the Kali VM (192.168.50.40), read-only stdlib script `~/hvac_read.py` on Kali (unit ID 1, FC 1 coils 0-2, FC 3 holding registers 0-4)
- [x] **Address convention on this container:** `%IX100.x` / `%QX100.x` / `%IW100+` / `%QW100` confirmed from the Slave Devices listing (Oct 21)

## Tests

Luis ran the functional tests on Oct 21 (values forced on the Monitoring page) and reported "the test ran fine". No values, LED colours or step list were reported, so these are logged as **reported as worked**, not measured:

- [x] Idle with `SCADA_HVAC_ENABLE` = 0: unit off (yellow LED), `PUB_COMPRESSOR_RUNNING` = 0. **Reported as worked**
- [x] Target 150 then enable 1: setpoint reaches the ESP32, relay closes (blue LED), `PUB_COMPRESSOR_RUNNING` = 1. **Reported as worked**
- [x] Target 300: relay opens (green LED). **Reported as worked**
- [x] Enable 0 with the target below the room temperature: unit forced off. **Reported as worked**
- [ ] Sensor fault (unplug the DHT11 data wire, `HVAC_FAULT` latches, `PUB_TEMP_C` holds its last value): **not reported**
- Kali read test (measured, Oct 21): read 1 coils [0,0,0], holding [target 0, temp 193, humidity 60, heartbeat 135, reconnect 0]; read 2 (about 5 s later) coils [0,0,0], holding [0, 192, 60, 140, 0]. The heartbeat moved by 5, so the PLC polls the ESP32 live

## Results and problems

- Oct 21, 2026: Luis built R1-R8 himself, taught rung by rung (name, what it must do, concept, tied to the real variables). R1 and R2 were checked from a screenshot; the saved `main.ld` was checked for all 8 rungs.
- **Decided Oct 21: comms-loss design A.** Plain level copy for `HVAC_ENABLE` and a MOVE for the setpoint, 8 rungs, no timer. Fail-off stays in the ESP32 only (WiFi down more than 10 s opens the relay, as decided Oct 7). Rejected for now: B (a PLC `PUB_NODE_STALE` flag from a frozen heartbeat: it only reports, and Ignition can alarm on `PUB_HEARTBEAT`) and C (a SCADA heartbeat that forces the enable off when SCADA goes silent: needs Ignition and a new 0-block register, and anyone who can write the 0-block can fake it; revisit after Ignition exists). **Accepted risk:** if the PLC stops polling while the ESP32's WiFi stays up, the ESP32 keeps its last enable and setpoint; SCADA can only see it from a frozen heartbeat. Left for the attack-scenario session.
- Firmware read first (`FH_HVAC_Node.ino`): `HVAC_ENABLE` and `TARGET_TEMP_C` are levels, the ESP32 never writes them itself (only the serial commands `e` and `+/-` do), so OpenPLC rewriting them every poll cannot override any physical local state. No firmware change needed.
- **The PLC owns the unit from the first poll.** `SCADA_HVAC_ENABLE` and `SCADA_TARGET_TEMP_C` start at 0, so the ESP32's boot defaults (enabled, 24.0 C) are overwritten: the unit is off until SCADA sets them. With enable = 1 and the target left at 0, the firmware clamps the target to 10.0 C and the relay closes whenever the room is above 10.5 C. Set the target first, then the enable.
- **`TEMP_C` is not always a multiple of 10:** the Kali reads showed 193 and 192 (19.3 C, 19.2 C) although the firmware header says the DHT11 gives whole degrees. The PLC copies the value unchanged; the likely cause is the DHT library reporting the DHT11 decimal digit (not checked). The `FH_HVAC_Node.ino` header comment is inaccurate on this point.
- Slave Devices form fields checked in the OpenPLC v3 source on earlier nodes; the generic TCP type was used with holding-read size 0.
- Not tested: sensor-fault latch through the PLC, direct write from Kali to the ESP32 coil and holding register, WiFi-drop case (fail-off after 10 s and `RECONNECT_COUNT` + 1 through the PLC), unplug case, and one Kali read compared with the ESP32 serial output.

- Update Oct 21, 2026 (night): Luis confirmed, for all eight containers, that the compile logs, the Settings pages (S7 and EtherNet/IP off) and the Slave Devices listings are all fine. Reported by Luis, not pasted: the numbers (Polling Period, Timeout, listing addresses) and the log text were not recorded, and the Hardware page was not named separately.
