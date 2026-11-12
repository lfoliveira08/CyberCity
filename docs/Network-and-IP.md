# Network & IP plan

Source of truth: [`../CLAUDE.md`](../CLAUDE.md) §2 and [`../Proxmox_Build_Log.md`](../Proxmox_Build_Log.md). All addresses **verified** against `CLAUDE.md` §2. Subnet: `192.168.50.0/24`.

## Nodes and their containers

| Node | ESP32 | OpenPLC container |
|---|---|---|
| BSH-Elevator | 192.168.50.11 | 192.168.50.111 |
| BSH-FireAlarm | 192.168.50.12 | 192.168.50.112 |
| BSH-Door | 192.168.50.13 | 192.168.50.113 |
| BSH-PowerBox | 192.168.50.14 | 192.168.50.114 |
| FH-HVAC | 192.168.50.21 | 192.168.50.121 |
| FH-FireAlarm | 192.168.50.22 | 192.168.50.122 |
| FH-Door | 192.168.50.23 | 192.168.50.123 |
| FH-PowerBox | 192.168.50.24 | 192.168.50.124 |

Each OpenPLC container exposes **port 8080** (web UI) and **port 502** (Modbus). Ignition connects only to the containers' `.1xx` addresses, never to an ESP32.

## Infrastructure

| Host | IP | Role |
|---|---|---|
| `pve` | 192.168.50.143 | Proxmox VE host |
| VM 100 `openplc-docker` | 192.168.50.10 | Docker host running the 8 OpenPLC containers (macvlan) |
| VM 101 `scada-hmi` | 192.168.50.30 | Ignition 8.3.10 Gateway |
| VM 102 `kali` | 192.168.50.40 | Attacker / analysis VM |

## Known infrastructure notes (from STATUS.md, Nov 10 2026)
- **Reported:** the `pve` `local-lvm` thin pool is over-provisioned and filled to 100% on Oct 26, pausing VM 100 once.
- **Reported:** VM 100's root filesystem hit 100% on Oct 28 from an un-capped Docker log (~24 GB); freed the same evening, but **no per-container log size limit is set yet**, so it can refill.
- The Ignition Capstone licence is active (expires 2027-01-31) and renews on a ~72-hour lease, so VM 101 **needs internet**.
- **Open:** a recon capture on Nov 10 reported an unidentified ESP32 at `.161` and a flat OT subnet with personal devices present — see the [Security scenarios](Security-Scenarios.md) page.

For the live state always check [`../STATUS.md`](../STATUS.md).
