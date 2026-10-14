# Automated Cyber-Physical Campus System Design — Revised Architecture Overview (Per-System PLC Design)

**Status:** Superseded in part — `Piranga_Workflow_v2.md` is authoritative for networking (its Section 6 replaced this document's shared-hub/custom-port scheme with per-system Docker containers on their own IPs, Sep 22 2026), for the Door's field bus (RC522 RFID, not Wiegand, Sep 15 2026) and for slot 5 (Backup Power / ATS Monitor as of Sep 28 2026, which replaced Perimeter/Motion, which had replaced Air Quality on Sep 14 2026; slot 5 was then dropped entirely on Oct 5 2026, leaving 4 systems per building). Those changes have been folded into the tables below. Kept for its "why all-Modbus, no bridges" reasoning, which still stands.

**Original status:** Locked in as the current design. All 8 systems connect directly to their own OpenPLC over Modbus TCP — no protocol-translation bridges anywhere. Not yet merged into the main `Piranga_Workflow.md`; use `Piranga_Workflow_v2.md` as the full current reference.

## What Changed From the Original Workflow

The original workflow compared protocols **per building**: BSH ran entirely on Modbus TCP, Founder's Hall ran entirely on MQTT, bridged through one shared OpenPLC + Node-RED hub. The revised design compares protocols **per system** instead, and gives each system its own dedicated PLC — closer to how a real building's fire alarm, elevator, access control, and lighting systems are actually separate vendor systems integrated at a SCADA/BMS layer, not one shared controller.

Both buildings now have the identical 4 systems (Elevator, Fire Alarm, Door, Power Box — Backup Power was dropped Oct 5, 2026 — with Founder's Hall's slot 1 since changed from Elevator to HVAC, see `FH_Systems/01_FH-HVAC.md`), each mirrored between BSH and Founder's Hall with the same protocol choices — 8 ESP32 nodes total, 8 dedicated OpenPLC instances total, unified by one SCADA/dashboard layer.

## Why All-Modbus, No Bridges

OpenPLC's master/polling engine only speaks Modbus TCP or Modbus RTU to field devices — it has no native way to poll BACnet or subscribe to MQTT. A direct ESP32-to-OpenPLC connection (each ESP32 wired straight to its own dedicated PLC with its own ladder logic, as the project team specified) is therefore only possible over Modbus. Protocol diversity for teaching purposes is preserved at the **field-bus layer** instead — the layer between each ESP32 and its own physical sensors/actuators, which has nothing to do with OpenPLC:

| System | Field layer (sensor/actuator ↔ ESP32) | Backhaul (ESP32 ↔ its OpenPLC) |
|---|---|---|
| Elevator | GPIO/I2C | Modbus TCP |
| Fire Alarm | GPIO | Modbus TCP |
| Door | **SPI** (RC522 RFID card reader) | Modbus TCP |
| Power Box | Relay (lights + fan) | Modbus TCP |

An alternate design explored BACnet/IP (Elevator, Fire Alarm) and MQTT (Air Quality) as the backhaul instead, each requiring a translation bridge in front of its OpenPLC. That design is fully documented in `Alternate_Design_BACnet_MQTT_Option.md` and can be revisited later if desired — nothing about the current field-layer choices (RC522 RFID over SPI at the Door) would need to change if that switch happens.

## Full Network / PLC Map

| Node | ESP32 IP | Backhaul | OpenPLC Instance | OpenPLC Container IP | Web UI Port | Modbus Server Port |
|---|---|---|---|---|---|---|
| Proxmox server (`pve`, Proxmox VE 9.2.2) | - | - | 192.168.50.143 (host, Web UI :8006) | - | - |
| OpenPLC Docker VM (on Proxmox; hosts all 8 containers) | - | - | 192.168.50.10 (VM host) | - | - |
| SCADA/HMI VM (on Proxmox; Ignition) | - | - | TBD (to be assigned) | - | - |
| BSH-Elevator | 192.168.50.11 | Modbus TCP | OpenPLC-BSH-Elevator | 192.168.50.111 | 8080 | 502 |
| BSH-FireAlarm | 192.168.50.12 | Modbus TCP | OpenPLC-BSH-FireAlarm | 192.168.50.112 | 8080 | 502 |
| BSH-Door | 192.168.50.13 | Modbus TCP | OpenPLC-BSH-Door | 192.168.50.113 | 8080 | 502 |
| BSH-PowerBox | 192.168.50.14 | Modbus TCP | OpenPLC-BSH-PowerBox | 192.168.50.114 | 8080 | 502 |
| FH-HVAC | 192.168.50.21 | Modbus TCP | OpenPLC-FH-HVAC | 192.168.50.121 | 8080 | 502 |
| FH-FireAlarm | 192.168.50.22 | Modbus TCP | OpenPLC-FH-FireAlarm | 192.168.50.122 | 8080 | 502 |
| FH-Door | 192.168.50.23 | Modbus TCP | OpenPLC-FH-Door | 192.168.50.123 | 8080 | 502 |
| FH-PowerBox | 192.168.50.24 | Modbus TCP | OpenPLC-FH-PowerBox | 192.168.50.124 | 8080 | 502 |

Every OpenPLC instance plays two Modbus roles: **master** to its own ESP32 (polling it directly at the ESP32's IP on port 502), and **server** to the SCADA layer (on the canonical port 502 — every instance can bind 502 because each container holds its own LAN IP, so instances are told apart by address, not by port).

All 8 OpenPLC instances run as separate Docker containers inside one Proxmox VM (the OpenPLC Docker VM) on the Proxmox server, each attached to the LAN over macvlan so it holds its own real IP — this keeps hardware cost and count the same as the original single-hub design while still giving each system its own logical PLC, ladder program, and point database. Nothing here prevents moving individual instances to their own physical Raspberry Pi later if you want that added realism.

## SCADA Layer

Runs in its own SCADA/HMI VM on Proxmox. Ignition (Maker Edition; professor approved Oct 12, 2026, company license pending), polling all 8 OpenPLC Modbus servers as separate device connections and merging them into one dashboard. Fallbacks: Rapid SCADA, FUXA. Options and status: `Piranga_Workflow_v2.md` Section 10.

## Open Items

1. `Piranga_Workflow_v2.md` is the authoritative current reference. (The original v1 workflow has no local copy — it exists only in the Claude project, as `claude/CyberUIW_Workflow.md`.)
2. The cascade event flow (originally Section 5 of the v1 doc) now routes entirely through the SCADA layer via uniform Modbus reads/writes to any of the 8 OpenPLC instances — see `Piranga_Workflow_v2.md` Section 7.
3. DALI was retired Sep 28, 2026 in favor of a plain relay for Power Box lighting, removing the project's only unverified field-bus library — see `BSH_Sensors/12_DALI_LED_Driver.md` (now a retirement stub) and `Piranga_Workflow_v2.md` Section 14. (The Door's reader risk dropped when Wiegand was replaced by the RC522 RFID module on Sep 15 2026.)
4. Decide whether all 8 OpenPLC containers truly run on one shared hub (as mapped above) or get split across multiple physical hosts.
5. macvlan needs a wired Ethernet uplink: resolved Oct 9, 2026, the Proxmox server (a dedicated wiped PC) is cabled to the lab router by Ethernet (bridge `vmbr0`). Old fallback, no longer needed: if it were Wi-Fi only, this becomes Docker `ipvlan` L3 mode or a USB Ethernet adapter (see `Piranga_Workflow_v2.md` Section 6).
