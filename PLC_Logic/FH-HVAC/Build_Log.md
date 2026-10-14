# FH-HVAC -- PLC build log

Status: **container running, program not loaded yet** (files drafted Oct 11, 2026; container started Oct 14, 2026).

## Phase C steps

- [x] Container `OpenPLC-FH-HVAC` running at 192.168.50.121; Web UI reachable on port 8080 from the laptop (Oct 14, 2026: HTTP 200 from the laptop)
- [ ] Modbus server enabled on port 502 (OpenPLC Settings)
- [ ] ESP32 added as a slave device (192.168.50.21:502) with the sizes in `Register_Map.md`; slave ID that worked: ______
- [ ] `Program.st` uploaded and compiled without errors
- [ ] Monitoring page matches the ESP32's live values; `HEARTBEAT` counts up and `RECONNECT_COUNT` is present
- [ ] A Modbus client on the laptop reads every `PUB_*` address in `Register_Map.md` and gets the right values
- [ ] **Address convention verified:** slave-block addresses (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) confirmed or corrected: ______

## Tests

- [ ] Write SCADA_HVAC_ENABLE = 1 and SCADA_TARGET_TEMP_C = a value: the setpoint reaches the ESP32 (check ESP32 serial).
- [ ] PUB_TEMP_C tracks the real temperature; PUB_COMPRESSOR_RUNNING follows the relay.
- [ ] Write SCADA_HVAC_ENABLE = 0: unit is forced off regardless of setpoint.

## Results and problems

(none yet)

## Open decision

Comms-loss safe state for HVAC_ENABLE (see FH_Systems/01_FH-HVAC.md): decide fail-off vs fail-open, then add a rung that watches PUB_HEARTBEAT and forces SCADA-side enable to the chosen state.
