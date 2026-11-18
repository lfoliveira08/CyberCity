# ESP32 Core + WiFi/Modbus — Build Log (FH-FireAlarm)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

> **Phase B complete, bench-confirmed Oct 6, 2026.** This folder covers the
> board itself joining WiFi and answering as a Modbus TCP slave, not any one
> sensor. Done ahead of the build order at Luis's request; mirrors
> `BSH-FireAlarm`'s bench-confirmed Phase B sketch. Network: SCT5, 192.168.50.x.
>
> This folder holds only a `Build_Log.md` — no `Wiring.md` — because there is
> nothing component-specific to wire beyond USB power. That exception is written
> down in `Firmware/WORKFLOW.md` Section 3.

## Status
- [x] Board boots, Serial Monitor readable at 115200
- [x] Joins the shared project WiFi AP
- [x] Static IP confirmed as **192.168.50.22** in the Serial output
      — not BSH-FireAlarm's `.12`. Two FH sketches have already shipped with a
      BSH address copy-pasted in (FH-Door had `.13`, the retired FH-Elevator had
      `.11`), so check this one rather than assume it
- [x] Answers a Modbus TCP read from a laptop client on the same subnet
- [x] All seven registers respond per `FH_Systems/02_FH-FireAlarm.md`
      (DI0 MANUAL_PULL_TRIGGERED, DI1 FLAME_DETECTED, DI2 GAS_SMOKE_ALERT,
      IR0 GAS_RAW_VALUE, IR1 HEARTBEAT, IR2 RECONNECT_COUNT,
      Coil0 ALARM_ACTIVATE)
- [x] `HEARTBEAT` counts up ~1/second; `RECONNECT_COUNT` reads 0 after boot, 1 after a Wi-Fi drop and recovery, 0 again after reset
- [x] Local alarm logic (button, flame, gas) works with WiFi down (Fire Alarm has no comm watchdog by design: local sources always sound the alarm)
- [x] Both fire alarm nodes up at once on distinct IPs, each independently
      reachable — the test that proves the IP above is really set

## Date log
| Date | What happened |
|---|---|
| 2026-10-06 | Phase B firmware written as a mirror of BSH-FireAlarm's confirmed sketch: static IP 192.168.50.22, same pins, logic, registers and heartbeat. NOT yet flashed. FH's standalone Phase A was never logged, so the first Serial run also serves as FH's hardware check. |
| 2026-10-06 | **Phase B bench-confirmed.** Flashed; Serial showed `Connected. IP address: 192.168.50.22` (not .12). `Test-NetConnection` port 502 succeeded. Regs: gas baseline 2773 at boot, 2695 a minute later, heartbeat counting, reconnects 0. Inputs read 0 at rest; Coil0 on/off worked; button, flame and gas inputs each worked; Wi-Fi drop and reset checks passed. Nothing needed changing after the flash. |
| 2026-11-16 | **Board swapped (Luis-reported).** This node was found disconnected (Ignition `Status`=2, OpenPLC log repeating `Device FH-FireAlarm-ESP32 is disconnected`, alternating `Connection timed out` / `Bad file descriptor` / `Connection refused` / `Operation now in progress`). Wiring and the known marginal-WiFi-signal theory were both suspected first; Luis instead swapped the ESP32 board, and it connected immediately. **Root cause: the original board itself (confirmed by substitution, not diagnosed further) — not the Wi-Fi signal, not the PowerBox wiring.** See `Logs/2026-11-16.md` for the full session. This also means the gas sensor now bench-confirmed below is physically a different unit than the one characterized Oct 6 (see Lessons Learned) — its baseline and warm-up behavior do not match the Oct 6 numbers. |

## Lessons Learned
- FH's clean-air gas baseline is about 2700 (BSH's is about 40): a different sensor part or module, not a fault. It stayed within ~80 counts over a minute and the 400 margin gave no false alerts. Re-tune `GAS_ALERT_MARGIN` if FH's sensor is swapped.
- **Superseded 2026-11-16:** after the board swap above, this node's gas sensor behaves completely differently from the paragraph above — baseline now lands in the 1100-1700 range (varies run to run) and climbs for 10-20+ seconds after power-on before settling, instead of holding steady near 2700 within ~80 counts. Luis identified the sensor as an **MQ-4** (this folder is still named/labeled `MQ-2_Gas_Smoke_Sensor` from an earlier assumption — see that component's own `Build_Log.md`). The fixed `GAS_ALERT_MARGIN=400` plus a 2-second post-boot calibration was grabbing a baseline mid-drift, causing a false alarm that stayed latched (the alarm never clears because `gasBaseline` is only ever set once, at boot). Fixed in `FH_FireAlarm_Node.ino`: `calibrateGasSensor()` now waits for 5 consecutive 1-second readings to settle within 15 counts (120s timeout) before taking the baseline, and both that wait loop and the final averaging pass call `mb.task()` so the node keeps answering Modbus throughout instead of going quiet. Also reordered `setup()` so WiFi/Modbus start before this wait, not after (the first version of this fix calibrated before WiFi, so the node looked "disconnected" in Ignition for the whole warm-up on every boot -- that was a self-inflicted bug from the same session, not the original problem). Confirmed via Serial transcript (2026-11-16): WiFi connects first, warm-up settles after ~20 one-second readings, `baseline=1385`/`alert>1785`, live `gasRaw` (1392, 1328) stays below threshold with `alarmActive=false (green)`. **Second-pass fix, same day:** first flash still blipped Ignition `Status` 0->1->2->0 during the warm-up (Luis-observed) -- `mb.task()` kept Modbus answering but `heartbeat` only ticks in `loop()`, so `HEARTBEAT` went stale and tripped the Device UDT's `Status` rule even though the node was never actually disconnected. Fixed by ticking `heartbeat` once/second inside `calibrateGasSensor()`'s warm-up wait and once in the final averaging pass. Re-flashed and **confirmed by Luis: `Status` stayed at 0 throughout the full reboot.** Both bugs (latched false alarm, Status blip) are now bench-confirmed fixed. **Still open:** `STABLE_WINDOW_COUNTS`/`STABLE_READINGS_NEEDED`/`WARMUP_TIMEOUT_MS` are first-pass placeholders from one run, not tuned across repeats; BSH-FireAlarm's sketch was deliberately left untouched (Luis's call) even though it has the same placeholder margin and fixed-delay calibration.
- Also noticed, same session: Luis observed the FH-FireAlarm Wi-Fi dropping in correlation with FH-PowerBox switching a rail -- confirmed **not** the HVAC rail; which rail (if any) still open. `Firmware/FH-PowerBox/Schematic/Connections.md` documents (as an unconfirmed assumption, not bench-verified) that FH-FireAlarm's own 5V power is fed through a relay on FH-PowerBox, and all three PowerBox rails are assumed to share one COM/VIN source -- if true, switching one rail could sag the shared supply and brown out another node's WiFi. Not confirmed with a multimeter; not yet resolved.
- `$ESP`, and the PowerShell test functions, are lost when the PowerShell window closes; paste them again in a new window.

## Troubleshooting Tips
