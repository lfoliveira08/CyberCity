# ESP32 Board + WiFi + Modbus TCP -- Build Log (FH-PowerBox)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Board bring-up and Modbus TCP verification (Phase B), mirrored from
`BSH-PowerBox/ESP32_Core_Modbus/Build_Log.md`. FH-PowerBox was built ahead of
the BSH build order at Luis's request (Oct 6, 2026).

## Prerequisites
- [x] `ModbusIP_ESP8266` library installed in the Arduino IDE
- [x] `ping 192.168.50.24` times out (address is free on the SCT5 network)
- [x] Both relay boards' polarity checked (BSH's were active-HIGH; FH's may differ)

## Status
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.24` (not .14)
- [x] Both outputs are OFF at boot
- [x] Coil 0 (`LIGHTS_ON_CMD`) and Coil 1 (`FAN_CMD`) write turn the light and fan on and off; DI0/DI1 follow
- [x] A local button flips its output without any laptop command; a fresh coil write takes control back
- [x] With WiFi off, both outputs go OFF after ~5s and the buttons still work
- [x] `HEARTBEAT` (IR0) counts up ~1/second; `RECONNECT_COUNT` (IR1) reads 0 after boot and 1 after a WiFi drop and recovery
- [x] BSH-PowerBox (.14) and FH-PowerBox (.24) up together, each controlled independently (confirmed by Luis)

## Date log
| Date | What happened |
|---|---|
| 2026-10-21 | **Authority rule changed in `FH_PowerBox_Node.ino`** (same change as BSH-PowerBox, made ahead of FH Phase C because OpenPLC rewrites the coils every ~100 ms and undid local button presses on BSH). Luis reported flashing it; NOT bench-tested on FH (no PLC on this node yet, nothing reported). Old sketch: `Archive/FH_PowerBox_Node_PhaseB_every-write-wins.ino`. |
| 2026-10-07 | **Phase B bench-confirmed at 192.168.50.24.** Flashed; Serial showed `Connected. IP address: 192.168.50.24`. Outputs OFF at boot with both flags at `false`, so FH's relay boards are active-HIGH like BSH's (verified on this hardware, not assumed). Laptop coil writes drive the lights and fan and the status DIs follow; local buttons override immediately; a fresh coil write takes control back; 5 s WiFi-link watchdog forces both OFF with buttons still working; HEARTBEAT counts up and RECONNECT_COUNT reads 0 after boot and 1 after a WiFi drop. Tested from PowerShell with raw Modbus frames. Both PowerBoxes online together was later confirmed by Luis (2026-10-09). |
| 2026-10-06 | Phase B firmware written as a mirror of BSH-PowerBox's confirmed sketch, static IP 192.168.50.24. NOT yet flashed. FH's standalone Phase A was never logged separately; relay polarity on the FH boards is unchecked. |

## Lessons Learned
- FH's relay boards turned out active-HIGH, same as BSH's, but this was confirmed by the boot-state check, not carried over.
- Relay polarity belongs to the physical board; do not carry BSH's active-HIGH result over without checking.

## Troubleshooting Tips
- Output ON at boot, or commands do the opposite: flip `RELAY_LIGHTS_ACTIVE_LOW` / `RELAY_FAN_ACTIVE_LOW` and re-flash.
- Serial says `.14`: wrong sketch flashed.

## 2026-10-28 (evening) -- power-rails sketch flashed and confirmed on WiFi
- Luis flashed `FH_PowerBox_Node.ino` (power rails, redesigned the same day -- see `Firmware/FH_PowerBox_Bringup_Checklist.md`). Reported: WiFi joined at 192.168.50.24 and the heartbeat is ticking (no screenshot/values pasted, not seen by the agent). Not yet shown: the status line's rail states (expect all CUT at boot, option A), a rail toggled from Serial, or the OpenPLC side.
