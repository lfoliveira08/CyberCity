# silahcen001 / DALI-ESP32 (`DaliController`)

- **Link:** https://github.com/silahcen001/DALI-ESP32
- **System:** Power Box (BSH and FH) — evaluated for, no longer used
- **Status:** **Rejected** — Sep 28, 2026 (previously Adopted, not yet bench-verified)
- **License:** not recorded; irrelevant, since nothing was taken to real hardware
- **Checked:** not independently verified before the decision to drop it

## What it is

A DALI master implementation for the ESP32, exposing a `DaliController` class.
DALI is the dedicated commercial lighting-control bus, and it supports
addressable dimming, which the project's `LIGHTS_DIM_LEVEL` register (HR0,
0-254) used to exercise before Power Box switched to a plain relay.

## Why it was rejected

Dropped in favor of a plain on/off relay for lighting — the same pattern
already used for the fan channel and proven on Fire Alarm/Elevator. See the
Sep 28, 2026 entries in `Piranga_Workflow_v2.md` Section 14 for the full
decision and trade-off (this removes the project's only unverified
field-bus library and untested transceiver hardware, at the cost of the
dimming/partial-dim demo value DALI was chosen for).

## Why the note exists anyway

So nobody re-evaluates it without knowing why it was dropped, and so anyone
reading old cross-references understands what replaced it
(`claude/BSH_Sensors/09_Relay_Module.md` / `claude/FH_Sensors/09_Relay_Module.md`).
Also kept as a record of the risk this node used to carry: several
incompatible forks of DALI-ESP32 exist, the project's firmware was written
against this repo's public README without method names ever confirmed
against an installed version, and it needed a DALI bus transceiver add-on
plus a DALI-addressable LED driver that nobody had bench-tested.
