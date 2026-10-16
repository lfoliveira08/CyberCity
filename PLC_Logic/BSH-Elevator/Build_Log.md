# BSH-Elevator -- PLC build log

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Status: **container running, program not loaded yet** (files drafted Oct 11, 2026; container started Oct 14, 2026).

## Phase C steps

- [x] Container `OpenPLC-BSH-Elevator` running at 192.168.50.111; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [ ] Modbus server enabled on port 502 (OpenPLC Settings)
- [ ] ESP32 added as a slave device (192.168.50.11:502) with the sizes in `Register_Map.md`; slave ID that worked: ______
- [ ] `Program.st` uploaded and compiled without errors
- [ ] Monitoring page matches the ESP32's live values; `HEARTBEAT` counts up and `RECONNECT_COUNT` is present
- [ ] A Modbus client on the laptop reads every `PUB_*` address in `Register_Map.md` and gets the right values
- [ ] **Address convention verified:** slave-block addresses (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) confirmed or corrected: ______

## Tests

- [ ] Write SCADA_CALL_FLOOR_2 = 1: elevator moves to floor 2 and PUB_ELEVATOR_MOVING pulses.
- [ ] Write SCADA_ELEVATOR_DISABLE = 1: elevator locks out (fault-injection target).
- [ ] PUB_CURRENT_FLOOR follows the real floor.

## Results and problems

(none yet)
