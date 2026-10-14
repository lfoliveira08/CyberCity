# BSH-FireAlarm -- PLC build log

Status: **container running, program not loaded yet** (files drafted Oct 11, 2026; container started Oct 13, 2026).

## Phase C steps

- [x] Container `OpenPLC-BSH-FireAlarm` running at 192.168.50.112; Web UI reachable on port 8080 from the laptop (Oct 13, 2026)
- [ ] Modbus server enabled on port 502 (OpenPLC Settings)
- [ ] ESP32 added as a slave device (192.168.50.12:502) with the sizes in `Register_Map.md`; slave ID that worked: ______
- [ ] `Program.st` uploaded and compiled without errors
- [ ] Monitoring page matches the ESP32's live values; `HEARTBEAT` counts up and `RECONNECT_COUNT` is present
- [ ] A Modbus client on the laptop reads every `PUB_*` address in `Register_Map.md` and gets the right values
- [ ] **Address convention verified:** slave-block addresses (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) confirmed or corrected: ______

## Tests

- [ ] Write coil SCADA_ALARM_ACTIVATE = 1 from a Modbus client: buzzer sounds and PUB_REMOTE_ONLY = 1.
- [ ] Press the pull station: MANUAL_PULL_TRIGGERED reads 1 and PUB_REMOTE_ONLY stays 0 (physical cause).
- [ ] Clear the command: alarm stops (unless a physical trigger is still active).

## Results and problems

- Oct 13, 2026: container started by hand: `sudo docker run -d --name OpenPLC-BSH-FireAlarm --hostname OpenPLC-BSH-FireAlarm --network cybercity_lan --ip 192.168.50.112 --restart unless-stopped -v openplc_bsh_firealarm:/docker_persistent openplc:v3`. No `--privileged`, no `-p`. .112 was free beforehand (no ping reply). The dashboard loaded from the laptop: Blank Program, Stopped.
- Decision Oct 13, 2026: the default OpenPLC login is kept, not changed.
- Still to do in the web UI: add the slave device, upload `Program.st`, start the PLC.
