# BSH-PowerBox -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **program running and the functional tests passed on real hardware (Oct 21, 2026)**, after a firmware change on the ESP32 (see "Decision" below). Still open: compile log not kept, Polling Period/Timeout/Hardware not seen on this container, WiFi-drop and direct-write checks not done (files drafted Oct 11, 2026; container started Oct 14; ladder built in the Editor and uploaded Oct 21).

## Phase C steps

- [x] Container `OpenPLC-BSH-PowerBox` running at 192.168.50.114; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [x] Modbus server enabled on port 502 (Oct 21, 2026). The runtime log at start shows `stop_dnp3()`, `stop_enip()`, `stop_pstorage()` and `stop_snap7()` and `Server: Listening on port 502`, so DNP3, EtherNet/IP, S7 and persistent storage are off. Hardware = Blank was not read back
- [x] ESP32 added as a slave device (192.168.50.14:502) with the sizes in `Register_Map.md`; **slave ID that worked: 0** (Oct 21, 2026; the log shows `Connected to MB device BSH-PowerBox-ESP32` and the heartbeat then counted live). Device type TCP (the generic one, not the `ESP32` preset)
- [x] `Program.st` uploaded and compiled without errors (Oct 21, 2026; the PLC ran and Monitoring worked; Luis did not paste the compile log, so its text was not recorded)
- [x] Monitoring page matches the ESP32's live values (Oct 21, 2026): `HEARTBEAT` counts up (Luis: yes); at idle the four bits `PUB_LIGHTS_STATUS`, `PUB_FAN_STATUS`, `PUB_LIGHTS_MISMATCH` and `PUB_FAN_MISMATCH` read 0
- [x] A Modbus client reads every `PUB_*` address in `Register_Map.md` (Oct 21, 2026). **Done from the Kali VM (192.168.50.40)** with a read-only stdlib script (`~/powerbox_read.py`, on the VM only): coils 0-5 by FC 1 and holding registers 0-1 by FC 3, unit ID 1, two reads 5 s apart. Read 1 (15:01:32): coils 0,0,0,0,0,0; heartbeat 881; reconnect count 0. Read 2 (15:01:38): same coils; heartbeat 887. **Heartbeat +6 in about 6 s: the PLC polls the ESP32 live and publishes it; port 502 was open.** Not compared with the ESP32 serial output
- [x] **Address convention verified (Oct 21, 2026):** the Slave Devices page lists `BSH-PowerBox-ESP32`, type TCP, DI `%IX100.0` to `%IX100.1`, DO `%QX100.0` to `%QX100.1`, AI `%IW100` to `%IW101`, AO none. Matches `Register_Map.md` and `Program.st` exactly
- [ ] Settings page: Polling Period and Timeout. Luis said "same as the other ones" (100 ms and 1000 ms on BSH-Door and, reported, BSH-FireAlarm); the page was not seen for this container

## Tests (values forced on the Monitoring page; physical buttons pressed by Luis)

Part 1, with the Oct 6 firmware (every coil write counts as a hub command):
- [x] `SCADA_LIGHTS_ON_CMD` forced to 1: the light turns on and stays on; `PUB_LIGHTS_STATUS` = 1, `PUB_LIGHTS_MISMATCH` = 0 (reported as matching the prediction).
- [x] Local lights button pressed with the force on: the light flickered off and came back (the PLC rewrites 1 every poll). Force released: the light went off.
- [x] Command at 0 and the light off: pressing the button turned the light on for an instant and it went back off (the PLC rewrites 0 every poll). **This showed that, with the PLC running, the local button could not keep the light on.**

Part 2, after the firmware change (see Decision) was flashed:
- [x] Lights, command at 0: button press -> the light stays on, status and mismatch as expected (Luis; the exact status/mismatch values were reported only as "as expected"); second press -> off; force command to 1 -> on; button press with the force on -> stays off with the mismatch up; force released -> follows. All reported as passed.
- [x] Fan: the same sequence, reported by Luis as "ran as expected" (per-step values not reported).
- [ ] Not done: the direct-write check from Kali to the ESP32 coils (expected: a different value than the PLC's last write takes effect, then the PLC's next write puts it back within about 100 ms), the WiFi-drop case (watchdog forces both outputs OFF after 5 s, `RECONNECT_COUNT` + 1), the ESP32-unplug case, and a comparison of one Kali read with the ESP32 serial output.

## Decision (Oct 21, 2026): A, then the firmware change

- Luis chose **option A**: plain level copy for R1/R2 (like BSH-Door), no timers, 8 rungs, `T#20ms`. Rejected: B (a tracking latch in the PLC; the master writes the previous scan's buffer, so it would likely lose button presses or flap) and a PLC-side stretch of the mismatch flag (`TP`).
- Bench result with A: the PLC rewrote the command on every poll (read in `modbus_master.cpp`: `modbus_write_bits` of the whole coil block every cycle, then a sleep of the polling period), so the Oct 6 firmware let the PLC undo every button press within about 100 ms. Luis expected the button to stick and the status to show it.
- **Change (option C, Luis's decision):** in `BSH_PowerBox_Node.ino` a coil write only counts as a hub command when its value differs from the last value the hub wrote; the "last hub value" is forgotten after a WiFi rejoin. The ladder, `Program.st` and the container settings did not change. The old sketch is archived as `Archive/BSH_PowerBox_Node_PhaseB_every-write-wins.ino`. The same change was made in `FH_PowerBox_Node.ino` ahead of FH Phase C (not tested on FH). Luis reported flashing both ESP32 boards.
- Consequences: a button press sticks and `PUB_*_MISMATCH` stays 1 until SCADA sends a different command; SCADA cannot re-force the same value (it has to send the opposite value and then the original); a direct write from an attacker that differs from the PLC's last value still takes effect for about one poll and is then undone. Held SCADA bits are harmless on this node (they no longer override the button), unlike the BSH-Door.

## Results and problems

- Oct 21, 2026: Luis built all 8 rungs in the OpenPLC Editor (project `BSH-PowerBox-ESP32/`, saved with Save Project), guided rung by rung against the real variables, and checked against the saved `main.ld`: 14 variables with the addresses in `Register_Map.md`, and R1-R8 wired left rail to right rail (R1/R2/R5/R6 contact-coil; R3/R4 a parallel block with two series branches of one normal and one inverted contact each into the mismatch coil; R7/R8 MOVE with EN from the left rail and ENO to the right rail). Task `task0`, Cyclic, **`T#20ms`**, the value saved in the Editor project (not copied from another system). `Program.st` was regenerated from that build (it replaces the Oct 11 hand draft).
- The mismatch flags are computed from the SCADA command and the ESP32 status, so a stale SCADA command with a button-changed state keeps the flag at 1 after the firmware change; before it, the flag lasted about one poll.
- Not recorded: the first Start log text beyond the lines shown above (`Device ... is disconnected. Attempting to reconnect...` then `Connected to MB device`; this is the normal first-poll sequence, no timeout loop).

- Update Oct 21, 2026 (night): Luis confirmed, for all eight containers, that the compile logs, the Settings pages (S7 and EtherNet/IP off) and the Slave Devices listings are all fine. Reported by Luis, not pasted: the numbers (Polling Period, Timeout, listing addresses) and the log text were not recorded, and the Hardware page was not named separately.
