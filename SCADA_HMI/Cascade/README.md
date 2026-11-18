# Cascade -- cross-system rules

The rules themselves are defined in `Piranga_Workflow_v2.md` Section 7. This folder holds how they are built in Ignition (tags, scripts, alarm pipelines) and how each path was tested.

Order of work: within each building first, then cross-building. Each path gets its own test entry with date and result.

| Path | Status |
|---|---|
| **BSH fire alarm → disable BSH elevator** | ✅ Built + bench-confirmed, **redesigned Nov 17, 2026** (supersedes the Nov 16 version below). Writes `[default]BSH/Elevator/ElevatorDisable` = True — the elevator's own coil 3 — instead of cutting `ElevRailCmd`. |
| **FH fire alarm → cut FH HVAC power** | ✅ Built + bench-confirmed Nov 16, 2026. Value Changed script on `[default]FH/FireAlarm/ManualPull`; writes `[default]FH/PowerBox/HvacRailCmd` = False. Confirmed: press → `HvacRailCmd` flips false and the HVAC rail physically turned off (Gateway log `FH_Cascade`). Unaffected by the Nov 17 elevator change. |

## Cascade rule (FH unchanged since Nov 16; BSH redesigned Nov 17, 2026)

**Each building's fire-alarm pull station acts on only its own building's hazard:** FH has HVAC — cut it on a fire, don't spread smoke through the ducts — so the FH script still cuts `FH/PowerBox/HvacRailCmd`. BSH has an elevator — don't ride elevators in a fire — but **as of Nov 17 the BSH script disables the elevator (`ElevatorDisable` = True) instead of cutting its power rail.**

**Why it changed (Nov 17, 2026):** the BSH-Elevator node brownout-looped whenever it was powered through the BSH-PowerBox relay rail (heavy DC-motor load through a weak source — see `STATUS.md`). Luis decided to power the elevator over USB permanently instead of chasing the rail fix further, which means the PowerBox can no longer physically cut its power. The cascade now uses the elevator's existing `ELEVATOR_DISABLE` coil (already built for Scenario 5's lockout attack) to the same effect — the car refuses new calls — without depending on the rail. **Known limit, from the firmware's own header comment:** `DISABLE` does not stop a move already in progress, only blocks new calls.

This also **supersedes the Nov 3 design** (BSH pull cutting both the BSH elevator rail *and* FH HVAC — that cross-building cut was already dropped Nov 16, see `Logs/2026-11-16.md`) and **supersedes the Nov 16 rail-cut version** of the BSH script itself (see `Logs/2026-11-17.md` for the full decision).

## How the scripts are written (important Ignition gotcha)

Tag event scripts in Ignition **already provide the `def valueChanged(...)` function signature** — you write only the **body**, indented under it (the manual's examples are body-only: `../References/Ignition/platform/tags/tag-event-scripts.md`). Pasting your own `def valueChanged(...)` on top creates a **nested function that is never called** — the script silently does nothing (no error, no write, no log). Both cascade scripts had this bug from Nov 12 until Nov 16, which is why the FH cascade "failed its first test."

**FH script (unchanged since Nov 16) — writes `HvacRailCmd`:**

```python
    if not initialChange:
        logger = system.util.getLogger("FH_Cascade")
        if currentValue.value:
            system.tag.writeBlocking(["[default]FH/PowerBox/HvacRailCmd"], [False])
            logger.info("FH pull station -> cut HvacRailCmd")
```

**BSH script (redesigned Nov 17) — writes `ElevatorDisable`, not a rail coil:**

```python
    if not initialChange:
        logger = system.util.getLogger("BSH_Cascade")
        if currentValue.value:
            system.tag.writeBlocking(["[default]BSH/Elevator/ElevatorDisable"], [True])
            logger.info("BSH pull station -> set ElevatorDisable True")
```

The `logger.info(...)` line writes an audit entry to the Gateway Logs (Status → Diagnostics → Logs, filter by the logger name) every time the cascade fires — useful for the demo and for confirming the rule ran.

**History:** the Nov 16 version of the BSH script wrote `ElevRailCmd` = False (cutting the rail), confirmed at the SCADA/tag level only — the elevator could not be powered through its PowerBox rail during that session (a power-delivery brownout, see `STATUS.md`). On Nov 17 the elevator was moved to permanent USB power, so the rail-cut approach no longer applies; the script was rewritten to use `ElevatorDisable` instead, which works regardless of how the elevator is powered. See `Logs/2026-11-17.md`.
