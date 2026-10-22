# FH-FireAlarm -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **program running; the four functional tests were reported as worked and the Kali read test passed (Oct 21, 2026)**. Open: the compile log, Slave Devices listing and Settings values were not shown (files drafted Oct 11, 2026; container started Oct 14; ladder built in the Editor, `Program.st` regenerated, program uploaded and run Oct 21).

## Phase C steps

- [x] Container `OpenPLC-FH-FireAlarm` running at 192.168.50.122; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [x] Modbus server answering on port 502 (proven Oct 21 by the Kali read). **Not shown:** the Settings page (Modbus checkbox, S7 and EtherNet/IP off, Hardware = Blank), Polling Period and Timeout. The PLC log lines `stop_dnp3` / `stop_enip` / `stop_snap7` were not seen either.
- [x] ESP32 added as a slave device (192.168.50.22:502), generic TCP, slave ID 0, sizes from `Register_Map.md` (reported by Luis Oct 21). **Not shown:** the Slave Devices listing, so the ranges were not compared line by line with the draft. **Slave ID 0 CONFIRMED by live polling Oct 21, 2026:** `PUB_HEARTBEAT` read 149 then 154 from Kali about 5 s apart.
- [x] Ladder built by Luis in the OpenPLC Editor (project `FH-FireAlarm-ESP32/`, saved Oct 21): 8 rungs R1-R8, task `T#20ms` (his Editor setting, cyclic, priority 1). Saved `main.ld` read and checked: 15 variables with the expected addresses; R1 plain contact to coil; R2 one normal contact and three inverted contacts in series to `PUB_REMOTE_ONLY` (first build had the coil on `ALARM_ACTIVATE`, caught from the screenshot and fixed before saving); R3-R5 contact to coil; R6-R8 MOVE with EN from the left rail
- [x] `Program.st` regenerated from the saved project (replaces the Oct 11 hand draft, no backup kept), uploaded by Luis and run. **The compile log was not pasted.** The program started and polled the ESP32, so it compiled
- [x] Monitoring page values: `HEARTBEAT` counting and `RECONNECT_COUNT` 0 are confirmed through the Kali read of `PUB_HEARTBEAT` / `PUB_RECONNECT_COUNT` (Luis did not paste Monitoring screenshots)
- [x] A Modbus client reads the `PUB_*` addresses (Oct 21, 2026), from the Kali VM (192.168.50.40), read-only stdlib script `~/firealarm_read.py` on Kali (unit ID 1, FC 1 coils 0-4, FC 3 holding registers 0-2)
- [ ] **Address convention on this container:** `%IX100.x` / `%QX100.x` / `%IW100+` verified on the BSH containers; on FH it is supported by the working program (heartbeat and pull-station tests), but the Slave Devices listing was not shown

## Tests

- [x] `SCADA_ALARM_ACTIVATE` = 1 (forced on the Monitoring page): buzzer sounds, `ALARM_ACTIVATE` and `PUB_REMOTE_ONLY` go to 1. **Reported as worked** (no values reported)
- [x] Pull station pressed with the remote command on: `PUB_MANUAL_PULL_TRIGGERED` = 1 and `PUB_REMOTE_ONLY` drops to 0. **Reported as worked**
- [x] Clear the command: alarm stops. **Reported as worked**
- [x] Pull station alone (command 0): buzzer sounds locally, `ALARM_ACTIVATE` stays 0, `PUB_REMOTE_ONLY` stays 0. **Reported as worked**
- [ ] Flame and gas triggers not tested (optional test 5 not run)
- Kali read test (measured, Oct 21): read 1 coils [0,0,0,0,0], gas 0, heartbeat 149, reconnect 0; read 2 (about 5 s later) coils [0,0,0,0,0], gas 0, heartbeat 154, reconnect 0. The heartbeat moved, so the PLC polls the ESP32 live

## Results and problems

- Oct 21, 2026: Luis built R1-R8 himself, taught rung by rung (name, what it must do, concept, tied to the real variables); the teaching was also meant for Luis to repeat to his partners. The R2 coil mistake (wrong coil would have made two rungs write `ALARM_ACTIVATE`) was found from the screenshot and fixed.
- Firmware read first (`FH_FireAlarm_Node.ino`): the alarm is `button OR flame OR gas OR Coil0`, the coil is a level, the ESP32 never writes the coil itself, so OpenPLC rewriting it every poll cannot override local state. **Decided Oct 21: plain level copy for R1, same as BSH-FireAlarm** (a held SCADA bit keeps the buzzer on until SCADA writes 0; it cannot silence a real cause). No firmware change needed.
- **Gas reading was 0 on both Kali reads. Closed by Luis's report Oct 21 ("the gas is fine"); no new reading and no explanation of the 0 were given, so this is a report, not a measurement.** `Firmware/WORKFLOW.md` records an FH clean-air baseline of about 2700. A flat 0 suggests the MQ-2 analog line is not reaching GPIO 34 or the sensor is unpowered. Not investigated yet. Check `GAS_RAW_VALUE` on the Monitoring page and the ESP32 serial output (baseline line at boot).
- Not tested: direct write from Kali to the ESP32 coil, WiFi-drop case, unplug case, flame and gas triggers, one Kali read compared with the ESP32 serial output.

- Oct 21, 2026 (later): Luis said the gas sensor is fine. The 0 on both Kali reads against the 2700 baseline in `Firmware/WORKFLOW.md` was not re-read or explained; if the gas alert ever misbehaves, start with `GAS_RAW_VALUE` on the Monitoring page and the baseline line in the ESP32 serial output.

- Update Oct 21, 2026 (night): Luis confirmed, for all eight containers, that the compile logs, the Settings pages (S7 and EtherNet/IP off) and the Slave Devices listings are all fine. Reported by Luis, not pasted: the numbers (Polling Period, Timeout, listing addresses) and the log text were not recorded, and the Hardware page was not named separately.
