# FH-Door -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **program running; Kali read test passed and the functional tests were reported as worked on real hardware (Oct 21, 2026)**. Still open: the compile log, the Settings values and the S7 / EtherNet/IP switches were not shown; no test values were reported (files drafted Oct 11, 2026; container started Oct 14; ladder built in the Editor, uploaded and run Oct 21).

## Phase C steps

- [x] Container `OpenPLC-FH-Door` running at 192.168.50.123; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [~] Modbus server enabled on port 502 (OpenPLC Settings). The Kali read reached port 502 on this container (Oct 21), so the server is on. **Not shown:** the Settings page itself (S7 and EtherNet/IP off, Hardware = Blank) and the runtime log with the `stop_*` lines
- [x] ESP32 added as a slave device (192.168.50.23:502) with the sizes in `Register_Map.md`; **slave ID that worked: 0** (Oct 21, 2026; proven by the heartbeat moving in the Kali read). Device type TCP (the generic one, not the `ESP32` preset)
- [x] `Program.st` uploaded and compiled (Oct 21, 2026; the program runs and the PLC polls the ESP32, so it compiled). **The compile log was not shown**
- [x] Monitoring page works with the PLC running; `HEARTBEAT` counting and `RECONNECT_COUNT` 0 are confirmed through the Kali read (Luis did not paste Monitoring screenshots)
- [x] A Modbus client reads every `PUB_*` address in `Register_Map.md` (Oct 21, 2026). **Done from the Kali VM (192.168.50.40), not the laptop**, with a read-only stdlib script (`~/fhdoor_read.py`, on the VM only; unit ID 1, FC 1 coils 0-5, FC 3 holding registers 0-1, two reads 5 s apart). Read 1 (21:16:12): coils 0,0,0,1,1,0; `PUB_HEARTBEAT` 156; `PUB_RECONNECT_COUNT` 0. Read 2 (21:16:17): same coils; heartbeat 161. **Heartbeat +5 in 5 s: the PLC polls the ESP32 live and publishes it.** `PUB_IR_DETECTED` and `PUB_DOOR_POSITION` read 1 at idle, the same as BSH-Door; not compared with the ESP32 serial output
- [x] **Address convention verified (Oct 21, 2026):** the Slave Devices page of this container lists `FH-Door-ESP32`, type TCP, DI `%IX100.0` to `%IX100.2`, DO `%QX100.0` to `%QX100.1`, AI `%IW100` to `%IW101`, AO none (Luis's screenshot). This matches `Register_Map.md` and `Program.st` exactly

## Tests

Luis ran the functional tests on Oct 21 (first on the ESP32 itself, then through the PLC with values forced on the Monitoring page) and reported "worked well and as expected" and "everything worked as expected". No values, LED colours or step details were reported, so these are logged as **reported as worked**, not measured:

- [x] Write SCADA_DOOR_OPEN_CMD = 1 with no card: gate opens and PUB_REMOTE_OPEN_NO_CARD = 1. **Reported as worked**
- [x] Short pulse of SCADA_DOOR_OPEN_CMD (1 then 0): gate opens, holds and closes by itself. **Reported as worked**
- [x] Write SCADA_DOOR_CLOSE_CMD = 1: gate closes. **Reported as worked**
- [x] Valid FH card: ACCESS_GRANTED pulses and the flag stays 0. **Reported as worked**
- [x] Wrong card (BSH card) denied. **Reported as worked**
- [x] IR_DETECTED and DOOR_POSITION follow the sensors. **Reported as worked**
- Kali read test (measured, Oct 21): see above.

## Results and problems

- Oct 21, 2026: Luis built all 8 rungs himself in the OpenPLC Editor (project `FH-Door-ESP32/`, saved with Save Project at 21:07 local), taught with the variable list and the rung diagram. The saved `main.ld` was checked: 15 variables with the addresses in `Register_Map.md`, R1-R8 wired left rail to right rail (R1/R2 contact-coil, R3 contact + inverted contact on `ACCESS_GRANTED` + coil, R4-R6 contact-coil, R7/R8 MOVE with EN from the left rail and ENO to the right rail), one writer per coil and MOVE output. Task interval **`T#20ms`** (cyclic, priority 1): the value saved in Luis's Editor project, not copied from BSH-Door (`T#100ms`). The R3 rung comment has trailing spaces (cosmetic).
- `Program.st` regenerated from that saved project (replaces the Oct 11 draft, no backup kept; ASCII; all variables located so one VAR block).
- **FH-Door is a mirror of BSH-Door, verified Oct 21 by diffing the sketches:** `FH_Door_Node.ino` and `BSH_Door_Node.ino` differ only in the IP (.23 vs .13), the authorized card (FH `3D D2 C6 62`, BSH `B3 12 87 2D`) and banner/comment text. Registers, callbacks, gate state machine, watchdog and counters are identical, and `Register_Map.md` differs from BSH-Door's only in names, IPs and status notes.
- **Decided Oct 21: design A, plain level copy for R1/R2, same as BSH-Door** (Luis: "keep like BSH"). Firmware read first: both command coils are momentary (the callback acts only on a write of 1 and the loop clears the coil), the PLC rewrites them about every 100 ms, so a held 1 re-arms every poll. Held open: the hold timer restarts each poll, the gate stays open and `pollCard()` returns (reader blocked), as seen on BSH-Door. Held close (read from the code, **not tested**): a card-opened gate would be sent back to closed on the next poll, which also blocks card access. Both held: open wins (close is handled first, then open). The coils start at 0 and a first-poll write of 0 does nothing, so no boot default is overwritten. Rejected: C (a 300 ms `TP` pulse like the Elevator; would stop a held bit pinning the gate but FH would no longer mirror BSH) and B (a bare `R_TRIG`; a one-scan pulse can be missed because the master writes then sleeps a full Polling Period). **Accepted risk:** a held open bit keeps the gate open and disables the card reader, and the 5 s WiFi watchdog does not cover it. Ignition: momentary open/close buttons (300 ms or longer), and log `PUB_REMOTE_OPEN_NO_CARD` on its rising edge.
- `ACCESS_GRANTED` is a pulse of about 2 s: Ignition must poll `PUB_ACCESS_GRANTED` well under 2 s. `PUB_REMOTE_OPEN_NO_CARD` only sees commands that go through the PLC; a direct write to the ESP32 coil at 192.168.50.23:502 bypasses it.
- Slave Devices form fields checked in the OpenPLC v3 source (`webserver.py`, `pages.py`) before the steps were given: Device Type "Generic Modbus TCP Device" (value `TCP`), Slave ID, IP, port, start and size per block, Save device.

## Open

- [ ] Compile log, Settings (Modbus on 502, S7 and EtherNet/IP off, Hardware = Blank), Polling Period and Timeout not shown for this container
- [ ] Test details not reported (Monitoring values, LED colours, whether the gate closed on release)
- [ ] Direct write from Kali to the ESP32 coils (192.168.50.23:502), WiFi-drop case (watchdog closes the gate after 5 s, `RECONNECT_COUNT` + 1 without a reboot), unplug case, and one Kali read compared with the ESP32 serial output: attack-scenario session
- [ ] Watch for a repeat of the BSH-Door RFID reader outage (reader stopped for a while with no known cause); not reported on FH-Door
- [ ] Held close bit blocking card access: read from the code, untested

- Update Oct 21, 2026 (night): Luis confirmed, for all eight containers, that the compile logs, the Settings pages (S7 and EtherNet/IP off) and the Slave Devices listings are all fine. Reported by Luis, not pasted: the numbers (Polling Period, Timeout, listing addresses) and the log text were not recorded, and the Hardware page was not named separately.
