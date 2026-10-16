# Project Automated Cyber-Physical Campus System Design — Master Workflow Document (v2)
**Revised Architecture: Per-System PLC Design**
**Original Project Window:** August 24 – December 11, 2026
**v2 Revised:** September 2026 (Week 3) — Section 9 updated September 10 with first-pass firmware status; Air Quality replaced by Perimeter/Motion September 14 (see Section 14); OpenPLC moved to per-system Docker containers with individual IPs September 22 (see Section 14); Founder's Hall slot 1 changed from Elevator to HVAC September 26 (see Section 14); Perimeter/Motion replaced by a Backup Power / ATS Monitor system September 28 (see Section 14); Backup Power / ATS Monitor dropped October 5, 2026, leaving four systems per building and eight nodes in total (see Section 14)

---

## Table of Contents
1. [Project Summary](#1-project-summary)
2. [Why the Architecture Changed](#2-why-the-architecture-changed)
3. [System Architecture](#3-system-architecture)
4. [Building & System Assignments](#4-building--system-assignments)
5. [Protocol Plan](#5-protocol-plan)
6. [Network / IP & PLC Map](#6-network--ip--plc-map)
7. [Cascade Event Flow](#7-cascade-event-flow)
8. [Hardware Checklist](#8-hardware-checklist)
9. [Firmware Development Workflow](#9-firmware-development-workflow)
10. [Dashboard & Integration Workflow](#10-dashboard--integration-workflow)
11. [Cybersecurity Scenario Workflows](#11-cybersecurity-scenario-workflows)
12. [Weekly Milestones](#12-weekly-milestones)
13. [Deliverables Tracker](#13-deliverables-tracker)
14. [Lessons Learned & Decisions Log](#14-lessons-learned--decisions-log)

---

## 1. Project Summary

Automated Cyber-Physical Campus System Design is a 3D-printed smart campus security testbed that models and demonstrates how cyber-physical systems (CPS) can monitor, secure, and improve resilience in a campus environment.

The testbed represents a scaled-down UIW campus zone with **two buildings — BSH and Founder's Hall (FH)** — each containing **four subsystems**: Fire Alarm, Automatic Door/Gate and Power Box in both buildings, plus one building-specific system in slot 1 — a 3-floor Elevator in BSH, and an HVAC zone unit in Founder's Hall (changed Sep 26, 2026 — see Section 14). Each subsystem is its own independent ESP32 node with its own dedicated OpenPLC instance, unified by a single SCADA/dashboard layer.

**Core Concept:** Eight independent systems (four per building), each speaking a realistic combination of field-bus and network protocols, each controlled by its own logical PLC, all aggregated into one campus-wide security picture — mirroring how a real building's fire alarm, elevator, access control, and lighting systems are genuinely separate vendor systems integrated at a BMS/SCADA layer, not one shared controller.

---

## 2. Why the Architecture Changed

The original workflow compared communication protocols **per building** (BSH = Modbus TCP, FH = MQTT), bridged through one shared OpenPLC + Node-RED hub. That design worked, but it under-represented how real smart buildings are actually integrated: distinct subsystems (fire alarm, elevator, access control, lighting) are almost always separate, dedicated controllers — not one shared PLC — precisely because of differing vendors, safety/certification requirements, and the need for one subsystem's failure not to take down another.

The revised design compares protocols **per system** instead, and gives each system its own dedicated logical PLC. This is the standard industrial pattern (Purdue Model levels 0–2: field devices → per-process controller, level 2–3: SCADA/HMI aggregation), and it gives a stronger teaching narrative: comparing *why a fire alarm and a lighting system use different protocols* is a more realistic security story than comparing two buildings.

**A key constraint settled the final design:** OpenPLC's master/polling engine only speaks Modbus TCP or Modbus RTU to field devices — it has no native way to poll BACnet or subscribe to MQTT. Since each ESP32 connects directly to its own dedicated OpenPLC instance (no protocol-translation bridge in between), the backhaul for all 8 systems is Modbus TCP. Protocol diversity for teaching purposes is preserved instead at the **field-bus layer** — an RFID reader for the Door's card reader, DALI for the Power Box's lighting — which is independent of OpenPLC entirely. An alternate design using BACnet/IP and MQTT as the backhaul (each requiring a translation bridge) was fully worked out and is preserved in `Alternate_Design_BACnet_MQTT_Option.md` in case the team revisits it later; nothing about the field-layer protocol choices would need to change if that switch happens.

---

## 3. System Architecture

```
                         ┌───────────────────────────────────────────┐
                         │           SCADA / Dashboard Layer          │
                         │     (Ignition, SCADA/HMI VM on Proxmox)    │
                         │   Polls all 8 OpenPLC instances via        │
                         │   Modbus TCP — uniform interface           │
                         └───────────────┬─────────────────────────────┘
                                         │
                 ┌───────────────┬───────┴───────┬───────────────┐
                 │               │               │               │
           ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐
           │ OpenPLC-  │   │ OpenPLC-  │   │ OpenPLC-  │   │ OpenPLC-  │
           │ Elevator  │   │ FireAlarm │   │ Door      │   │ PowerBox  │
           │ (Modbus,  │   │ (Modbus,  │   │ (Modbus,  │   │ (Modbus,  │
           │  direct)  │   │  direct)  │   │  direct)  │   │  direct)  │
           └─────┬─────┘   └─────┬─────┘   └─────┬─────┘   └─────┬─────┘
                 │               │               │               │
            Modbus TCP      Modbus TCP      Modbus TCP      Modbus TCP
                 │               │               │               │
           ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐
           │  ESP32    │   │  ESP32    │   │  ESP32    │   │  ESP32    │
           │ Elevator  │   │ FireAlarm │   │ Door+     │   │ PowerBox  │
           │           │   │           │   │ RFID      │   │ +Relays   │
           └───────────┘   └───────────┘   └───────────┘   └───────────┘
```

This stack repeats for Founder's Hall with one substitution — FH's slot 1 is an HVAC zone unit rather than an Elevator (Section 4) — for 8 ESP32 nodes and 8 OpenPLC instances total, all reporting to the one SCADA layer.

---

## 4. Building & System Assignments

| System | Function | Present In |
|---|---|---|
| Elevator | 3-floor simulated elevator (motor, buttons, display) | **BSH only** |
| HVAC | Zone AC unit: temperature/humidity sensing, relay-driven compressor/fan, remotely settable setpoint | **FH only** |
| Fire Alarm | Buzzer + RGB alert, local pull station and remote trigger | BSH & FH |
| Door (Gate) | Access-controlled gate: card reader, approach sensor, position feedback | BSH & FH |
| Power Box | Dimmable lighting + fan control | BSH & FH |

Founder's Hall mirrors BSH for three of its four systems — Fire Alarm, Door and Power Box are identical in components, pin maps and protocol choices — so those three can be compared apples-to-apples, and a compromise in one building's system can be tested for whether it cascades to the identical system in the other building. Slot 1 is deliberately not mirrored: BSH runs the Elevator, FH runs an HVAC zone unit. That asymmetry buys the project a clean analog-sensor-spoofing target (`TEMP_C`) that no mirrored pair provides, at the cost of one system that cannot be cross-building compared.

---

## 5. Protocol Plan

Each system has two protocol layers: a **field layer** (how the ESP32 talks to its own sensors/actuators — a local, non-routable bus) and a **backhaul** (how that system's ESP32 reports to its own dedicated OpenPLC instance). Every system's backhaul is direct Modbus TCP — each ESP32 is a plain Modbus slave to its own PLC, with no bridge anywhere in the design, per the project's decision to keep ESP32-to-OpenPLC as a direct connection with ladder logic assigned per system.

| System | Field Layer | Backhaul |
|---|---|---|
| Elevator (BSH only) | GPIO/I2C (motor, buttons, LCD) | Modbus TCP (direct) |
| HVAC (FH only) | One-wire digital (DHT11 as built; DHT22 in Wokwi) + relay (compressor/fan) | Modbus TCP (direct) |
| Fire Alarm | GPIO (buzzer, RGB, button) | Modbus TCP (direct) |
| Door | **SPI** (RFID reader) | Modbus TCP (direct) |
| Power Box | Relay (lights + fan) | Modbus TCP (direct) |

**Why these choices:** The Door node's RFID reader (MFRC522/RC522) was chosen over the originally-planned Wiegand reader for easier ESP32 integration (plain SPI, not Wiegand's finicky pulse timing) while keeping the same insecure-legacy-access-control teaching point — a plain UID read has no authentication, and a MIFARE Classic card's UID is trivially clonable with cheap tools, contrasted against the option to upgrade to a modern authenticated scheme (e.g., OSDP, or an authenticated NFC tag) later. Power Box originally used DALI, the dedicated commercial lighting-control standard, for individually dimmable lighting; DALI was retired Sep 28, 2026 in favor of a plain relay (see Section 14), so Door's RFID reader is now the project's only dedicated field-bus protocol. Elevator, Fire Alarm, and Power Box don't have a dedicated field bus for their simple GPIO/analog/relay components, so they use plain wiring at the field layer and Modbus at the backhaul, same as Door.

This is a narrower slice of real-world protocol diversity than initially planned — BACnet (Elevator, Fire Alarm) and MQTT (Air Quality) were considered as the backhaul instead, since those are genuinely common in real building integration, but OpenPLC cannot poll either natively, so using them would have required a translation bridge between each ESP32 and its OpenPLC. The project team chose the simpler, bridge-free design to match the "Stability Over Complexity" principle; the BACnet/MQTT version remains fully documented in `Alternate_Design_BACnet_MQTT_Option.md` if reconsidered later.

---

## 6. Network / IP & PLC Map

**Revised Sep 22, 2026:** each OpenPLC instance now runs as its own Docker container, attached to the LAN over a macvlan interface so it gets a real, independent IP address — not a shared IP differentiated only by port, as in the original single-host design below. This is deliberate: the team wants Modbus port 502 to stay genuinely, realistically open per system for the cybersecurity scenarios in Section 11, and a shared-IP/custom-port scheme hides the real target port a Modbus-scanning tool expects by default. See the Sep 22, 2026 entry in Section 14 for the full reasoning, including why full per-system VMs were considered and rejected in favor of containers.

| System | ESP32 IP (field node) | OpenPLC Instance | OpenPLC Container IP | Web UI Port | Modbus Server Port |
|---|---|---|---|---|---|
| Proxmox server (`pve`, Proxmox VE 9.2.2) | - | - | 192.168.50.143 (host, Web UI :8006) | - | - |
| OpenPLC Docker VM (on Proxmox; hosts all 8 containers) | - | - | 192.168.50.10 (VM host) | - | - |
| SCADA/HMI VM (on Proxmox; Ignition) | - | - | 192.168.50.30 (VM 101 `scada-hmi`, built Oct 14, static IP set Oct 15) | - | - |
| BSH-Elevator | 192.168.50.11 | OpenPLC-BSH-Elevator | 192.168.50.111 | 8080 | 502 |
| BSH-FireAlarm | 192.168.50.12 | OpenPLC-BSH-FireAlarm | 192.168.50.112 | 8080 | 502 |
| BSH-Door | 192.168.50.13 | OpenPLC-BSH-Door | 192.168.50.113 | 8080 | 502 |
| BSH-PowerBox | 192.168.50.14 | OpenPLC-BSH-PowerBox | 192.168.50.114 | 8080 | 502 |
| FH-HVAC | 192.168.50.21 | OpenPLC-FH-HVAC | 192.168.50.121 | 8080 | 502 |
| FH-FireAlarm | 192.168.50.22 | OpenPLC-FH-FireAlarm | 192.168.50.122 | 8080 | 502 |
| FH-Door | 192.168.50.23 | OpenPLC-FH-Door | 192.168.50.123 | 8080 | 502 |
| FH-PowerBox | 192.168.50.24 | OpenPLC-FH-PowerBox | 192.168.50.124 | 8080 | 502 |

*(FH slot 1 is FH-HVAC as of Sep 26, 2026, folded into this table and every dependent doc. It keeps the retired FH-Elevator's IP, container IP, ports and OpenPLC instance slot — only the instance name changed. The retired `FH_Systems/01_FH-Elevator.md` is in `Archive/`.)*

Container IPs mirror their ESP32's own address plus 100 (e.g. BSH-Elevator's ESP32 is `.11`, its OpenPLC container is `.111`), so the pairing stays easy to read at a glance. Every instance now uses OpenPLC's own default ports (8080 Web UI, 502 Modbus) — instances are told apart by IP, not by a custom port anymore.

All 8 OpenPLC instances run as Docker containers inside one Proxmox VM (the OpenPLC Docker VM, 192.168.50.10) on the Proxmox server, not as one full VM per PLC — this keeps hardware cost the same as the original single-host design and avoids the RAM/CPU overhead of eight concurrent guest operating systems, while still giving every instance a genuine, independently-addressable network identity, including its own real port-502 Modbus server.

### Hosting layer: Proxmox (added Oct 9, 2026)

The "hub" is no longer a single physical machine. It is a **Proxmox VE server**, a dedicated PC that was a Windows machine and was wiped to run Proxmox directly on the hardware, connected by Ethernet cable to the lab router SCT5 (`pve`, Proxmox VE 9.2.2, `192.168.50.143`, Web UI on port 8006, operational as of Oct 9, 2026) that hosts as many virtual machines as the build needs:

| Proxmox VM | Role | IP |
|---|---|---|
| OpenPLC Docker VM | Docker host for the 8 OpenPLC containers (macvlan, `.111`-`.124`) | 192.168.50.10 |
| SCADA/HMI VM | Ignition SCADA layer (Maker Edition, company license pending), HMI, event log, cascade rules | 192.168.50.30 (VM 101) |
| Further VMs (optional) | e.g. attacker/analysis VM for Section 11 scenarios | TBD |

The containers-not-VMs decision of Sep 22 still stands for the PLCs themselves (one container per OpenPLC, not one VM per PLC). Proxmox adds VMs one level above that: a VM for the Docker host and a separate VM for SCADA/HMI. Keeping SCADA in its own VM also avoids the macvlan limitation that a Docker host cannot reach its own macvlan containers.

**Proxmox networking requirements for macvlan:** the VM NIC must sit on a bridge (`vmbr0`) attached to the wired LAN, the Proxmox firewall MAC filter must be off for that NIC, and the uplink must be wired Ethernet (see open item below).

> **Wired uplink (resolved Oct 9, 2026):** macvlan needs a wired Ethernet link, and the Proxmox server has one: a dedicated PC (previously a Windows machine, wiped) running Proxmox directly on the hardware, connected by Ethernet cable to the lab router SCT5. The `ipvlan`/USB-adapter fallback is not needed. **Still to verify at Phase C:** that the containers' extra MAC addresses pass through the `vmbr0` bridge and the router, by pinging each `.111`-`.124` from the laptop.

Full per-system pin maps, component lists, and point/register/object tables are documented separately in `BSH_Systems/`, `FH_Systems/`, and `BSH_Sensors/`/`FH_Sensors/` (component reference sheets — one set per building, mirroring each other component-for-component).

---

## 7. Cascade Event Flow

Because every system now reports to the SCADA layer through the same Modbus interface, a cascade is no longer a single fixed pipeline (as it was in the original BSH→FH design) — it's a rule the SCADA layer evaluates: **any system's point changing can trigger the SCADA layer to write a command into any other system's OpenPLC instance**, regardless of building, since all 8 instances expose the same direct Modbus server with no translation step in between.

**Example — Unauthorized Door Access (within BSH):**
```
1. BSH-Door: IR_DETECTED = 1 while ACCESS_GRANTED = 0 (approach with no valid card)
2. OpenPLC-BSH-Door exposes this mismatch on its Modbus server
3. SCADA (Ignition) polls it, evaluates the rule, flags an intrusion
4. SCADA writes Coil0 (ALARM_ACTIVATE) = 1 into OpenPLC-BSH-FireAlarm's
   Modbus server
5. OpenPLC-BSH-FireAlarm's ladder logic passes that coil straight through
   to the ESP32 over its own direct Modbus link
6. BSH-FireAlarm's buzzer and RGB LED activate; dashboard shows both nodes red
```

**Example — Campus-Wide Cascade (cross-building):**
```
1. Same BSH-Door intrusion as above
2. SCADA also writes ALARM_ACTIVATE = 1 into OpenPLC-FH-FireAlarm
3. Founder's Hall's fire alarm activates even though nothing physically
   happened there — demonstrating campus-wide situational awareness,
   not just single-building response
```

**Resilience Scenario (SCADA hub offline):**
Each system's ESP32 + dedicated OpenPLC pair keeps running its own local safe-state logic independently of the hub (e.g., BSH-Door's watchdog still locks to a safe state, BSH-Elevator still reports faults locally) — losing the SCADA layer degrades the *campus-wide* picture but does not take down any individual system, an improvement over the original design where the single hub was a single point of failure for both buildings' cross-building bridging.

---

## 8. Hardware Checklist

*(Quantities below are stated as totals across both buildings. The four mirrored systems need two of everything; Elevator is BSH-only and HVAC is FH-only, so each of those needs one.)*

### Central Hub / SCADA
- [x] Proxmox VE server (`pve`, 192.168.50.143; a dedicated former-Windows PC, wiped, running Proxmox directly) — hosts the OpenPLC Docker VM (8 OpenPLC containers, each its own macvlan IP, see Section 6) and the SCADA/HMI VM (Ignition); operational Oct 9, 2026. Replaces the earlier Raspberry Pi 4 / laptop hub plan
- [ ] (Optional, for pfSense in the stretch phase) a second network interface for the Proxmox PC, e.g. a USB Ethernet adapter; not needed until pfSense is built
- [ ] Dedicated Wi-Fi access point / router
- [x] Wired Ethernet link from the Proxmox server to the lab router (SCT5) — done Oct 9, 2026 (Ethernet cable, no Wi-Fi involved). macvlan networking (Section 6) needs a wired link; the `ipvlan`/USB-adapter fallback is no longer needed

### Per Elevator Node (×1 total — BSH only)
- [ ] ESP32 DevKit
- [ ] L293D motor driver + small DC motor
- [ ] 3× push buttons
- [ ] I2C 16x2 LCD

### Per HVAC Node (×1 total — FH only)
- [ ] ESP32 DevKit
- [x] DHT11 3-pin module (pull-up on module; DHT22 only in Wokwi) -- as built
- [ ] 1-channel relay module (compressor/fan simulation)
- [ ] RGB LED (common cathode) — idle / cooling / fault status
- [ ] Small DC fan, wired through the relay on its own supply (optional, gives the demo something visibly moving)

### Per Fire Alarm Node (×2 total)
- [ ] ESP32 DevKit
- [ ] Active buzzer
- [ ] RGB LED (common cathode)
- [ ] Push button (pull station)

### Per Door Node (×2 total)
- [ ] ESP32 DevKit
- [ ] SG90 micro servo + external 5V supply
- [ ] RGB LED
- [ ] IR obstacle sensor
- [ ] Magnetic reed switch (+ external ~10kΩ pull-up resistor — GPIO 34 has no internal one; see the firmware's HARDWARE CAVEAT comment)
- [ ] MFRC522/RC522 RFID reader module + at least one 13.56MHz card/fob

### Per Power Box Node (×2 total)
- [ ] ESP32 DevKit
- [ ] 2× 1-channel relay module (one per channel -- lights, fan; swapped from a single 2-channel module Sep 30, 2026, see Section 14)
- [ ] 3× LED (one per floor, lights channel) + 3× series resistor
- [ ] Small fan (DC motor + fan blade, or stand-in)
- [ ] 2× momentary push button (local lights/fan override switches)

### 3D-Printed Structures
- [ ] BSH building shell (PLA)
- [ ] Founder's Hall building shell (PLA)
- [ ] Campus base platform
- [ ] Crosswalk / street separator
- [ ] Smart traffic light structure
- [ ] Wire routing channels integrated into CAD

---

## 9. Firmware Development Workflow

Firmware is written **per system type**, each flashed twice with building-specific IPs — 8 sketches total (not 4) — three system types flashed twice with building-specific IPs, plus BSH-Elevator and FH-HVAC which each exist in one building only — since the project settled on separate BSH/FH `.ino` files (each named for its building and system, e.g. `BSH_FireAlarm_Node.ino`) rather than one shared sketch with a build-time flag. This trades a small amount of duplication for a lower risk of a student flashing the wrong IP into a shared file mid-session.

**Current build order (revised Sep 23, 2026 — `Firmware/WORKFLOW.md` Section 1 is authoritative):** Fire Alarm → Elevator → Door → Power Box, for BSH first and then the same order for FH. This replaces the earlier risk-ramp order (Fire Alarm → Power Box → Perimeter/Motion → Elevator → Door); see the Sep 23, 2026 entry in Section 14. (Slot 5 was originally Air Quality, then Perimeter/Motion, then Backup Power / ATS Monitor, and was dropped entirely on Oct 5, 2026 — see the September 14, September 28 and October 5 entries in Section 14.)

| Sketch (BSH / FH) | Status | Backhaul Library Needed |
|---|---|---|
| `BSH_FireAlarm_Node.ino` / `FH_FireAlarm_Node.ino` | ✅ First-pass firmware complete, now upgraded with flame + MQ-2 gas/smoke sensing (Sep 2026 — see Section 14). BSH's original pull-station-only version went through the full Phase A (standalone hardware)/B (Modbus)/C (OpenPLC+SCADA) pilot bring-up in `Firmware/BSH_FireAlarm_PILOT_Bringup.md`; the flame/gas addition has not yet had its own bench bring-up pass on either building and should not be assumed proven until it does. FH mirror written but not yet bench-tested. | ModbusIP_ESP8266 |
| `BSH_PowerBox_Node.ino` / `FH_PowerBox_Node.ino` | ✅ First-pass firmware complete. **BSH-PowerBox: Phase A (standalone bench test) complete (Oct 5, 2026)** -- both relays and both override buttons confirmed working via a combined standalone integration sketch; both relay boards confirmed active-HIGH (`RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` = `false`). Phase B (Modbus) not yet started. **FH-PowerBox: Phase A not yet started** -- its own two relay boards are untested; firmware keeps both flags at their `true` default until FH's own bench test. Hardware uses two separate 1-channel relay modules rather than one 2-channel module (see Section 14, Sep 30, 2026 entry); `RELAY_ACTIVE_LOW` split into independent `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` flags to match. No third-party library beyond Modbus. | ModbusIP_ESP8266 **Phase B (Oct 6, 2026): BSH-PowerBox bench-confirmed at 192.168.50.14; FH-PowerBox bench-confirmed at 192.168.50.24 (Oct 7, 2026), relay boards active-HIGH.** |
| `BSH_Elevator_Node.ino` (BSH only) | ✅ **Phase A bench-tested and confirmed working (2026-09-25)** — 3 floor buttons, OLED status display, and L293D/motor all verified together in one non-blocking integration sketch. Rewritten from the original non-networked `Elevator_Motor.ino` prototype into a non-blocking state machine (the prototype's `delay()`-based movement would have frozen Modbus polling for seconds at a time); dropped the prototype's undocumented 7-segment display. Also substituted an I2C OLED for the originally-planned 16x2 LCD. Full writeup: `Firmware/BSH-Elevator/SYSTEM_REPORT.md`. There is no FH mirror any more — FH slot 1 is HVAC (row below), and `FH_Elevator_Node.ino` is retired along with it. | ModbusIP_ESP8266 |
| `BSH_Door_Node.ino` / `FH_Door_Node.ino` | ✅ First-pass firmware complete. **BSH-Door Phase B bench-confirmed 2026-10-09 (.13); FH-Door Phase B bench-confirmed 2026-10-09 (.23).** Uses `MFRC522` from `miguelbalboa/rfid` (one of the most common, best-supported ESP32 RFID libraries) over the built-in `SPI` library — swapped in Sep 2026 from the original `WIEGAND` (`monkeyboard/Wiegand-Protocol-Library-for-Arduino`) implementation; see Section 14 for the swap rationale. `AUTHORIZED_CARDS` is a placeholder pending real badged card UIDs. Discovered the reed switch's documented GPIO 34 has no usable internal pull-up on the ESP32 (input-only ADC pin) — needs an external pull-up resistor; not caught until writing this firmware. | ModbusIP_ESP8266 + MFRC522 (`miguelbalboa/rfid`) + SPI |
| `FH_HVAC_Node.ino` | 🟨 **Phase A bench-confirmed 2026-10-07** (DHT11 + keyestudio relay active-HIGH + DC motor fan + RGB LED; fault test pending). **Phase B (WiFi + Modbus, 192.168.50.21) bench-confirmed 2026-10-07.** Decisions: tenths scaling, fail-off. Design doc: `FH_Systems/01_FH-HVAC.md`; report: `Firmware/FH-HVAC/SYSTEM_REPORT.md`. First use of a Holding Register as a master-set setpoint. | ModbusIP_ESP8266 + Adafruit DHT library |

**Steps per sketch:**
0. Simulate the system in Wokwi first (Phase A0 — `Firmware/WORKFLOW.md` Section 7): same pin map, same logic, stand-ins for parts Wokwi doesn't have. Learn the wiring and behavior before touching real parts
1. Confirm final GPIO pin assignment against the corresponding `BSH_Systems`/`FH_Systems` doc
2. Bring up the field-layer hardware in isolation (motor/buttons, buzzer/LED, RFID reader, relay channel, or analog sensor)
3. Bring up the Modbus slave library and confirm connectivity to its target OpenPLC instance (acting as Modbus master)
4. Implement the communication watchdog (3–5 second timeout → safe state)
5. Integrate field logic with backhaul reporting
6. Flash and test the BSH instance, then repeat with the FH instance's IP

**Key libraries to validate early:** Door's field-bus library risk dropped substantially with the Sep 2026 swap from Wiegand to MFRC522 (`miguelbalboa/rfid` is one of the most common, best-tested Arduino libraries for any RFID chip), and Door is now built third, since the SPI wiring and UID-list logic are still new-to-this-node even if the library itself is low-risk. Power Box dropped its own field-bus library entirely on Sep 28, 2026 — DALI was retired in favor of a plain relay for lighting (see Section 14) — so it now joins Elevator and Fire Alarm in needing no third-party library beyond `ModbusIP_ESP8266`, the lowest firmware risk of the four. **Build-order note (Sep 23, 2026):** the earlier revision built Door last so the Modbus/watchdog/state-machine pattern would be proven four times first. The current order builds Door third, after Fire Alarm and Elevator — so that pattern has been proven twice by then, and the Wokwi simulation step (Phase A0, `Firmware/WORKFLOW.md` Section 7) now surfaces SPI/RFID logic problems before real hardware is involved.

**What "first-pass complete" means, and what's still needed before Week 4 integration:** every sketch above compiles against the documented register map and pin map and follows the project's non-blocking Modbus-slave template, but **none of the three post-pilot systems have been flashed to real hardware yet.** Per Section 10, Phase A (standalone hardware)/B (Modbus)/C (OpenPLC+SCADA) bring-up — proven on Fire Alarm — still needs to happen for Power Box, Elevator, and Door before they can be considered done. Expect the placeholders called out in the status table above (authorized card UIDs) to be exactly where Phase A testing surfaces the first real issues.

---

## 10. Dashboard & Integration Workflow

### Platform decisions and options (updated Oct 12, 2026)

Options are kept here so nobody re-evaluates them. Sources and licence notes are in `References/INDEX.md`.

**SCADA/HMI software** (runs in the SCADA/HMI VM on Proxmox)

| Option | Licence | Fit | Status |
|---|---|---|---|
| **Ignition** (Inductive Automation, Maker Edition) | Closed source; free for non-commercial and personal educational use; 10,000 tags, 10 Perspective sessions | Most industry-realistic; Modbus and OPC UA drivers and the Perspective web HMI included | **Chosen.** Professor approved it Oct 12, 2026 and submitted the application; **company license pending** |
| Rapid SCADA | Open-source core; free Community edition, no data-point limit | Real SCADA, documented OpenPLC Modbus setup | **Fallback #1** if the Ignition license is denied or late |
| FUXA | MIT | Modern web HMI, Docker install; alarms/scripting/roles not yet confirmed | **Fallback #2** |
| ScadaBR / Scada-LTS | GPL | ScadaBR's last release was 2021 (Java 8) | Not chosen |
| Node-RED | Apache 2.0 | The earlier plan; not what plants run as SCADA | Replaced by Ignition. Last-resort option only |

Fallback rule: if the license has not arrived by **Oct 21, 2026** (checkpoint moved from Oct 14 on Oct 14, 2026), start on FUXA or Rapid SCADA in the SCADA VM and switch to Ignition only once it is licensed. Ignition normally has a time-limited trial mode (confirm the current terms before relying on it).

**pfSense (firewall, segmentation, intrusion detection)**

| Option | What it means | Status |
|---|---|---|
| Up front | Build pfSense first, split zones, renumber `192.168.50.x`. About one extra week and extra risk to the core build | Not chosen |
| **Stretch goal, after the core works** | Add pfSense in weeks 6-7 of the original plan (about Nov 16-29, 2026) | **Chosen** |
| Not used | Flat network only | Fallback |

When it is added there are two insertion styles: a *transparent (bridge) firewall*, which keeps every IP and is the low-effort option, or *routed segmentation* with separate subnets, which is more realistic but needs renumbering. The ESP32s stay on the SCT5 Wi-Fi in either case, outside pfSense.

Prep rules now, so adding pfSense later takes a day or two: (1) make `vmbr0` VLAN-aware, (2) keep every IP in one place and use hostnames where possible, (3) keep the Docker host and SCADA in separate VMs, (4) keep each ESP32's IP as one constant at the top of its sketch, (5) leave a second network interface available.

**VPN for remote work**

| Option | Notes | Status |
|---|---|---|
| **Tailscale** (subnet router in a small VM/LXC on Proxmox advertising `192.168.50.0/24`) | Outbound-only, works behind the lab router SCT5 where ports cannot be forwarded | **Chosen, first** |
| WireGuard or OpenVPN on pfSense | More industrial, but needs an inbound port; SCT5 cannot forward one | Later, with pfSense, only if reachable some other way |
| None | Work only in the lab | Fallback |

Rules: never expose the Proxmox web UI (port 8006) to the internet, reach it only through the VPN; Modbus has no authentication, so the VPN is the only remote protection; check the lab's rules on running a VPN on its network.

**Target schedule (accelerated, set Oct 12, 2026; a target, not a commitment)**

| Dates | Goal |
|---|---|
| Oct 12-15 | Proxmox VMs; first OpenPLC container (BSH-FireAlarm); verify the address convention. license checkpoint moved to Oct 21 |
| Oct 16-19 | Remaining 7 containers and program tests; install the SCADA gateway |
| Oct 20-23 | Tags for all 8 nodes, status colors, heartbeat, event log, campus map |
| Oct 24-27 | Cascade rules, within each building then cross-building |
| Oct 28-31 | Attack scenarios and detection, resilience test, rehearsal, doc updates |
| Nov 1 - Dec 11 | Buffer; pfSense and VPN (stretch); report and deliverables. Final demo Dec 11 |

Protect the cascade and integration days (Oct 24-27); that is where schedules slip.

### Hub Setup
0. [ ] Proxmox: create the OpenPLC Docker VM (192.168.50.10, NIC on `vmbr0`, firewall MAC filter off) and the SCADA/HMI VM; on the Proxmox host, disable the enterprise repos and enable `no-subscription` so `apt update` stops failing
1. [x] (done Oct 13-14, 2026) Install Docker in the OpenPLC Docker VM; deploy 8 separate OpenPLC containers, each attached via macvlan to its own IP per Section 6 (every instance uses the same default 8080 Web UI / 502 Modbus ports now — instances are told apart by IP, not by port)
2. [ ] Install Ignition (Maker Edition; company license pending, see "Platform decisions" below) in the SCADA/HMI VM

### Per-System OpenPLC Configuration
1. [ ] For all 4 systems: add the corresponding ESP32 as a Generic Modbus TCP Slave Device in that system's own OpenPLC instance — identical configuration pattern for all 8 nodes, no bridges to build
2. [ ] Write the per-system ladder logic in each instance (safe-state behavior, and passing SCADA-written coils like `ALARM_ACTIVATE` through to the ESP32). Each system has a folder in `PLC_Logic/` (ladder diagram, register map, Structured Text program, build log); drafted Oct 11, 2026, not yet run

### SCADA (Ignition) Dashboard
1. [ ] Add a Modbus TCP device connection per OpenPLC instance (8 total)
2. [ ] Build the campus map layout — both buildings, all 4 systems each
3. [ ] Add per-system status indicators (Green/Yellow/Red)
   - Drive each node's state from its `HEARTBEAT` Input Register (see Section 14, Oct 6, 2026): frozen for about 3s = yellow, about 5s = red, resumed = blue briefly, then green. Log every lost/reconnected event with a timestamp and downtime, using `RECONNECT_COUNT` to say Wi-Fi drop vs reboot. The OpenPLC Input Register count for each node must include the two heartbeat registers.
4. [ ] Add the event log panel
5. [ ] Build the cascade rules described in Section 7
6. [ ] Test each cascade path end-to-end, within-building and cross-building

### Dashboard Color Coding
| Color | Meaning |
|---|---|
| 🟢 Green | Normal operation |
| 🟡 Yellow | Warning / degraded communication |
| 🔴 Red | Intrusion / compromised |
| 🔵 Blue | Active communication |

---

## 11. Cybersecurity Scenario Workflows

### Scenario 1 — Normal Campus Operation
All 8 nodes report green; SCADA shows both buildings healthy.

### Scenario 2 — Authorized Door Access
A valid RFID card read sets `ACCESS_GRANTED`, the gate opens, the event is logged with no alarm.

### Scenario 3 — RFID UID Clone/Replay Attack
A cloned or replayed card UID opens the door with a stale or duplicated credential. Detection: compare `ACCESS_GRANTED` timing/frequency against expected use patterns — a textbook demonstration of a plain RFID UID read's lack of authentication (MIFARE Classic UIDs are clonable with widely available ~$10-15 tools; this scenario was originally framed around Wiegand's identical lack of replay protection before the Sep 2026 reader swap — see Section 14).

### Scenario 4 — False Fire Alarm Injection (Modbus)
An attacker on the network issues an unauthenticated Modbus write directly to `Coil0` (`ALARM_ACTIVATE`) on a Fire Alarm node's OpenPLC, triggering the alarm with no corresponding `MANUAL_PULL_TRIGGERED`. Detection: the mismatch between the two inputs — and a broader discussion point, since standard Modbus TCP has no built-in authentication at all.

### Scenario 5 — Elevator Lockout (BSH only)
`ELEVATOR_DISABLE` is set remotely with no maintenance justification. Dashboard distinguishes this from a genuine `ELEVATOR_FAULT`. BSH-only, since FH has no elevator.

### Scenario 6 — Backup Power ATS Manipulation (Modbus) — DROPPED Oct 5, 2026
*The Backup Power system was dropped (see Section 14, Oct 5, 2026), so this scenario is no longer part of the demonstration. The original text is kept below for reference only.*

An attacker writes `1` directly into `Coil0` (`ATS_TRANSFER_CMD`) on the Backup Power node's OpenPLC, forcing a transfer test during normal operation with no real utility outage — or writes a fabricated value into `DI0` (`UTILITY_POWER_LOST`) to mask a real outage, or into `IR0`/`IR1` (`UPS_BATTERY_PCT`/`LOAD_CURRENT_MA`) to hide a draining battery or an overloaded circuit. Discussion point: comparing this to how MQTT-based sensor spoofing would work if that backhaul were used instead (see `Alternate_Design_BACnet_MQTT_Option.md`), and to `ATS_TRANSFER_CMD`'s own fail-direction design — the opposite asymmetry from the retired Perimeter node's `ZONE_DISABLE` (see `BSH_Systems/05_BSH-BackupPower.md`).

### Scenario 7 — Power Box Manipulation
An unauthorized Modbus write flips the lights or the fan with no scheduled/logged source — demonstrates that even a "boring" system (lighting) is a viable attack target.

### Scenario 8 — Cross-Building Cascade
An intrusion at one building's Door system cascades to both buildings' Fire Alarm systems through the SCADA layer, demonstrating campus-wide situational awareness (see Section 7).

### Scenario 9 — Node Isolation / Resilience
One OpenPLC instance is stopped; SCADA flags only that specific system offline while the other 7 continue reporting normally — demonstrating partial system availability rather than total failure.

### Scenario 10 — Red/Blue Team Exercise
**Red Team:** attempt the RFID UID clone/replay (Scenario 3) and the direct Modbus coil/register injection attacks against Fire Alarm, Elevator, and Power Box (Scenarios 4–7) above.
**Blue Team:** monitor the SCADA dashboard for the mismatches described in each scenario and identify the attack in progress.

### Scenario 11 — HVAC Sensor Spoofing / Setpoint Tampering (FH only)
An attacker writes a fabricated `TEMP_C` into FH-HVAC's input register, or rewrites `TARGET_TEMP_C` (HR0), making the zone run its compressor against a temperature that was never measured — or holding it off while the zone genuinely overheats. Detection: `COMPRESSOR_RUNNING` staying true while `TEMP_C` never moves toward `TARGET_TEMP_C`, which is precisely the correlation the SCADA layer is there to do. This is the project's cleanest analog false-data-injection demo, and the reason FH slot 1 was changed from Elevator to HVAC.

---

## 12. Weekly Milestones

The original 12-week milestone table (Aug 24 – Dec 11) still applies as the overall project cadence. This architecture revision happened during **Week 3 (Sep 7–11), Hub Integration** — before any BSH firmware or hub configuration was actually completed, so no prior work is being discarded. The Week 3–4 goals now specifically mean: stand up the 8-instance, direct-Modbus OpenPLC architecture described in Section 10, rather than the single-hub version originally planned.

*(See `Piranga_Workflow.md` Section 10 for the full week-by-week table — still valid as written; only the technical content of "Hub Integration" and "Cascade Logic" changes.)*

---

## 13. Deliverables Tracker

| # | Deliverable | Status | Notes |
|---|---|---|---|
| 1 | Functional 3D-printed smart campus model | ⬜ Not started | BSH + FH buildings |
| 2 | 8 ESP32 nodes (firmware + wiring), 4 per building | 🟩 Firmware complete and bench-confirmed through Phase B on all 8 nodes (Oct 6-9, 2026): BSH Elevator, Fire Alarm, Door, PowerBox and FH HVAC, Fire Alarm, Door, PowerBox, each a Modbus TCP slave with heartbeat and reconnect counters, verified from a laptop. Phase C (OpenPLC and SCADA) started Oct 13, 2026: OpenPLC Docker VM, image, macvlan network and all 8 containers are running (Oct 14), no program loaded yet, SCADA VM (VM 101) built Oct 14 but no SCADA software yet | See Section 9 |
| 3 | 8 OpenPLC instances configured in the Proxmox OpenPLC Docker VM | ⬜ Not started | See Section 10 |
| 4 | Ladder logic programs for all 8 OpenPLC instances | 🟨 Drafted, untested (Oct 11, 2026) | Safe-state behavior + SCADA-coil pass-through, see Section 10. Files in `PLC_Logic/` (one folder per system) |
| 5 | SCADA dashboard with cross-system cascade rules | ⬜ Not started | Ignition (Maker Edition; professor approved Oct 12, 2026, company license pending). See Sections 7 and 10 |
| 6 | Demonstration of 3+ security scenarios | ⬜ Not started | 10 candidate scenarios documented (Scenario 6 since dropped, Oct 5, 2026). Network evidence captured with Wireshark on a Linux machine per run; procedure, filters and per-run log live in `Security_Capture/` (added Oct 14, 2026) |
| 7 | System wiring and architecture diagrams | ⬜ Not started | |
| 8 | CAD / 3D model files | ⬜ Not started | Fusion 360 |
| 9 | Final technical presentation/poster | ⬜ Not started | |
| 10 | Final project report/documentation | ⬜ Not started | |

---

## 14. Lessons Learned & Decisions Log

> **Frozen as history on Oct 15, 2026 (last entry: Oct 14, 2026).** New decisions and lessons live in `Logs/`, one file per day (`Logs/YYYY-MM-DD.md`, index in `Logs/INDEX.md`, rules in `Logs/README.md`). The rows below are not moved or edited, because many documents cite them as "Section 14, Oct 6 entry".

| Date | Decision | Rationale |
|---|---|---|
| Sep 2, 2026 | Use two protocols: BSH = Modbus TCP, FH = MQTT | Enables side-by-side protocol comparison (superseded below) |
| Sep 2, 2026 | Bridge buildings through central hub (Option 1) | Most realistic architecture; enables cascade event scenario |
| Sep 2, 2026 | Selected BSH and Founder's Hall as the two buildings | Matches whiteboard layout and project scope |
| Sep 8, 2026 | Split BSH into 5 independent ESP32 systems (Elevator, Fire Alarm, Door, Power Box, Air Quality) | Mirrors real building integration (separate vendor systems per function) more closely than one node per building |
| Sep 8, 2026 | Moved from per-building to per-system protocol comparison; each system gets its own dedicated OpenPLC instance, unified by a SCADA layer | More realistic (Purdue Model pattern) and a stronger security teaching narrative; accepted trade-off of higher integration complexity |
| Sep 8, 2026 | Assigned field + backhaul protocols per system: BACnet/IP (Elevator, Fire Alarm), Modbus TCP (Door w/ Wiegand field bus, Power Box w/ DALI field bus), MQTT (Air Quality) | Chosen for real-world commonality per system type, verified ESP32 library feasibility, and reused BACnet across two systems to limit total protocol stacks to build (superseded by the entry below) |
| Sep 8, 2026 | Founder's Hall mirrors BSH exactly (same 5 systems, same protocols) rather than using contrasting protocols per system | Simpler integration path; keeps the comparison axis on system-vs-system rather than building-vs-building |
| Sep 8, 2026 | Locked all 10 systems to a direct Modbus TCP backhaul (each ESP32 a plain Modbus slave to its own dedicated OpenPLC, no bridges anywhere) | OpenPLC's polling engine only speaks Modbus natively — a direct ESP32-to-OpenPLC connection with per-system ladder logic can only be Modbus. Protocol diversity is preserved instead at the field-bus layer (Wiegand, DALI). The previously designed BACnet/MQTT-with-bridge version is archived in `Alternate_Design_BACnet_MQTT_Option.md` for possible future use |
| Sep 9, 2026 | Built the BSH-FireAlarm pilot first, with a dedicated Phase A/B/C bring-up checklist, before touching any other node | Proves the whole ESP32 -> OpenPLC -> SCADA chain end-to-end on the lowest-hardware-complexity node before committing time to the other nine — isolates "is the architecture sound" from "does this specific sensor/library work" |
| Sep 10, 2026 | Firmware build order set to Fire Alarm → Power Box → Air Quality → Elevator → Door (pilot already done; then simplest-remaining first, both field-bus/library-risk nodes — Door's Wiegand and Power Box's DALI — deliberately not adjacent, with Door pushed all the way to last) | By the time Door's firmware is written, the Modbus/watchdog/non-blocking-state-machine pattern has been proven on four other nodes, so Wiegand bring-up is the only genuinely new variable in play. Power Box (DALI) goes second rather than last since it's the *other* named high-risk node and the docs already flagged prototyping it in parallel with the pilot as worthwhile |
| Sep 10, 2026 | Rewrote the original `Elevator_Motor.ino` prototype (delay()-blocking, no Modbus, includes an undocumented 7-segment display) into `BSH_Elevator_Node.ino`/`FH_Elevator_Node.ino` as a non-blocking state machine matching the documented register map, and dropped the 7-segment display | The prototype's multi-second `delay()` calls during door-close and travel would have frozen `mb.task()` and made the node look like a dead/unresponsive Modbus slave to OpenPLC; the 7-segment display isn't in the documented BOM or pin map for this node and its GPIOs aren't reserved anywhere |
| Sep 10, 2026 | Added a locally-detected `ELEVATOR_FAULT` (stall-timeout watchdog on the movement state) that latches until reset, kept explicitly separate from `ELEVATOR_DISABLE` | The system doc's own "Notes" call for these two to stay distinguishable (a genuine fault vs. a remote lockout) — a fault that only ever gets *set* remotely wouldn't demonstrate that distinction |
| Sep 10, 2026 | Discovered GPIO 34 (documented reed-switch pin, `BSH_Sensors/08_Reed_Switch.md`) has no usable internal pull-up on the ESP32 — it's one of the input-only ADC pins (34–39) that ship with no internal pull resistors in silicon | `INPUT_PULLUP` on GPIO 34 compiles without error but silently does nothing, which would have produced an intermittently floating `DOOR_POSITION` reading. Firmware now uses plain `INPUT` with a comment calling for an external ~10kΩ pull-up resistor; added to the Section 8 hardware checklist. Worth a similar audit of any other GPIO 34–39 usage across the other nodes before bring-up |
| Sep 10, 2026 | Door node firmware reports `IR_DETECTED`, `DOOR_POSITION`, and `ACCESS_GRANTED` as three independent, honestly-reported values and does not attempt to locally infer "this looks like an intrusion" | Section 7 (Cascade Event Flow) already assigns that correlation job to the SCADA/OpenPLC layer, which can see all 10 nodes' state — a single ESP32 trying to make the same judgment with only its own three inputs would be duplicating (and could contradict) that layer |
| Sep 10, 2026 | DALI and Wiegand library calls were verified against each library's own public README/example (`silahcen001/DALI-ESP32`'s `DaliController` class; `monkeyboard/Wiegand-Protocol-Library-for-Arduino`'s `WIEGAND` class) rather than hand-written bit-banged protocol timing | DALI's Manchester-encoded physical layer is timing-critical (416.7µs half-bit periods) and unreliable to bit-bang correctly under an active WiFi stack without hardware-in-the-loop testing to verify; using the RMT-peripheral-backed library is both more realistic to how a real DALI master would be built and safer to hand over untested. Flagged in Section 9 as still needing real hardware bring-up regardless |
| Sep 14, 2026 | Retired the Air Quality system (BSH/FH, slot 5) and replaced it with a new Perimeter/Motion Intrusion Zone system, reusing the exact same IPs, OpenPLC instance names, and Web UI/Modbus ports (192.168.50.15/.25, ports 8085/5025 and 8095/5035) | Frees up the Fire Alarm system to be properly rebuilt later as a real flame/smoke/gas detection node (combining flame + smoke + gas sensing, rather than the current single-buzzer/RGB pull-station design) without that upgrade also needing to duplicate air-quality monitoring. The Perimeter/Motion system was chosen from several candidates (see the discussion that led here) as the best fit for the campus's already-planned crosswalk/pathway zone and an unused Hardware Checklist line. This is a separate, not-yet-executed follow-up — only the Air Quality → Perimeter/Motion swap was done in this pass |
| Sep 14, 2026 | Perimeter/Motion uses two independent sensors — a PIR motion sensor (`MOTION_DETECTED`, DI0) and an HC-SR04 ultrasonic ranger (`OBJECT_IN_RANGE`/DI1, `DISTANCE_CM`/IR0) — reported honestly and separately, with no local intrusion judgment, plus a `ZONE_DISABLE` coil (Coil0) | Matches the design precedent already set by BSH-Door (report raw truth, let SCADA correlate — see Section 7). `ZONE_DISABLE` deliberately fails toward "still enabled/reporting" on comm loss, the opposite fail-direction from `ELEVATOR_DISABLE`/other lockout coils — a security sensor going silent by default when it loses its PLC link would be the wrong failure mode. Worth calling out explicitly in any presentation of this node, since it's the one coil in the whole project that intentionally fails "open" rather than "safe-closed" |
| Sep 14, 2026 | The HC-SR04's `ECHO` pin is read with a bounded `pulseIn()` call (25ms timeout), issued once per 150ms sample interval rather than every loop iteration, instead of a fully non-blocking interrupt-driven read | A true interrupt-driven echo measurement would be more consistent with the project's "no blocking calls" firmware template (see the Elevator rewrite entry above), but a short, explicitly bounded `pulseIn()` is a well-precedented, simple pattern for this specific sensor and is unlikely to meaningfully disrupt Modbus TCP polling given typical poll intervals. Flagged as unproven on real hardware — if Phase B bring-up shows Modbus timeout symptoms on this node specifically, switch to an interrupt-based read |
| Sep 14, 2026 | `BSH_Sensors/10_MQ135_Air_Quality_Sensor.md` was annotated as currently unused rather than deleted | The sensor may be reused if the planned Fire Alarm flame/smoke/gas upgrade adopts it alongside a dedicated flame sensor — keeping the reference doc avoids re-researching wiring/specs already worked out once |
| Sep 14, 2026 | Local sync limitation discovered: `device_bash` still cannot mount the shared CyberCity folder (a known Windows-update-related issue, not a permissions problem), so the old AirQuality files/folders could not be deleted from the local copy in this pass — only overwritten with a deprecation notice pointing to the new Perimeter files | The device bridge's write path (`device_commit_files`) has no delete operation; until the mount issue is fixed, removing stale local files requires either the user deleting them by hand or a future working `device_bash` session with delete permission granted |
| Sep 28, 2026 | Retired the Perimeter/Motion Intrusion Zone system (BSH/FH, slot 5) and replaced it with a new Backup Power / ATS Monitor system, reusing the exact same IPs, OpenPLC instance names, and Web UI/Modbus ports (192.168.50.15/.115 and .25/.125, ports 8080/502) | Perimeter/Motion was a pure sensor-only node with no actuator, which read thin next to the other four systems' genuine actuators (elevator motor, fire alarm buzzer/RGB, door servo, power box relay/DALI). Backup Power gives the project a real actuator (`ATS_TRANSFER_CMD`) and a genuinely deployed OT-security attack surface (forcing a false transfer test, running down a UPS battery unnoticed, blocking a generator start during a real outage) that a generic motion zone didn't provide. Several other real-world candidates were discussed (smart parking, water leak detection, a pan-tilt camera node, restroom occupancy, a pedestrian crosswalk signal, and others); the user chose Backup Power / ATS Monitor as the strongest fit for a *cybersecurity* testbed specifically |
| Sep 28, 2026 | A pedestrian crosswalk / walk-signal system (call button, RGB walk/don't-walk, IR beam for pedestrian presence) was discussed as a strong alternative — it would reuse the campus's already-planned crosswalk/pathway zone even more literally than Perimeter/Motion did (see the Sep 14, 2026 entry above) — but was deliberately deferred rather than adopted now | The user liked the idea but flagged it as extra work better tackled after the rest of the project is done, not folded into this pass's swap. Noted here so it isn't lost — a candidate for a future sixth system or a second slot-5 revision |
| Sep 28, 2026 | Backup Power / ATS Monitor's register map: `UTILITY_POWER_LOST` (DI0) is raw from a toggle switch; `LOW_BATTERY` (DI1) is derived from a fixed threshold on `UPS_BATTERY_PCT` (IR0), which keeps reporting its real value regardless; `LOAD_CURRENT_MA` (IR1) is raw from an ACS712; `ATS_TRANSFER_CMD` (Coil0) is open-loop, with no feedback Discrete Input wired yet | The raw/derived split for DI1 mirrors the retired Perimeter node's `OBJECT_IN_RANGE`-from-`DISTANCE_CM` pattern; the open-loop coil mirrors BSH-PowerBox's `LIGHTS_CMD`/`FAN_CMD`. `ATS_TRANSFER_CMD`'s fail-on-comm-loss direction is deliberately the opposite of the retired `ZONE_DISABLE`'s: it defaults to *not* commanded, since an uncommanded transfer test during a real outage (not a security sensor going quiet) is the bad outcome here — worth calling out explicitly in any presentation of this node, same as `ZONE_DISABLE` was |
| Sep 28, 2026 | Moved the retired system docs, sensor docs, bring-up checklists, and the `Firmware/BSH-Perimeter/` build folder (including its preliminary PIR/HC-SR04 `BenchTest.ino`/`Wiring.md`/`Build_Log.md` files, written ahead of this system's turn in the build order) into `Archive/`, with new rows in `Archive/README.md` | Same pattern as every prior retirement in this project. Also demonstrates that the local sync limitation noted in the Sep 14, 2026 entry above (`device_bash` couldn't mount the shared folder at all back then) is now resolved — this pass moved every retired file on local disk directly, with no deprecation-stub workaround needed |
| Sep 14, 2026 | Added `FH_Sensors/` as a full mirror of `BSH_Sensors/` (all 14 component reference docs, "Used in" and pin-table headers swapped from BSH-X to FH-X) instead of leaving the component docs in one shared `BSH_Sensors/` folder referenced by both buildings | Keeps the sensor-reference layer consistent with every other folder pair in the project (`BSH_Systems`/`FH_Systems`, per-building firmware, per-building build folders) — one location per building, same as everywhere else, even though the underlying components and wiring are identical. Also dropped the "(and its Founder's Hall mirror)" asides from `BSH_Sensors/11_Wiegand_Card_Reader.md` and `12_DALI_LED_Driver.md` now that FH has its own copies, and repointed every FH `.ino` file's Hardware comment block from `BSH_Sensors/...` to `FH_Sensors/...` |
| Sep 15, 2026 | Executed the previously-deferred "Fire Alarm done properly" upgrade: BSH/FH-FireAlarm now add a digital flame sensor (`FLAME_DETECTED`, DI1) and an MQ-2 analog gas/smoke sensor (`GAS_SMOKE_ALERT`/DI2, `GAS_RAW_VALUE`/IR0) on top of the existing pull-station button and remote coil, all OR'd into the same local `alarmActive` output logic | Two candidate open-source ESP32 fire-alarm projects were reviewed (`JordyMail/Fire-and-Smoke-Detection-ESP32` — DHT22/MQ7/MQ135/dust sensor over WiFi+MQTT to a public broker; `bugsNburgers/IOT-Fire-Alarm-System` — flame sensor + MQ-2 + DHT11 over WiFi+Blynk cloud, plus a USB-serial webcam capture script). Neither was adopted as-is: both route through an external cloud/broker dependency, which is inconsistent with every other node's direct-Modbus-to-OpenPLC architecture and a poor fit for a security testbed specifically trying to demonstrate resilience without relying on outside infrastructure; `bugsNburgers`'s webcam/serial piece also assumes a permanently-tethered desktop, which the embedded, 3D-printed building can't provide. Instead, the team adopted `bugsNburgers`'s sensor selection and GPIO wiring (flame sensor on GPIO 18 active-LOW, MQ-2 AOUT on GPIO 34) largely unchanged, since wiring doesn't depend on what's polling the ESP32, and rebuilt only the software layer as a Modbus TCP register extension matching the pattern every other node already uses |
| Sep 15, 2026 | Chose MQ-2 over the MQ-135 previously flagged as a maybe-reuse for this exact upgrade (see the Sep 14 entry above); the old `BSH_Sensors/10_MQ135_Air_Quality_Sensor.md` / `FH_Sensors/10_MQ135_Air_Quality_Sensor.md` docs were retired (deleted from the Project; local copies overwritten with a deprecation stub, same limitation as the AirQuality retirement below) and replaced by new `15_Flame_Sensor.md` / `16_MQ-2_Gas_Smoke_Sensor.md` docs in both `BSH_Sensors/` and `FH_Sensors/` | MQ-135 is tuned for general air-quality gases (NH3, NOx, alcohol, benzene, CO2) and only detects smoke as a side effect; MQ-2 is purpose-built for combustible gas/LPG/smoke, which is what a fire alarm actually needs. The two chips aren't interchangeable, so the doc was retired rather than repurposed |
| Sep 15, 2026 | Documented a hardware caveat the `bugsNburgers` reference omitted entirely: the ESP32's ADC pins are only 3.3V-tolerant, but many MQ-2 breakout boards run their heater at 5V and can swing their AOUT close to that rail depending on the specific board | Wiring a 5V-swinging AOUT directly into GPIO 34 risks damaging the pin. `16_MQ-2_Gas_Smoke_Sensor.md` now calls for measuring AOUT with a multimeter before wiring and adding a voltage divider (same 1kΩ/2kΩ pattern already used for the HC-SR04's ECHO pin) if it exceeds 3.3V — flagged here since it's exactly the kind of gap that's easy to copy straight from a reference project without noticing |
| Sep 15, 2026 | Replaced the `bugsNburgers` firmware's fixed magic-number gas threshold (`GAS_THRESHOLD = 1500`, with its own comments inconsistently referencing 2150) with a boot-time baseline calibration (`gasBaseline + GAS_ALERT_MARGIN`) | A fixed threshold tuned on one sensor unit in one room is fragile across different units/rooms/humidity; sampling 20 readings in clean air at boot (same sample count/approach as the reference) and alerting relative to that baseline tolerates more variation. `GAS_ALERT_MARGIN` is still a placeholder (400) pending real bench calibration, and the "baseline only computed once, at boot" limitation carries over unchanged — power on in clean air |
| Sep 15, 2026 | Swapped the Door node's card reader from a Wiegand reader to an MFRC522/RC522 RFID module (SPI), on both BSH-Door and FH-Door — register map unchanged (`ACCESS_GRANTED`/DI2 still means the same thing); GPIO 32/33 repurposed from Wiegand D0/D1 to RFID SS/RST, and the ESP32's default VSPI pins (18/19/23) are used for SCK/MISO/MOSI, the first use of that bus on this node | The user found real Wiegand and RC522 reader hardware to compare and preferred the RC522 module. Beyond preference, the swap also lowers integration risk: MFRC522 (`miguelbalboa/rfid`) is one of the most common, best-supported ESP32 RFID libraries, versus Wiegand's comparatively niche library support and finicky pulse-timing requirements. The teaching narrative survives the swap — a plain RFID UID read has no authentication, and MIFARE Classic's UID is trivially clonable with cheap tools, which is arguably a more concrete, more replicable cybersecurity demo than sniffing raw Wiegand pulses. Scenario 3 was renamed from "Wiegand Replay Attack" to "RFID UID Clone/Replay Attack" to match |
| Sep 15, 2026 | While rewriting `FH_Door_Node.ino` for the RFID swap, fixed a pre-existing bug: its static IP was hardcoded to `192.168.50.13` (BSH-Door's address) instead of this node's own documented `192.168.50.23` | Both Door nodes would have collided on the same static IP had this reached real hardware unnoticed — `FH_Systems/03_FH-Door.md` and Section 6 of this document always specified `.23`, so this was a copy-paste error from the BSH file, not a design decision. Worth an audit of the other nine `.ino` files' hardcoded IPs against Section 6 as a precaution, since this one went unnoticed since Sep 10 |
| Sep 22, 2026 | Moved OpenPLC from 10 processes sharing the hub's single IP (differentiated by port, e.g. `.10:5021`) to 10 separate Docker containers, each on its own macvlan IP (`.111`–`.125`), all using OpenPLC's default Web UI port (8080) and the canonical Modbus port (502) | The team wants Modbus port 502 to stay genuinely open per system for the cybersecurity scenarios in Section 11 — the old shared-IP/custom-port scheme (e.g. `.10:5021`) hides the real target port a Modbus-scanning tool expects by default, which weakens the realism of the exercise. Full per-system VMs were considered instead of containers and rejected: OpenPLC's official VM image is x86-only and won't run on an ARM Raspberry Pi 4 hub without slow emulation, and ten concurrent full guest OSes risk exactly the demo-day instability the project's "Stability Over Complexity" principle warns against. Containers give the same "one real IP, real port 502, per system" result at a fraction of the resource cost. **Open item:** macvlan (how a container gets a real LAN IP) typically needs a wired Ethernet link from the hub to the router — most Wi-Fi drivers don't support it; if the hub is Wi-Fi-only this needs Docker's `ipvlan` L3 mode instead, or a USB Ethernet adapter. Needs verification against whichever hub hardware is actually used |
| Sep 23, 2026 | Firmware build order changed to Fire Alarm → Elevator → Door → Power Box → Perimeter/Motion, applied identically to BSH (Phase 1) and FH (Phase 2); `Firmware/WORKFLOW.md` Section 1, its status tracker, every system README, and every bring-up checklist updated to match | Team decision. Elevator (GPIO/I2C only, no field-bus library) moves up as a low-risk second step after the Fire Alarm pilot; Door now comes third rather than last, with the new Wokwi simulation step (Phase A0, see the next entry) covering the RFID/SPI logic risk that previously justified building it last |
| Sep 23, 2026 | Added Phase A0 — Wokwi simulation — before every system's Phase A bench build (`Firmware/WORKFLOW.md` Section 7). Each BSH system gets a `Firmware/<System>/Wokwi/` folder (`diagram.json`, `sketch.ino`, `README.md`); FH reuses BSH's. Stand-ins are documented for parts Wokwi lacks (L293D/DC motor, IR obstacle sensor, reed switch, DALI, flame sensor). BSH-FireAlarm's simulation was built first, retroactively, as the template | Lets the team learn each system's wiring and sensor behavior, and catch logic bugs, before real parts are involved — cheaper and safer than debugging on the bench. Explicitly limited to Phase A: Wokwi does not model voltage levels, pull-ups, or warm-up, so every bench hardware caveat still applies, and a simulated ESP32 can't join the hub's LAN for Phase B/C |
| Sep 25, 2026 | BSH-Elevator reached Phase A complete — 3 floor buttons, status display, and L293D/motor all bench-tested individually then confirmed together in one non-blocking integration sketch (`Firmware/BSH-Elevator/BSH_Elevator_Node/BSH_Elevator_Node.ino`); full writeup in `Firmware/BSH-Elevator/SYSTEM_REPORT.md`. Along the way, substituted a 128x64 I2C SSD1306 OLED for the originally-documented 16x2 LCD (same SDA/SCL pins, different address/library) | Second system (after the Fire Alarm pilot) to reach Phase A, per the Sep 23 build-order change above. Bring-up surfaced several reusable lessons: a motor that doesn't spin while its control-line debug LEDs toggle correctly points to the driver chip's own power/wiring (common ground, chip orientation, voltage at the chip itself), not the code; a DC gearmotor's two leads aren't polarized and can go to either driver output; and a stray `*/` inside a header comment's example file path can silently truncate a C-style block comment and cascade into unrelated compiler errors far from the real mistake. OLED swap was the user's preference; register map and firmware call signature (`lcdPrint`) are unaffected |
| Sep 26, 2026 | **Founder's Hall slot 1 changed from Elevator to an HVAC zone unit**, and the change folded into every dependent doc in one pass: Sections 1, 3, 4, 5, 6, 8, 9, 11 and 13 of this document, `Firmware/WORKFLOW.md` (Phase 2 order, status tracker, Section 7), a new `FH_Systems/01_FH-HVAC.md` status, a new `Firmware/FH_HVAC_Bringup_Checklist.md`, and a new `FH_Sensors/17_DHT22_Temp_Humidity_Sensor.md`. The retired `01_FH-Elevator.md` and its bring-up checklist moved to `Archive/` | The HVAC design had been sitting in `FH_Systems/` as a second file numbered `01`, claiming the same IP and OpenPLC slot as FH-Elevator, with every other doc still assuming the Elevator — an unresolved fork masquerading as a decision. Resolved in favour of HVAC: it gives the project a genuine analog false-data-injection target (`TEMP_C`, Scenario 11) that no mirrored system provides, and the project's first master-set analog setpoint (`TARGET_TEMP_C`, a Holding Register). Accepted costs: FH and BSH are no longer 1:1 mirrors, slot 1 can no longer be cross-building compared, FH-HVAC cannot reuse a BSH Wokwi simulation, and `FH_Sensors/01` and `/03` (L293D, LCD) are now annotated as unused in FH |
| Sep 26, 2026 | **Propagated the Sep 22 per-container networking scheme into all 19 downstream docs** — 5 `BSH_Systems/`, 6 `FH_Systems/` and 8 bring-up checklists now say each OpenPLC is its own Docker container on its own IP (`.111`—`.125`) using the default ports 8080/502, instead of processes sharing the hub IP on custom ports (8081—8095 / 5021—5035). `New_Architecture_Overview.md` was also brought current (its port table, its "can't all bind 502" reasoning, its Wiegand and Air Quality rows) and given a banner saying this document is authoritative | The Sep 22 decision was recorded only in Section 6 here. Every per-system doc and every checklist — the files someone actually opens while configuring a PLC — still described the superseded scheme, so the first person to reach Phase C would have configured the wrong IP and port from an authoritative-looking doc |
| Sep 26, 2026 | **Established one home per kind of file (`Firmware/WORKFLOW.md` Section 8) and emptied the `Claude outputs/` folder.** Component folders may now hold an optional third file, `BenchTest.ino`; three real bench-test sketches moved from the download folder into their component folders; two stale copies of the elevator firmware and an unreferenced PNG moved to a new `Archive/` (with its own README explaining every entry); `Claude outputs/README.md` now states the folder is a download inbox that should stay empty | An audit found six files parked in an untracked download folder that no document referenced. One of them, `BSH_Elevator_Node-1.ino`, used the abandoned L293D pin mapping (ENABLE 13 / DIRA 14 / DIRB 26) rather than the bench-confirmed 18/19/5 — a file that looks current, sits beside a correct copy, and would have been flashed and wired wrong. The rule that follows: never two copies of a sketch, and a preserved variant goes to `Archive/` with a stated reason, never beside the live file under a `-1` name |
| Sep 26, 2026 | **Removed the em dash from all 7 non-ASCII `.ino` files** (`--` instead), including `BSH_FireAlarm_Node.ino`, both Wokwi sketches, all three bench tests and `FH_FireAlarm_Node.ino` | The Sep 25 Elevator bring-up had already established that non-ASCII characters in Arduino comments fail to compile on this project's Windows Arduino IDE setup ("extended character is not valid in an identifier") — but only the file that hit the error was fixed. Seven others still carried the same hazard, including the pilot node's firmware. A lesson learned that only gets applied to the file where it was found is half a lesson |
| Sep 26, 2026 | **Gave FH-FireAlarm a real system folder.** `Firmware/FH_FireAlarm_Standalone_Test/` became `Firmware/FH-FireAlarm/`, matching BSH-FireAlarm's structure: the sketch now sits at the canonical `FH_FireAlarm_Node/FH_FireAlarm_Node.ino` with an honest STATUS block (never run on FH hardware; `GAS_CALIBRATION_INTERVAL_MS = 0` flagged as a test compromise), five component folders carry FH-adapted `Wiring.md` copies plus empty `Build_Log.md` files, and `ESP32_Core_Modbus/` carries the `.22`-not-`.12` IP warning | The FH work was real and worth keeping — it was the filing that was wrong. A loose folder outside the system-folder convention made Phase 2 look started when it has not started, and left an FH sketch with no system doc, no checklist link and no component logs. Mirrored systems stay as separate folders and separate `.ino` files on purpose: the only difference between the pair is one hardcoded IP, and that has already been got wrong twice (FH-Door shipped with `.13`, FH-Elevator with `.11`) |
| Sep 26, 2026 | **Every reference now has a note file in `References/GitHub/`, libraries included** (10 new notes), and two libraries the project actually depends on were added to the index: `Adafruit SSD1306` + `Adafruit GFX` (what the working elevator firmware uses) and the planned `Adafruit DHT sensor library` for FH-HVAC. `LiquidCrystal_I2C`'s status changed from Adopted to superseded-on-the-bench/simulation-only | The old rule exempted libraries from needing a note, which left the dependencies used every day as the only undocumented references — and let the Sep 25 LCD→OLED substitution go unrecorded in the index entirely, so the index still credited a library the firmware no longer calls. Notes state plainly which details (licenses, versions, commit counts) are unverified rather than inventing them |
| Sep 26, 2026 | **Repaired cross-references broken by the two parallel copies of this documentation.** Five files pointed at `claude/CyberUIW_Workflow_v2.md`, which does not exist on local disk (the local master doc is `Piranga_Workflow_v2.md`), and 38 more references across 28 files used `claude/`-prefixed paths that resolve only inside the Claude project. All now resolve against the local folder; the one genuine Project-only reference (`claude/CyberUIW_Workflow.md`, the v1 workflow, which has no local copy) kept its prefix deliberately | **Open issue, not yet resolved:** the Claude project and this folder are two copies of the same documentation that have drifted. The project holds the Phase B/Modbus `.ino` files for Door, PowerBox, Perimeter and the FH mirrors, which exist nowhere on local disk; this folder holds `SYSTEM_REPORT.md`s, the Wokwi folders and the bench firmware, which are not in the project. Until one copy is named authoritative and the other reconciled to it, this class of broken reference will keep coming back |
| Sep 28, 2026 | Retired DALI for BSH/FH-PowerBox lighting in favor of a plain on/off relay — the second channel of the same 2-channel relay module already driving the fan | DALI was the project's only unverified field-bus library (several incompatible forks, method names never confirmed against an installed version) and needed an untested transceiver add-on plus DALI-addressable fixture. Register map lost `LIGHTS_DIM_LEVEL` (HR0); Power Box is now the same relay-only pattern as Fire Alarm/Elevator/Backup Power, no longer flagged higher-risk than Fire Alarm. Door's RFID reader is now the project's only dedicated field-bus protocol (Section 5) |
| Sep 28, 2026 | Moved `Firmware/BSH-PowerBox/DALI_LED_Driver/` to `Firmware/BSH-PowerBox/Lights_Relay/` (rewritten `Wiring.md`/`Build_Log.md` for the relay), retired `BSH_Sensors/12_DALI_LED_Driver.md`/`FH_Sensors/12_DALI_LED_Driver.md` (deleted from the Project, local copies overwritten with a deprecation stub), and changed `References/GitHub/PowerBox_silahcen001_DALI-ESP32.md`'s status from Adopted to Rejected | Same retirement pattern used for the Air Quality → Perimeter/Motion and MQ135 → MQ-2 swaps: keep a stub/record rather than deleting outright, so nobody re-evaluates the dropped choice or hits a dangling cross-reference |
| Sep 28, 2026 | Moved BSH/FH-PowerBox's lights relay off GPIO 12 onto GPIO 13, after the Phase A0 Wokwi simulation showed the lights channel behaving inverted/unreliable while the fan channel (GPIO 15) worked correctly with identical firmware logic | GPIO 12 is an ESP32 strapping pin (MTDI), sampled at reset to select flash voltage. It was carried over unchanged from the old DALI TX pin assignment when DALI was dropped for a relay (see the two entries above), without re-checking whether a relay's IN pull-up made it unsafe. A relay module's onboard pull-up drags GPIO 12 HIGH during the reset sampling window, before `setup()` ever runs, causing the mis-detected flash voltage to manifest as inverted output on that pin. GPIO 13 has no boot-strapping role. Documented generally in `00_START_HERE.md` Section 7 (strapping pins 0, 2, 5, 12, 15) so the same mistake isn't repeated on a future system |

| Sep 29, 2026 | BSH/FH-PowerBox's Wokwi Phase A0 sim needed `RELAY_ACTIVE_LOW` split per channel and set to active-HIGH (`false`) for both lights and fan, after moving lights to GPIO 13 didn't fix the inverted-behavior symptom | GPIO 13 isn't a strapping pin, so the Sep 28 GPIO 12->13 move was ruled out as the cause once the same inversion persisted there -- confirmed by checking the relay module's own onboard energization indicator, not just the external test LED, which also stayed inverted. Testing the fan channel just as carefully (not just glancing at it) then showed it was **also** inverted the whole time, just not noticed until lights was fixed and fan was compared against it again. Conclusion: Wokwi's `wokwi-relay-module` energizes on HIGH, not LOW, on both channels -- a simulator-only quirk, not evidence about the real relay hardware. The real firmware (`BSH_PowerBox_Node.ino`/`FH_PowerBox_Node.ino`) keeps `RELAY_ACTIVE_LOW = true` and still needs its own fresh bench verification. See `Firmware/BSH-PowerBox/Wokwi/README.md`'s "Relay polarity" section |
| Sep 29, 2026 | Added a local physical override push button per channel to BSH/FH-PowerBox (GPIO 4 lights, GPIO 16 fan, mirroring the pin choices already used by BSH-Elevator's floor buttons on separate hardware), plus two new Discrete Inputs, `LIGHTS_STATUS`/`FAN_STATUS`, reporting each relay's actual physical state | Modeled on the same local-vs-remote authority idea the Fire Alarm pull station already demonstrates against a hub-issued `ALARM_ACTIVATE`, applied here to an actuator instead of an alarm -- a real building almost always has a local switch alongside networked control, and the mismatch between the two is itself a realistic ICS teaching point. Authority rule: whichever source (hub or button) acted most recently wins; the hub only regains authority on a *fresh* coil write (the coil's value actually changing), not merely by holding its last-written value, otherwise a local press would be overwritten within one `ACTION_INTERVAL_MS` cycle by the hub's stale, unchanged command. This also closes `BSH_Sensors/09_Relay_Module.md`'s previously-flagged open-loop gap ("no feedback Discrete Inputs are wired yet") as a side effect, since honest status feedback was needed for the button anyway |

| Sep 29, 2026 | Replaced the `ButtonState` struct in BSH/FH-PowerBox's button-debounce code (Wokwi sim and real firmware) with plain per-button `int`/`unsigned long` variables passed by reference | Wokwi's build failed with `'ButtonState' was not declared in this scope` on `readButtonEdge(ButtonState &btn)`, even though the struct was defined earlier in the same file. Cause: the Arduino/Wokwi build auto-generates function prototypes and inserts them near the top of the file, ahead of any struct the sketch defines -- a known limitation when a function takes a custom struct/class by reference. Caught in Wokwi Phase A0 testing before it could bite on real hardware; worth remembering for any future system that reaches for a shared struct across multiple debounced inputs |

| Sep 30, 2026 | Bench-tested BSH-PowerBox's fan relay channel alone (Phase A, Step 1; DC motor + fan blade as the load) using the original 2-channel relay module. Found it active-HIGH (`RELAY_ACTIVE_LOW = false`), the opposite of the "typically active-LOW" assumption the docs carried over from general relay-module lore | Confirms two channels on one physical module aren't guaranteed to share a polarity with each other, let alone with the simulator. Logged in `Firmware/BSH-PowerBox/Fan_Relay/Build_Log.md` and `Wiring.md`. **Superseded the same day** by the next entry, which replaces this module entirely -- this finding no longer applies to the fan relay currently in use, kept here and in the component Build_Log purely as a record of what was tried |
| Sep 30, 2026 | Switched BSH/FH-PowerBox from a single 2-channel relay module (lights + fan sharing one board) to two separate 1-channel relay modules, one per channel | No functional difference to the ESP32 -- a GPIO pin drives a relay regardless of which board it's soldered to -- but two consequences follow: each board now needs its own VCC/GND wired to the ESP32 instead of sharing one pair, and the two relays are no longer on the same board, so `RELAY_ACTIVE_LOW` was split into independent `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` flags in `BSH_PowerBox_Node.ino`/`FH_PowerBox_Node.ino` rather than one shared flag. The fan channel's Sep 30 active-HIGH finding (previous entry) was on the now-retired 2-channel module and does not carry over to the new 1-channel fan relay -- it needs its own fresh bench verification, same as the lights channel (not yet tested at all). Section 8's Hardware Checklist and both PowerBox bring-up checklists updated to list 2x 1-channel relay modules instead of 1x 2-channel module |

| Oct 5, 2026 | BSH-PowerBox's Phase A (standalone bench test) completed and confirmed. Using a standalone, no-WiFi/no-Modbus combined integration sketch (both relays + both local override buttons together, printing state to Serial), both of BSH-PowerBox's new 1-channel relay boards were bench-verified: `RELAY_LIGHTS_ACTIVE_LOW = false` and `RELAY_FAN_ACTIVE_LOW = false` (both active-HIGH). Both physical override buttons correctly toggle only their own channel, and both relays' physical state matches the Serial-printed state on every transition for both the buttons and the `l`/`f` Serial commands | This supersedes the "not yet tested at all" status left by the Sep 30, 2026 hardware-swap entry above. Both channels happened to land on the same active-HIGH direction as the old, now-retired 2-channel module's fan channel and as the Wokwi simulator's quirk -- noted as a coincidence worth tracking, not as evidence that one predicts the other, since each was (and should keep being) verified independently. Along the way, an earlier run of this bench test used the wrong, older (lights-only) sketch, which produced a misleading symptom -- the fan relay and both buttons looked completely dead, with zero Serial output for any of them -- initially suspected as a GPIO-strapping-pin or ESP32-board-variant hardware issue before the actual cause (wrong code flashed) was found. Worth keeping as a standing troubleshooting reminder: total silence from a whole code path points first at "is this the sketch I think it is," not at wiring. FH-PowerBox has NOT had its own Phase A bench test -- its two relay boards are different physical units, and `FH_PowerBox_Node.ino` deliberately keeps both flags at their untested `true` default until it gets one |
| Oct 5, 2026 | Dropped the Backup Power / ATS Monitor system (BSH-BackupPower and FH-BackupPower, slot 5) and did **not** replace it. Each building now has four systems and the project has **8 ESP32 nodes and 8 OpenPLC instances** instead of 10. IPs 192.168.50.15 / .25 and OpenPLC containers .115 / .125 are now unused. Its Phase A0 Wokwi simulation had been built and verified on Oct 5, 2026 before the decision. All its design docs, sensor sheets, bring-up checklists, and the `Firmware/BSH-BackupPower/` folder (including the Wokwi simulation and its four component Build_Logs) were moved to `Archive/BackupPower/` | Decision made by Luis together with the course professor. No technical problem with the system was recorded; the scope was reduced to four systems per building. Scenario 6 (Backup Power ATS Manipulation) is dropped with it. Files were archived rather than deleted so the work stays recoverable |
| Oct 6, 2026 | Every node gets two extra Input Registers for connection monitoring: `HEARTBEAT` (counts up once a second) and `RECONNECT_COUNT` (WiFi rejoins since boot). Piloted on BSH-FireAlarm as IR1/IR2 (it already uses IR0). Elevator and PowerBox follow after Fire Alarm passes its bench test; each node's heartbeat registers sit directly after its last existing Input Register | SCADA polls OpenPLC, not the ESP32. If an ESP32 drops off WiFi, its OpenPLC container keeps answering Node-RED with stale or zero values, so a plain Modbus read cannot show the loss. A counter incremented by the ESP32 itself can: frozen heartbeat = lost (yellow after about 3s, red after about 5s), heartbeat resumes with `RECONNECT_COUNT` +1 = WiFi drop, both back at 0 = reboot. Consequence for Phase C: when adding each ESP32 as a Generic Modbus TCP slave in its OpenPLC instance, the Input Register count must include these two registers (Fire Alarm: 3, not 1), or OpenPLC will not poll them |
| Oct 6, 2026 | Whole project address plan moved from 192.168.1.x to **192.168.50.x**, same last numbers: hub .10, nodes .11-.24, OpenPLC containers .111-.124, gateway 192.168.50.1. Network is the lab router SCT5 (2.4 GHz band used by the ESP32s) | BSH-FireAlarm's first Phase B flash joined SCT5 but could not be reached: the laptop was on 192.168.50.142 while the sketch used a static 192.168.1.12. The SCT5 router is not under the team's control, so the plan changed instead of the router. Open risk: the router's DHCP pool overlaps these addresses and cannot be reserved from our side, so ping an address before using it for the first time (a reply means a collision). macvlan for the OpenPLC containers still needs the hub on wired Ethernet to this router |
| Oct 6, 2026 | Phase B started: BSH-FireAlarm (.12), FH-FireAlarm (.22) and BSH-PowerBox (.14) bench-confirmed; FH-PowerBox (.24) and BSH-Elevator (.11) written, not yet flashed. FH-FireAlarm and FH-PowerBox were built ahead of the BSH-first order | Luis built the FH hardware early and asked to proceed; the single-focus rule was waived for those nodes. FH's standalone Phase A was never logged separately, so the Phase B run doubles as FH's hardware check, and FH relay polarity must be re-verified rather than copied from BSH |
| Oct 7, 2026 | FH-PowerBox (.24) Phase B bench-confirmed: coil writes, status DIs, local buttons, hub-regains-authority on a fresh coil write, 5 s WiFi watchdog, HEARTBEAT and RECONNECT_COUNT all passed from PowerShell. Both outputs were OFF at boot with `RELAY_LIGHTS_ACTIVE_LOW` and `RELAY_FAN_ACTIVE_LOW` = `false` | FH's relay boards are active-HIGH like BSH's, verified on FH hardware rather than copied. BSH and FH PowerBox online together later confirmed by Luis (Oct 9). Phase C (OpenPLC rewrites coils continuously and may undo button overrides) has not started |
| Oct 7, 2026 | FH-HVAC (.21) Phase B bench-confirmed from PowerShell: live TEMP_C/HUMIDITY_PCT, HR0 setpoint switches the relay, Coil0 clear forces it off, sensor unplug latches DI1 with TEMP_C held at last good, WiFi cut fails off after ~10 s and recovers with RECONNECT_COUNT 1, reset zeroes both counters | First node using a Holding Register as a master-set setpoint and the first with fail-off on a link loss. Still open: comms loss is WiFi-link only (a master that stops polling with WiFi up is not detected); Phase C must set OpenPLC counts DI 2, coil 1, IR 4, HR 1 |
| Oct 7, 2026 | BSH-Elevator (.11) Phase B bench-confirmed from PowerShell: remote CALL_FLOOR coils move the car and self-clear, ELEVATOR_MOVING true only in transit, CURRENT_FLOOR updates, ELEVATOR_DISABLE blocks new calls, local buttons work, HEARTBEAT/RECONNECT_COUNT pass | ELEVATOR_FAULT (DI1) cannot latch because the normal arrival timeout always fires before the fault timer (dead safety net, left as is). DISABLE does not stop a move in progress and there is no fail-off on WiFi loss -- worth stating in any Phase C scenario. OpenPLC counts: DI 2, coil 4, IR 3 |
| Oct 9, 2026 | Door Phase A firmware received for both buildings and placed at `Firmware/BSH-Door/BSH_Door_Node/BSH_Door_Node.ino` and `Firmware/FH-Door/FH_Door_Node/FH_Door_Node.ino` (no `_PhaseA` suffix, per `Firmware/WORKFLOW.md` Section 8). Written by Luis's partner after a review against the pin map | The first drafts were adapted from a kit tutorial and differed from the plan: an I2C RFID module (BSH), SPI SCK/MISO moved to 16/17 (FH), IR on GPIO 27, a single red LED on 19 plus a buzzer on 18, and a card check that compared hex text and dropped leading zeros. The final sketches follow the pin map (RC522 on 18/19/23/32/33, IR on 14, RGB LED on 25/26/27, no buzzer) and use a byte-array card list. Still open: the IR and reed polarity flags are unverified on hardware, and so is whether the 3-pin reed module has its own pull-up (GPIO 34 has none internally) |
| Oct 9, 2026 | BSH-Door (.13) Phase B bench-confirmed from PowerShell: DOOR_OPEN_CMD opens the gate with no card and no ACCESS_GRANTED, DOOR_CLOSE_CMD closes it, card badge pulses ACCESS_GRANTED (DI2), HEARTBEAT/RECONNECT_COUNT and the 5 s WiFi watchdog pass | The remote-open-without-badge result is the vulnerability this node is built to demonstrate (SCADA must correlate DOOR_POSITION with ACCESS_GRANTED). IR_DETECTED and DOOR_POSITION were confirmed by Luis on the bench (the pasted capture showed both held at 1, so he verified the transitions himself). OpenPLC counts: DI 3, coil 2, IR 2 |
| Oct 9, 2026 | FH-Door (.23) Phase B bench-confirmed, all tests confirmed by Luis: same results as BSH-Door (remote open with no card and no ACCESS_GRANTED, remote close, card pulse, IR/reed, heartbeat, WiFi watchdog), and the FH card (3D D2 C6 62) was confirmed as FH's | All eight nodes are now through Phase B, and the two PowerBoxes were confirmed together. Phase C not started. OpenPLC counts: DI 3, coil 2, IR 2 |
| Oct 9, 2026 | Cleared the `Claude outputs/` download inbox: moved `phaseA_03_buttons_only_test.ino`, `phaseA_04_combined_test.ino` and `BSH-PowerBox_Wiring_Reference.html` to `Archive/PowerBox_PhaseA_inbox/`, and recorded both that folder and `Archive/Door_PhaseA_sketches/` in `Archive/README.md` | Per the inbox README's own rule, files left there move to their real home or to `Archive/` with a row explaining why. The PowerBox Phase A sketches are superseded by the bench-confirmed Phase B sketches; the Door Phase A sketches were replaced in place by Phase B. Nothing was deleted |
| Oct 9, 2026 | Regenerated `Piranga_Revised_Architecture.docx` from the current design: eight nodes and eight OpenPLC containers (was ten, with the retired Air Quality nodes), Door field layer now SPI RC522 RFID (was Wiegand), Power Box now relay only (was DALI), container IPs 192.168.50.111-.124 on default ports 8080/502, scenario table matching Section 11 (Scenario 6 dropped, 11 added), a new architecture diagram, and a Phase B status and Phase C risk list. Also corrected three leftover "10" counts in this document (Sections 2, 7 and Scenario 9) to 8 | The .docx was a Sep 16 snapshot of the first revision and had drifted from this document, which is authoritative. A backup of the previous .docx sits outside the project folder |
| Oct 9, 2026 | Hosting moved to a **Proxmox VE server** (`pve`, 192.168.50.143, operational today, installed on a wiped former-Windows PC, wired by Ethernet to the lab router SCT5). Proxmox hosts the OpenPLC Docker VM (192.168.50.10, 8 containers) and a separate SCADA/HMI VM (Node-RED), plus any further VMs the build needs. PLCs stay containers (Sep 22 decision unchanged); the Raspberry Pi/laptop hub plan is retired | Luis wants VMs for the Docker layer and SCADA/HMI; SCADA in its own VM also sidesteps the macvlan host-to-container limitation |
| Oct 11, 2026 | New top-level folder `PLC_Logic/`: one subfolder per system with `README.md`, `Register_Map.md`, `Ladder_Diagram.md`, `Program.st` and `Build_Log.md`, plus `Templates/` and `Setup/` (Docker/macvlan templates). All 8 systems drafted, nothing compiled or run; Phase C is not declared started. Programs publish node data into the PLC's own `%QX0.x` / `%QW0..` addresses, which the OpenPLC docs define, because the docs do not state where slave-device data (`%IX100.0` etc.) appears | Luis wanted the OpenPLC logic to have its own home with diagrams and explanations, beside `Firmware/`, and to write some of the programs himself. Open items: slave-block address convention, Modbus slave ID, OpenPLC Docker image, FH-HVAC comms-loss direction |
| Oct 12, 2026 | Four decisions. (1) **SCADA/HMI = Ignition (Maker Edition)**, replacing Node-RED: the professor approved it and submitted the license application; the company's response is pending. Fallbacks: Rapid SCADA, then FUXA; start a fallback on Oct 14 if no license. (2) **pfSense = stretch goal** after the core works (about Nov 16-29), with five prep rules so it can be added later. (3) **VPN = Tailscale first** (subnet router on Proxmox), because SCT5 cannot forward ports; a pfSense VPN is optional later. (4) **Accelerated target**: core system working by Oct 31, final demo still Dec 11 | Luis's partner suggested pfSense and Ignition/ScadaBR for industry realism; Luis asked the professor, who approved Ignition. Options and the schedule are in Section 10 ("Platform decisions and options"). Next-session handoff: `Next_Steps.md` |
| Oct 13, 2026 | **Phase C started.** Built the OpenPLC Docker VM (VM 100, 192.168.50.10) and, the same evening, the `openplc:v3` image (local build from github.com/thiagoralves/OpenPLC_v3), the macvlan network `cybercity_lan` (parent `ens18`) and the first container, `OpenPLC-BSH-FireAlarm` at 192.168.50.112, reachable from the laptop on port 8080 (Blank Program, stopped). Each container keeps its programs and settings in its own named Docker volume on `/docker_persistent` and runs without `--privileged`. The default OpenPLC login is kept on every container (decision: not changed) | The OpenPLC_v3 Dockerfile keeps its database, uploaded programs and settings under `/docker_persistent` (declared as a volume), so a named volume per container survives a container rebuild. The repo README uses `--privileged` only in a generic example; the container started and served the web UI without it. Ubuntu 26.04's `sudo` ignores `-E`, so scripts take variables as `sudo env VAR=value ...`. Still open: slave-device address convention, Modbus slave ID, FH-HVAC comms-loss direction. Infrastructure detail is in `Proxmox_Build_Log.md` |
| Oct 14, 2026 | **All 8 OpenPLC containers running.** The 7 remaining containers (.111, .113, .114, .121-.124) were started on the OpenPLC Docker VM with `run_containers.sh`; all 8 web UIs (port 8080) answer HTTP 200 from the laptop. Each has its own named volume on `/docker_persistent`, no `--privileged`, default OpenPLC login, and a blank program. Order for the rest of the day: containers first, then the SCADA/HMI VM | The script was fixed to skip existing containers and use a volume per container before running. The VM keeps its own copy of `Setup/`, so re-copy it after any change. Still open: ESP32 slave device and `Program.st` in each container, slave-device address convention, Modbus slave ID, FH-HVAC comms-loss direction. Detail in `Proxmox_Build_Log.md` |
| Oct 14, 2026 | **Ignition license checkpoint moved from Oct 14 to Oct 21.** The rule is otherwise unchanged: if the license has not arrived by Oct 21, start on a fallback (Rapid SCADA, then FUXA) and switch to Ignition once it is licensed. The professor's application is with Inductive Automation (Educational Engagement Program); a follow-up goes out if there is no reply | Luis extended the wait by a week. Effect: the SCADA tag and HMI work (Oct 20-23 in the schedule) starts on the license, or on a fallback, only from Oct 21; the OpenPLC and infrastructure work continues meanwhile |
| Oct 14, 2026 | **SCADA/HMI VM built: VM 101 `scada-hmi`.** Three decisions. (1) **OS = Ubuntu 24.04.5.1 LTS desktop**, not Windows Server 2022, not the Win10/Win11 ISOs, and not the 26.04.1 ISO already on `Big_Storage`. (2) **Specs:** 2 vCPU type `host`, 8 GB RAM, 25 GB disk on `local-lvm`, VirtIO NIC on `vmbr0` with the firewall off, QEMU agent on, hostname `SCADA-HMI`, user `scada-hmi`. (3) **Static IP 192.168.50.30** (set Oct 15; the VM was on DHCP 192.168.50.237 until then), outside the .10-.24 and .111-.124 ranges. Also fixed `pve`'s DNS (it pointed at `192.168.100.1`; now `192.168.50.1` + `8.8.8.8`) so the ISO could be downloaded. Ignition itself is not installed yet (license pending, checkpoint Oct 21) | Ubuntu LTS is the common Linux platform for Ignition and matches the Linux VM 100; Ignition's docs name Ubuntu but no release, and 24.04 is the release most Ignition users run, so it removes one unknown. Not a published support statement for the standalone installer; the Ignition 8.3.0 release notes do say Cloud Edition and the Docker image are based on Ubuntu 24.04 LTS (re-check before installing). Windows Server would need a license, 50-60 GB (does not fit the 28 GB left on `local-lvm`) and VirtIO driver setup, and nothing in the design needs OPC-DA/DCOM; Win10 22H2 is out of support and Win11 needs about 64 GB and a TPM. Desktop edition keeps the plan to show the HMI on-screen. Real plants still mostly run Windows Server for SCADA, so a Windows VM may be added later as a victim/HMI-client machine for Section 11, on `Big_Storage`. 24.04 differs from VM 100's 26.04 on purpose. Full record: `Proxmox_Build_Log.md`. |

> **Instructions carried over from v1:** Update this section throughout the project — not just at the end. Document any issues encountered and how they were resolved.

---

*Document maintained by the Automated Cyber-Physical Campus System Design project team*
