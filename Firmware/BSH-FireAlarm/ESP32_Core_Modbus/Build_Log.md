# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-FireAlarm)

This one isn't a single sensor — it's the ESP32 board itself joining the
project WiFi and standing up as a Modbus TCP slave. This is
`BSH_FireAlarm_PILOT_Bringup.md` **Phase B**, tracked here so it has the
same checklist/log format as the sensor folders.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] Arduino IDE has ESP32 board support installed, board set to
      "ESP32 Dev Module"
- [x] `ModbusIP_ESP8266` library installed via Library Manager
- [x] USB cable confirmed to have data wires (not charge-only) — board
      shows up in Device Manager

## Status
- [x] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_FireAlarm_Node.ino` (placeholders stay in the shared/Drive copy)
- [x] Flashed, Serial Monitor at 115200 baud shows
      `Connected. IP address: 192.168.50.12` (see Oct 6 note: SCT5 is a 192.168.50.x network)
- [x] From a laptop on the same subnet, a Modbus TCP client (a PowerShell script sending raw Modbus frames; modpoll
      was not used) can read Discrete Input 0 (`MANUAL_PULL_TRIGGERED`)
- [x] Same client can write Coil 0 (`ALARM_ACTIVATE`) = 1 and see the
      buzzer/LED respond exactly like the physical button does
- [x] Write Coil 0 = 0 and confirm the alarm stops (with button released)

- [x] `HEARTBEAT` (IR1) counts up ~1/second; `RECONNECT_COUNT` (IR2) reads 0 after boot, 1 after a Wi-Fi drop and recovery, and both go to 0 after a reset button press

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Phase B firmware written (WiFi static IP 192.168.1.12 + Modbus TCP via ModbusIP_ESP8266), same register map as the system doc. Fire Alarm is the pilot, so this is the template the other nodes reuse. Gas calibration interval raised from 0 to 100ms (2s real averaging). NOT yet flashed -- checklist above still open. |
| 2026-10-06 | **Phase B bench-confirmed.** Needed the ModbusIP_ESP8266 library installed in the Arduino IDE (compile error `ModbusIP_ESP8266.h: No such file or directory` until then; install the one named modbus-esp8266 by Emelianov). First flash joined SCT5 but was unreachable: the sketch had static IP 192.168.1.12 / gateway 192.168.1.1, while SCT5 is a **192.168.50.x network (gateway 192.168.50.1)**. Changed to 192.168.50.12 / 192.168.50.1 in this sketch only; the rest of the project is still on the 192.168.1.x plan until Luis decides. Decided the same day: Luis cannot change the router, so the whole project moved to 192.168.50.x. After that: `Test-NetConnection` port 502 succeeded; Input Registers read (gas, heartbeat counting, reconnects 0); Discrete Inputs read 0 at rest; Coil0 write turned the buzzer and red LED on, and off again; button, flame and gas inputs each changed their Discrete Input; heartbeat and reconnect counter behaved on a Wi-Fi drop. Also an earlier password typo (S3n!orC@p vs S3n!0rC@p, letter o vs zero) cost a flash. |

## Lessons Learned
- **Check the real network before trusting the IP plan.** The plan assumes 192.168.1.x; the lab router SCT5 is 192.168.50.x. The ESP32 joined fine at L2 and printed `Connected`, yet nobody could reach it. `ipconfig` on the laptop (default gateway) is the fastest check, and a laptop address on a different subnet from the node is the tell.
- **Serial `Connected` only means the board joined WiFi,** not that your laptop can reach it. Test with `Test-NetConnection <ip> -Port 502`.
- The laptop can be on 5 GHz while the ESP32 uses 2.4 GHz of the same router; that is fine.
- Letter `o` and digit `0` in the WiFi password look alike. Re-read it from the router, not from memory.

## Troubleshooting Tips
- `ModbusIP_ESP8266.h: No such file or directory`: install `modbus-esp8266` (Emelianov) from Tools > Manage Libraries, then re-upload.
- Serial says `Connected` but `Test-NetConnection` fails: compare the laptop's `ipconfig` subnet and gateway with the sketch's `ip` and `gateway` lines.
- A PowerShell test without installing anything: define a function that sends a raw Modbus TCP frame (see this system's Phase B notes in the Oct 6, 2026 session); functions 2, 4 and 5 cover discrete inputs, input registers and the alarm coil.
*(the pilot bring-up doc's Common Pitfall #1 — charge-only USB cables not
showing up in Device Manager — already lives one level up; add anything
new you actually hit here)*
