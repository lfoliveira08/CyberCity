# Roadmap

The seven milestones, mirrored from the GitHub milestones and [`../STATUS.md`](../STATUS.md). Status is **as of Nov 23, 2026**; functional results on the SCADA and attack layers are **Luis-reported** unless a capture is cited. Target: **demo Dec 11, 2026**.

| Milestone | Window | State |
|---|---|---|
| **M1 Firmware** | Aug 24 – Oct 9 | **Done** — ESP32 firmware bench-confirmed on all 8 nodes |
| **M2 OpenPLC layer** | Oct 10 – Oct 21 | **Done** — all 8 OpenPLC programs built and running on their containers |
| **M3 SCADA tags & Device UDT** | Oct 22 – Nov 2 | **Closed Nov 23** — built; leftovers (restore test of the Gateway backup, unplug test on 7 nodes) tracked in `STATUS.md` Open items |
| **M4 HMI & Campus_Map** | Nov 1 – Nov 3 | **Closed Nov 23** — built (reported); leftovers (blue and intrusion-red health states, HMI_View docs, Security Settings on buttons) tracked in `STATUS.md` |
| **M5 Cascade rules** | Nov 4 – Nov 5 | **Closed Nov 23** — both pull-station cascades bench-confirmed Nov 16; BSH cascade redesigned Nov 17 to write `ElevatorDisable`. #27 (system keeps working if SCADA goes offline) was closed but **never tested** |
| **M6 Attack scenarios & capture** | Nov 6 – Dec 4 | **In progress** — S3, S4, S5, S6 run Nov 18; S7, S8, S9, S10, S11 run Nov 23, with pcaps in `Security_Capture/Captures/`. Not run: S12, S13, S14, S15. S9 as scoped (isolate one node) was not demonstrated; a flood at one node took down the whole flat network |
| **M7 Demo** | Dec 5 – Dec 11 | Not started — live scenarios, model, diagrams, report, healthy lab |

The date windows are the planning phases used on the GitHub Project roadmap (`CyberCity Roadmap`). They are planning estimates, not exact per-task schedules.

## One line
Phases A and B are done on all eight nodes. The OpenPLC layer is done. The SCADA/HMI layer is **built and partly tested**, and its three milestones are closed. Attack scenarios are **under way: nine of the scenarios have been run and captured** (S3–S11); S12–S15 remain. See [`../STATUS.md`](../STATUS.md) for the authoritative, current status and the full open-items list.
