# Fan relay (GPIO 15) -- Build Log (FH-PowerBox)

Mirror of `BSH-PowerBox/Fan_Relay/Build_Log.md`. Bench-tested on FH hardware Oct 7, 2026 (Phase B).

## Status
- [x] Component in hand
- [x] Wired per `Wiring.md`
- [x] Relay polarity checked on THIS board (BSH's boards were active-HIGH; do not assume)
- [x] Works with the Phase B firmware (see `../ESP32_Core_Modbus/Build_Log.md`)

## Date log
| Date | What happened |
|---|---|
| 2026-10-07 | Phase B: polarity confirmed active-HIGH (`RELAY_FAN_ACTIVE_LOW = false`) -- output OFF at boot; Modbus coil, status DI, local button, hub-regains-authority and 5 s watchdog all passed. |

## Lessons Learned

## Troubleshooting Tips
