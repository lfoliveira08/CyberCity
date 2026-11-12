# SCADA / HMI (Ignition)

Source of truth: [`../STATUS.md`](../STATUS.md), [`../SCADA_HMI/`](../SCADA_HMI/) and its `CLAUDE.md`. Ignition how-to is always looked up in `../References/Ignition/` (the 8.3 manual) and cited there. State below is **as of Nov 10, 2026**; most functional results are **Luis-reported**, not independently verified.

## What exists (reported, Nov 3 2026)
- **Eight devices and tag trees** — one Modbus device per OpenPLC container, tags hand-built in a `<Bldg>/<System>/` folder layout, all exported to `../SCADA_HMI/Exports/` with dated names.
- **`Device` UDT** — one definition with members including `Heartbeat`, `ReconnectCount`, `HeartbeatAge` and `Status`; **eight instances**, one per node.
- **Campus_Map** — a Perspective view with a building image per building and **eight status dots**, each dot's colour bound to its node's `Status`.
- **Detail popups** — `Device_Detail`, `FireAlarm_Detail`, `Door_Detail`, `Elevator_Detail`, `HVAC_Detail`, `PowerBox_Detail`.
- **Alarms** — a `NodeOffline` alarm on the UDT `Status` member, an Alarm Status Table, and an internal alarm journal `CampusLog` (7-day pruning) shown in an `Alarms_Panel` view.
- **Project export + Gateway backup** taken Nov 3 (git-ignored; see below).

## The Status rule
`Status` drives each dot's colour: **0 green** (healthy), **1 yellow** (heartbeat frozen ~3 s), **2 red** (frozen ~5 s or bad quality). The rule lives in `../SCADA_HMI/Templates/README.md`.

> **Open question:** the Section 10 colour table says red = "intrusion / compromised", while the heartbeat rule makes red = "frozen". Which meaning red carries is **not yet decided**.

## Addressing — settled
**Zero-based Addressing is ON** on every device (closed Oct 29, 2026). With it on, `HR0` = gas, `HR1` = heartbeat, `HR2` = reconnects; coils are `C0`–`C4`. Turn it **on** when creating any new device.

## Built, but not fully tested (from STATUS.md "Next up")
- The unplug test (`Status` 0→1→2→0) was reported on **BSH-FireAlarm only** at first; Luis later reported testing the others (reported, no values).
- The pull-station cascade (a Value Changed script that cuts the Elevator and HVAC rails) is **saved but not tested**.
- `Active` (lost) rows in the alarm journal had not been seen at the time of writing.

## Backups
Gateway backups (`.gwbk`) and the project `.zip` are **git-ignored** (they can contain credentials) and live only on the build laptop. Restores have **not** been tested. See `../SCADA_HMI/Exports/README.md`.
