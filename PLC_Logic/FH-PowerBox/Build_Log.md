# FH-PowerBox -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **program running; Kali read test passed; functional tests reported as worked, with no values given (Oct 21, 2026)**. Compile log, Settings (S7 and EtherNet/IP off) and the Slave Devices listing were confirmed fine by Luis later that night (reported, not pasted). Still open: the per-step test values (files drafted Oct 11, 2026; container started Oct 14; ladder built in the Editor and uploaded Oct 21).

## Phase C steps

- [x] Container `OpenPLC-FH-PowerBox` running at 192.168.50.124; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [x] Modbus server answers on port 502 (Oct 21, 2026: the Kali read connected). S7 and EtherNet/IP off: **confirmed by Luis** (night of Oct 21, not pasted); Hardware = Blank not named separately
- [x] ESP32 added as a slave device (192.168.50.24:502) with the sizes in `Register_Map.md`; **slave ID that worked: 0** (Oct 21, 2026; proven by the moving heartbeat in the Kali read). Luis added the device; the Slave Devices listing was not pasted, so the sizes entered are not read back
- [x] `Program.st` uploaded and run (Oct 21, 2026; the Kali read returned the program's `PUB_*` values live). Luis later confirmed the compile was fine; the log text was not pasted or recorded
- [x] A Modbus client reads every `PUB_*` address in `Register_Map.md` (Oct 21, 2026). **Done from the Kali VM (192.168.50.40)** with a read-only stdlib script (`~/fhpowerbox_read.py`, on the VM only; written and checked against a stand-in server first): unit ID 1, FC 1 coils 0-5, FC 3 holding registers 0-1, two reads 5 s apart. Read 1 (21:41:52): coils 0,0,0,0,0,0; `PUB_HEARTBEAT` 536; `PUB_RECONNECT_COUNT` 0. Read 2 (21:41:57): same coils; heartbeat 541. **Heartbeat +5 in 5 s: the PLC polls the ESP32 live and publishes it; slave ID 0 proven.** Not compared with the ESP32 serial output
- [ ] Monitoring page vs the ESP32 live values: no values reported
- [x] **Address block listing:** confirmed fine by Luis (not pasted); expected DI `%IX100.0`-`.1`, coils `%QX100.0`-`.1`, IR `%IW100`-`101` (same as BSH-PowerBox). The moving `PUB_HEARTBEAT` shows `%IW100` is read
- [x] Settings: Polling Period and Timeout. Luis said they are the same as BSH-FireAlarm (default 100 ms and 1000 ms were read off BSH-Door); **reported, numbers not seen on this container**

## Tests (values forced on the Monitoring page; physical buttons pressed by Luis)

Luis reported "working fine" for the ESP32 test and for the functional tests, and did not give per-step values when asked. Everything below is **reported as worked, not measured**; the outcomes written after "expected" are the test plan, not readings Luis gave.

- [x] Lights command = 1 (expected: light on, `PUB_LIGHTS_STATUS` = 1, mismatch 0) -- reported as worked.
- [x] Local lights button with the command held at 1 (expected: the light stays off, mismatch 1) -- reported as worked. This is the check of the Oct 21 firmware rule on FH; the serial lines (only the button line, no repeated `Hub wrote`) were not reported.
- [x] Command 0 then 1 (expected: the light follows) -- reported as worked.
- [x] Button with the command at 0 (expected: the light stays on, mismatch 1; command 1 then clears it) -- reported as worked.
- [x] Same sequence for the fan (reported as worked).
- [ ] Not done: the direct-write check from Kali to the ESP32 coils, the WiFi-drop case (watchdog forces both outputs OFF after 5 s, `RECONNECT_COUNT` + 1), the ESP32-unplug case, and a comparison of one Kali read with the ESP32 serial output. They belong to the attack-scenario session.

## Decision (Oct 21, 2026): option A, plain level copy

- FH-PowerBox is a mirror of BSH-PowerBox: `diff` of `BSH_PowerBox_Node.ino` against `FH_PowerBox_Node.ino` shows only the IP (.24 vs .14), the banner text and header comments. The code, including the Oct 21 rule (a coil write counts as a hub command only when its value differs from the last hub value; forgotten after a WiFi rejoin), is identical. `Register_Map.md` differs from BSH's only in names, IPs and status notes.
- Luis chose **option A**: R1/R2 plain level copies, R3/R4 XOR mismatch flags, R5-R8 publish copies and MOVEs, 8 rungs, no timers. Rejected: B (a TON on the mismatch flag; adds timers and would differ from BSH).
- Consequences (read from the code): a button press sticks and `PUB_*_MISMATCH` stays 1 until SCADA sends a different command; SCADA cannot re-force the same value (send the opposite value, then the original), so Ignition buttons for this node must write a changed value. On the first poll after a PLC start or a WiFi rejoin the PLC writes the `SCADA_*` values (0 until SCADA sets them); because the last hub value is -1 then, that first write always counts, so a light or fan that was ON from the local button is turned OFF once (read from the code, not tested).

## Results and problems

- Oct 21, 2026: Luis built all 8 rungs in the OpenPLC Editor (project `FH-PowerBox-ESP32/`, saved with Save Project) from the rung list given in chat, and the saved `main.ld` was checked: 14 variables with the addresses in `Register_Map.md`, and R1-R8 wired left rail to right rail (R1/R2/R5/R6 contact-coil; R3/R4 a parallel block with two series branches of one normal and one inverted contact each into the mismatch coil; R7/R8 MOVE with EN from the left rail and ENO to the right rail). One writer per coil and MOVE output. Task `task0`, Cyclic, **`T#20ms`**, priority 1, the value saved in the Editor project (not copied from another system). `Program.st` was regenerated from that build (it replaces the Oct 11 hand draft; body identical to BSH-PowerBox's, header and names aside).
- The header of `FH_PowerBox_Node.ino` still says the Oct 21 rule was "NOT yet flashed or bench-tested". Luis reported flashing it and the tests as working, so the comment is out of date; not edited (firmware source).
- Update Oct 21, 2026 (night): Luis confirmed, for all eight containers, that the compile logs, the Settings pages (S7 and EtherNet/IP off) and the Slave Devices listings are all fine. Reported by Luis, not pasted: the numbers (Polling Period, Timeout, listing addresses) and the log text were not recorded, and the Hardware page was not named separately.
