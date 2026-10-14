# FH-PowerBox -- PLC build log

Status: **container running, program not loaded yet** (files drafted Oct 11, 2026; container started Oct 14, 2026).

## Phase C steps

- [x] Container `OpenPLC-FH-PowerBox` running at 192.168.50.124; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [ ] Modbus server enabled on port 502 (OpenPLC Settings)
- [ ] ESP32 added as a slave device (192.168.50.24:502) with the sizes in `Register_Map.md`; slave ID that worked: ______
- [ ] `Program.st` uploaded and compiled without errors
- [ ] Monitoring page matches the ESP32's live values; `HEARTBEAT` counts up and `RECONNECT_COUNT` is present
- [ ] A Modbus client on the laptop reads every `PUB_*` address in `Register_Map.md` and gets the right values
- [ ] **Address convention verified:** slave-block addresses (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) confirmed or corrected: ______

## Tests

- [ ] Write SCADA_LIGHTS_ON_CMD = 1: light turns on and PUB_LIGHTS_STATUS = 1, mismatch flag 0.
- [ ] Press the local lights button so the light flips: PUB_LIGHTS_MISMATCH = 1 (command and actual now disagree).
- [ ] Write the command again (fresh write): hub regains authority and the mismatch clears.
- [ ] Same three checks for the fan.

## Results and problems

(none yet)
