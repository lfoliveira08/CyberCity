# BSH-FireAlarm -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **slave device added, program not loaded yet** (files drafted Oct 11, 2026; container started Oct 13, 2026; ESP32 slave device added Oct 15, 2026).

## Phase C steps

- [x] Container `OpenPLC-BSH-FireAlarm` running at 192.168.50.112; Web UI reachable on port 8080 from the laptop (Oct 13, 2026)
- [x] Modbus server enabled on port 502 (Oct 15). Also: Hardware Layer = Blank; S7 and EtherNet/IP servers turned OFF (both were on by default).
- [x] ESP32 added as a slave device (192.168.50.12:502) with the sizes in `Register_Map.md` (Oct 15, 2026). Listed on the Slave Devices page as `BSH-FireAlarm-ESP32`, type TCP. **Slave ID entered: 0.** Accepted by the form and the device is listed; **not yet confirmed by live polling** -- the PLC was still Stopped with a Blank Program, so no Modbus request has been sent to the ESP32 yet. If polling fails later, retry with slave ID 1 and record the change here
- [ ] `Program.st` uploaded and compiled without errors
- [ ] Monitoring page matches the ESP32's live values; `HEARTBEAT` counts up and `RECONNECT_COUNT` is present
- [ ] A Modbus client on the laptop reads every `PUB_*` address in `Register_Map.md` and gets the right values
- [x] **Address convention VERIFIED (Oct 15, 2026).** The Slave Devices page states "Slave devices are attached to address 100 onward (i.e. %IX100.0, %IW100, %QX100.0, and %QW100)" and lists this device as DI `%IX100.0` to `%IX100.2`, DO `%QX100.0` to `%QX100.0`, AI `%IW100` to `%IW102`, AO `-`. This matches `Register_Map.md` section 3 exactly, so the drafted `Program.st` addresses are correct as written. Confirmed from the OpenPLC web UI, not yet from live values on the Monitoring page

## Tests

- [ ] Write coil SCADA_ALARM_ACTIVATE = 1 from a Modbus client: buzzer sounds and PUB_REMOTE_ONLY = 1.
- [ ] Press the pull station: MANUAL_PULL_TRIGGERED reads 1 and PUB_REMOTE_ONLY stays 0 (physical cause).
- [ ] Clear the command: alarm stops (unless a physical trigger is still active).

## Results and problems

- Oct 13, 2026: container started by hand: `sudo docker run -d --name OpenPLC-BSH-FireAlarm --hostname OpenPLC-BSH-FireAlarm --network cybercity_lan --ip 192.168.50.112 --restart unless-stopped -v openplc_bsh_firealarm:/docker_persistent openplc:v3`. No `--privileged`, no `-p`. .112 was free beforehand (no ping reply). The dashboard loaded from the laptop: Blank Program, Stopped.
- Decision Oct 13, 2026: the default OpenPLC login is kept, not changed.
- Still to do in the web UI: add the slave device, upload `Program.st`, start the PLC.
- Oct 15, 2026: ESP32 added as a slave device on the container at 192.168.50.112. The Slave Devices page lists `BSH-FireAlarm-ESP32`, type TCP, with DI `%IX100.0`-`%IX100.2`, DO `%QX100.0`, AI `%IW100`-`%IW102`, AO none -- matching `Register_Map.md` exactly. Slave ID 0 was used. **This settles the project-wide `%IX100.x` question:** the page's own banner says slave devices attach at address 100 onward. PLC still Stopped with Blank Program at this point, so nothing has been polled yet.
- Oct 15, 2026: Settings done -- Modbus server on (502), Hardware = Blank, S7 + EtherNet/IP OFF (default-on, turned off; applies to all 8). All 8 rungs built in the OpenPLC Editor (project `BSH-FireAlarm-ESP32/`); compiled output verified identical to `Program.st` (15 vars, same logic, same 15 located addresses). Not yet compiled/Started on the container. Deploy artifact = hand-written `Program.st` (v4 Editor does not export a loose `.st` for the v3 web UI).
