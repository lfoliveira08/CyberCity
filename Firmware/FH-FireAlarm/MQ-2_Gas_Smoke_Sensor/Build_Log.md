# MQ-2 Gas/Smoke Sensor — Build Log (FH-FireAlarm)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

> **Phase 2 — not started.** FH systems begin only after all five BSH systems
> are complete (`Firmware/WORKFLOW.md` Section 1). This log was created empty on
> Sep 26, 2026 when FH-FireAlarm was given a proper system folder; every box
> below is genuinely unchecked.
>
> **Read BSH's log for this same component first:**
> `../../BSH-FireAlarm/MQ-2_Gas_Smoke_Sensor/Build_Log.md`. The component and wiring are
> identical, so its Lessons Learned apply here before you start — the point of
> this separate file is to record what happens on *Founder's Hall's* parts, not
> to re-derive what BSH already learned.

## Status
- [ ] Simulated in Wokwi first (Phase A0 — FH reuses BSH's simulation,
      `../../BSH-FireAlarm/Wokwi/README.md`; copy it to `../Wokwi/` when this
      system becomes active) — date: ______
- [ ] Component in hand
- [ ] Wired per `Wiring.md`
- [ ] Bench-tested standalone (see checklist in `Wiring.md`)
- [ ] Confirmed working with `../FH_FireAlarm_Node/FH_FireAlarm_Node.ino`

## Date log
| Date | What happened |
|---|---|
| 2026-11-16 | **This file's status table above is stale** (last touched Sep 26, 2026, "Phase 2 not started") -- the node has actually been bench-confirmed since Oct 6 per `../ESP32_Core_Modbus/Build_Log.md`; not corrected here today, flagging only. **Sensor identified as an MQ-4** (Luis), not MQ-2 as this folder's name assumes and not MQ-9 like BSH's bench sensor -- `Hardware_BOM.md`'s "FH gas module... model unrecorded" open row is now answered. Found a false-alarm bug: the fixed 2-second post-boot calibration in `FH_FireAlarm_Node.ino` was grabbing a baseline while this MQ-4 was still warming up (climbing for 10-20+ seconds after power-on), so `GasSmokeAlert` latched true with no gas present. Fixed in firmware (self-settling warm-up before calibrating; see `../ESP32_Core_Modbus/Build_Log.md` for the full entry and the Serial evidence). Folder name/label (`MQ-2_...`) not renamed this session -- same kind of model-vs-folder-name mismatch as BSH-FireAlarm's MQ-9 (`Hardware_BOM.md`), flagging rather than fixing silently. |

## Lessons Learned
- Don't trust a model name from a folder or an old doc -- BSH turned out to be MQ-9 and FH MQ-4, both against an MQ-2 assumption baked into file/folder names early on.
- A fixed, short post-boot calibration window is not safe to assume works for a newly swapped sensor unit -- this MQ-4 needed 10-20+ seconds to settle, unlike the original FH sensor's near-instant ~2700 baseline recorded Oct 6.

## Troubleshooting Tips
- If `GasSmokeAlert`/the buzzer latches true with no real gas present and won't clear, check the Serial warm-up trace (`warm-up reading=... delta=... settled(n)`) before suspecting the sensor itself -- it may just need longer to settle than the current placeholder constants allow.
