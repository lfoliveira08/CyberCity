# CyberCity — Project Wiki

A reader-friendly guide to the CyberCity ICS/SCADA cybersecurity testbed. These pages **summarise** the design docs; the files they cite are the source of truth, and each claim is labelled **verified**, **reported** (by Luis, not independently checked) or **not checked**.

> **Why this folder and not a GitHub Wiki:** GitHub Wikis are not available on this private repository on the current plan (Settings → General). These Markdown pages act as the wiki and can be copied into a real Wiki tab if the repo ever goes public.

> **Lab use only.** The attack material describes a private, isolated test network. Do not run it against anything you do not own or have written permission to test.

## Pages

| Page | What it covers |
|---|---|
| [Architecture](Architecture.md) | The two buildings, eight nodes, the ESP32 → OpenPLC → Ignition data path, with a diagram |
| [Network & IP plan](Network-and-IP.md) | Every host, VM, container and port on `192.168.50.0/24` |
| [SCADA / HMI](SCADA.md) | The Ignition layer: devices, the Device UDT, Campus_Map, alarms |
| [Security scenarios](Security-Scenarios.md) | The attack and defence scenarios and how evidence is captured |
| [Rebuild guide](Rebuild-Guide.md) | How to reproduce one node end to end |
| [Roadmap](Roadmap.md) | The seven milestones and where each stands |

## Where the real docs live
- [`../STATUS.md`](../STATUS.md) — the single home for project status (read this for "where are we")
- [`../00_START_HERE.md`](../00_START_HERE.md) — what the project is and what lives where
- [`../CLAUDE.md`](../CLAUDE.md) — the rules and the hard facts
- [`../Piranga_Workflow_v2.md`](../Piranga_Workflow_v2.md) — the master design document
- [`../Logs/`](../Logs/) — one decision-and-lessons log per working day

Target: **demo Dec 11, 2026**.
