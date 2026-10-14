# ESP32 Core + WiFi/Modbus — Build Log (FH-FireAlarm)

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

## Lessons Learned
- FH's clean-air gas baseline is about 2700 (BSH's is about 40): a different sensor part or module, not a fault. It stayed within ~80 counts over a minute and the 400 margin gave no false alerts. Re-tune `GAS_ALERT_MARGIN` if FH's sensor is swapped.
- `$ESP`, and the PowerShell test functions, are lost when the PowerShell window closes; paste them again in a new window.

## Troubleshooting Tips
