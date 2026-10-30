# ESP32 Board + WiFi + Modbus TCP — Build Log (BSH-PowerBox)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Board bring-up and Modbus TCP verification (Phase B) — not tied to one
sensor, tracked separately from `Fan_Relay/` and `Lights_Relay/`.

## Prerequisites
- [x] Dedicated project WiFi AP already broadcasting
- [x] `ModbusIP_ESP8266` library installed
- [x] Both `Fan_Relay/` and `Lights_Relay/` bench-tested standalone
      first (per their own Build_Log.md checklists) — don't debug WiFi/
      Modbus and hardware problems at the same time

## Status
- [x] Real WiFi SSID/password filled into a **local-only** copy of
      `BSH_PowerBox_Node.ino`
- [x] Flashed, Serial Monitor shows `Connected. IP address: 192.168.50.14`
- [x] From a laptop on the same subnet, a Modbus TCP client can write
      Coil 0 (`LIGHTS_ON_CMD`) and see the light turn on
- [x] Write Coil 1 (`FAN_CMD`) and see the fan relay respond

- [x] `HEARTBEAT` (IR0) counts up ~1/second; `RECONNECT_COUNT` (IR1) reads 0 after boot and 1 after a Wi-Fi drop and recovery

## Date log
| Date | What happened |
|---|---|
| 2026-10-05 | Phase B firmware written (WiFi static IP 192.168.50.14 + Modbus TCP via ModbusIP_ESP8266), same register map as the system doc. First written Phase B sketch for this node; the authority rule uses an onSetCoil callback so a fresh write is honored even if the value is unchanged. Watchdog detects WiFi link loss only. NOT yet flashed -- checklist above still open. |
| 2026-10-21 | **Authority rule changed.** With OpenPLC polling (it rewrites the coils every ~100 ms) the Oct 6 sketch let the PLC undo every local button press within one poll. A coil write now counts as a hub command only when its value differs from the last hub value (forgotten after a WiFi rejoin). Flashed by Luis and bench-tested with OpenPLC for lights and fan: button press sticks, force/release follow, button press with the force on stays. Old sketch: `Archive/BSH_PowerBox_Node_PhaseB_every-write-wins.ino`. Resolves the Lessons Learned item below ("check this when the PowerBox ladder logic is written"). |
| 2026-10-06 | **Phase B bench-confirmed.** `ping 192.168.50.14` showed the address free; flashed; Serial showed `Connected. IP address: 192.168.50.14`; compiled first time (the `onSetCoil` callbacks were accepted by the library). `Test-NetConnection` port 502 succeeded. Coil0/Coil1 writes switched lights and fan, and `Status` (DI0/DI1) followed. Local buttons flipped each output without any laptop command, and a fresh `LightsOff`/`FanOff` write took control back. With the router off, both outputs went OFF after ~5s and the buttons kept working; after the router returned, heartbeat kept counting and `RECONNECT_COUNT` read 1. Nothing needed changing after the flash. |

## Lessons Learned
- The PowerShell test functions are per-node: PowerBox's registers differ from Fire Alarm's (DI0/DI1 are lights/fan status, IR0/IR1 are heartbeat/reconnects), so the Fire Alarm `Regs`/`Inputs` helpers read the wrong layout here.
- OpenPLC writes coils continuously in Phase C, which will repeatedly re-assert the hub's value and can undo a button override; check this when the PowerBox ladder logic is written.

## Troubleshooting Tips

## 2026-10-28 -- Power-rails redesign: new sketch adopted (adoption Step 2)
- The BSH-PowerBox no longer drives lights and a fan; it switches power to Elevator, FireAlarm and Door. `BSH_PowerBox_Node.ino` was **replaced** by the power-rails sketch (three rails on GPIO 25/26/27, NO contact, rails off at boot, off after 5 s without WiFi, same changed-value hub-write rule, heartbeat and reconnect count last). The previous sketch is `Archive/BSH_PowerBox_Node_lights-and-fan.ino`; the 2026-10-06 and 2026-10-21 entries above describe that old design.
- **Bench results (Luis reported, no values, not seen by the agent):** the prototype version (`Prototype_PowerRails/Firmware/PowerRails_Node/`, same logic, real WiFi values filled in) ran on a spare ESP32 at 192.168.50.14 against container .114; heartbeat counted up; all three rails powered the real nodes; WiFi loss cut the rails and they returned. **Not recorded:** serial output, current readings, relay boot behaviour, compile output.
- **This repo copy has placeholder WiFi values and stops at boot until real ones are set locally.** It was written by the agent and **not compiled or flashed** from this path; Luis flashes it with his own values and records the result here.
- Pair rule: system doc, bring-up checklist and PLC register map still describe the old design until adoption Steps 3 and 4 are done (`Prototype_PowerRails/ADOPTION_CHECKLIST.md`).

### 2026-10-28 evening -- first bench output of the power-rails sketch (pasted by Luis, verified from the text)
- Flashed by Luis with his own WiFi values (the repo copy later found to hold real values again, see `Logs/2026-10-28.md` Update 56). 15 consecutive Serial status lines pasted, one per second: `Elevator=CUT  FireAlarm=CUT  Door=CUT  hb=31 rc=0  wifi=up` through `hb=45`. So: WiFi joined, HEARTBEAT counts +1 per second, RECONNECT_COUNT 0, all three rails off (option A, boot state) at about 31 s after boot.
- **Not yet shown:** the boot banner and IP line, the compile output, a rail turned on (Serial `1`) with the relay click and the node powered, a rail turned on through the PLC (Modbus write to `%QX0.0` to `%QX0.2` on container .114), whether the relays click at reset, any current reading. The Step 2 bench box in `Prototype_PowerRails/ADOPTION_CHECKLIST.md` stays open until those are recorded.

### 2026-10-28 evening -- Serial test of all three rails (Luis reported, no output pasted)
- With the new sketch flashed, typing `1`, `2` and `3` in the Serial Monitor powered the Elevator, FireAlarm and Door rails, and typing each again powered it off (Luis reported; which nodes were attached, the relay clicks and the Serial lines were not shown). This covers the local Serial path only. **Still open:** the on-command through the PLC (Modbus write to the container's `%QX0.0` to `%QX0.2`), boot-time relay behaviour, current, compile output.
