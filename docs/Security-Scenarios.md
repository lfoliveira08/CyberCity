# Security scenarios

Source of truth: [`../Piranga_Workflow_v2.md`](../Piranga_Workflow_v2.md) §11 and [`../Security_Capture/`](../Security_Capture/) (`Scenarios.md`, `Threat_Model_Scope.md`, `ATTACK_ICS_Mapping.md`, `Real_World_Incidents.md`, `References.md`). Runbooks are in `../Security_Capture/Kali/`.

> **Lab use only — closed network.** Capture and attack only the testbed subnet `192.168.50.0/24`. These are demonstrations on hardware the project owns. Do not run them anywhere else.

## Status (Nov 23, 2026)
**Scenarios 3 to 11 have been run against the closed lab and captured** (Luis-reported from script output; the pcaps stay on the lab disk, not in git). S3, S4, S5 and S6 ran on Nov 18; S7, S8, S9, S10 and S11 ran on Nov 23. **Not run yet:** S12, S13, S14 and S15.

**S9 finding:** isolating one node was **not demonstrated** as scoped. A SYN flood aimed at one low-value node took down the whole flat network, not just that node, and recovered the moment the flood stopped. That is a stronger result than planned and needs its own write-up.

Earlier, the Nov 10 reconnaissance capture confirmed the inventory and surfaced: both FireAlarm ESP32s flapping on weak Wi-Fi, an unidentified ESP32 at `.161`, and a flat OT subnet. Full detail: [`../STATUS.md`](../STATUS.md).

## Scenario list (Workflow §11, plus later additions)

| # | Scenario | Note |
|---|---|---|
| 1 | Normal campus operation | baseline |
| 2 | Authorized door access | baseline |
| 3 | **Modbus door-open injection** | revised Nov 8 from an RFID clone to an FC 5 write to the door-open coil on `.113` |
| 4 | False fire-alarm injection | unauthenticated write to the alarm coil; detection is the mismatch with the pull-station input |
| 5 | Elevator lockout (BSH) | plus a persistent-lockout variant |
| 6 | *(dropped Oct 5, 2026)* | — |
| 7 | Power Box manipulation | switch a rail with no logged source |
| 8 | Cross-building cascade | an event in one building reacts in both |
| 9 | Node isolation / resilience | stop one PLC; only that system shows offline |
| 10 | Red/blue exercise | attack and detect together |
| 11 | HVAC sensor spoofing / setpoint tamper (FH) | analog false-data injection |
| 12 | **PLC logic injection** | via the OpenPLC web UI on port 8080 (engineering channel); ladder logic cannot be changed over Modbus |
| 13 | **ESP32 bricking** | spare board only, recoverable soft-brick; permanent eFuse brick documented, not performed |
| 14 | **Combined MITM** | ARP-spoof both links; gated on a Docker/L2 networking check |
| 15 | **Multi-target DoS** | built, not run; several nodes at once, not a DDoS |

Why Modbus is the target: standard Modbus TCP has **no authentication** — any host on the subnet can read or write points. That is the lesson most scenarios demonstrate.

## Evidence
Every run produces one `.pcapng` plus one run log, kept together in `../Security_Capture/Captures/`. Pcaps are **git-ignored**. The naming and capture rules are in `../Security_Capture/Captures/README.md` and `../Security_Capture/README.md`.

A later phase adds real Allen-Bradley PLC attacks; those are deferred. Scenario 12's runbook includes a **mandatory restore step** (re-upload the known-good `../PLC_Logic/<System>/Program.st`).
