# Hardware BOM — the eight ESP32 nodes (DRAFT)

Started Nov 4, 2026 (title says "eight ESP32 nodes"; Part two, section 4, covers servers, network and software). **Compiled from the docs, not counted on the bench.** Nothing here is "bench-confirmed" unless the Status column says so. Blank cells are blanks on purpose: fill them from what you actually own and bought, do not guess.

Sources: the eight `Firmware/*Bringup*.md` checklists (Parts Needed), `BSH_Sensors/` and `FH_Sensors/`, `Firmware/BSH-FireAlarm/*/Build_Log.md`, `STATUS.md`.

Status key: **Doc** = a checklist or sensor sheet lists it · **Bench** = a log says the real part was used · **Open** = docs disagree or the part is not recorded.

## 1. Per-node parts

### BSH-FireAlarm (.12) and FH-FireAlarm (.22) — same list, quantities are per node

| Qty | Part | Status | Notes |
|---|---|---|---|
| 1 | ESP32 DevKit | Doc | "any DevKitC/DOIT-style"; exact model not recorded |
| 1 | Active buzzer | Doc | GPIO 23, direct drive |
| 1 | Common-cathode RGB LED | Doc | GPIO 25/26/27 |
| 3 | 220 ohm resistor | Doc | one per RGB channel |
| 1 | Push button (pull station) | Doc | GPIO 4, internal pull-up |
| 1 | Flame sensor module (3-pin, LM393) | Doc | GPIO 18, active LOW. **The pilot checklist's parts table omits it**; it is in the sensor sheet and the FH checklist |
| 1 | Gas/smoke sensor module | **Open** | **BSH bench part is an MQ-9, not the documented MQ-2** (`MQ-2_Gas_Smoke_Sensor/Build_Log.md`, Sep 17). FH module model: not recorded. AO on GPIO 34; DO wired to GPIO 19 on BSH, unused |
| 0-1 | 1k + 2k resistor (AO divider) | **Open** | needed only if AO exceeds 3.3 V; the AO voltage check was **still pending** in that log (Sep 17), no later record found |
| 1 | Breadboard + jumpers, USB data cable | Doc | |

### BSH-Elevator (.11)

| Qty | Part | Status | Notes |
|---|---|---|---|
| 1 | ESP32 DevKit | Doc | |
| 1 | L293D motor driver | Doc | |
| 1 | Small DC motor | Doc | motor lead came off Oct 20, strain relief suggested |
| 3 | Push buttons (floors 1/2/3) | Doc | |
| 1 | 16x2 I2C LCD | Doc | |
| 1 | External motor supply | Doc | model and voltage not recorded |
| 1 | Breadboard + jumpers, USB data cable | Doc | |

The old 7-segment display is not in this node's BOM.

### BSH-Door (.13) and FH-Door (.23) — same list, per node

| Qty | Part | Status | Notes |
|---|---|---|---|
| 1 | ESP32 DevKit | Doc | |
| 1 | RC522 RFID reader (SPI) | Doc | 3.3 V only |
| 1+ | RFID card or fob | Doc | the UID goes in `AUTHORIZED_CARDS[]` |
| 1 | SG90 micro servo | Doc | external 5 V supply per sheet |
| 1 | External 5 V supply for the servo | Doc | what was actually used: not recorded |
| 1 | Common-cathode RGB LED + 3x 220 ohm | Doc | blue wired "for BOM consistency", never lit |
| 1 | IR sensor module | Doc | |
| 1 | Reed switch | Doc | `DoorPosition` reads wrong (open issue, STATUS.md) |
| 1 | ~10 kohm resistor (reed pull-up) | Doc | GPIO 34 has no internal pull-up |
| 1 | Breadboard + jumpers, USB data cable | Doc | |

### BSH-PowerBox (.14) and FH-PowerBox (.24) — redesigned Oct 28 as power rails, per node

| Qty | Part | Status | Notes |
|---|---|---|---|
| 1 | ESP32 DevKit | Doc | BSH: the former spare board; FH: spare or original board **not decided** per the FH checklist |
| 3 | 1-channel 5 V relay module | Doc | BSH reported as Songle SRD-05VDC-SL-C type (Luis); polarity per board **not confirmed** |
| 1 | Breadboard + jumpers, USB data cable | Doc | |

Dropped Oct 28: lights, fan, push buttons. Rail wiring and how each downstream node is fed: `Firmware/<node>/Schematic/Connections.md` (the FH one is still the old design, per STATUS.md, so do not take its parts).

### FH-HVAC (.21)

| Qty | Part | Status | Notes |
|---|---|---|---|
| 1 | ESP32 DevKit | Doc | |
| 1 | Temperature/humidity sensor | **Open** | checklist and sheet say DHT22; **as built: DHT11 3-pin module** (decided Oct 7). Yet readings have decimals (189 = 18.9 C), so the real part is unexplained (STATUS.md) |
| 1 | 1-channel relay module (keyestudio, active-HIGH) | Bench | polarity recorded Oct 7 |
| 1 | Small DC fan | Doc | runs from the board 5 V pin through the relay |
| 1 | 1N4007 diode | Doc | flyback across the fan |
| 1 | Common-cathode RGB LED + 3x 220 ohm | Doc | |
| 0 | 10 kohm DHT pull-up | Doc | in the pin table, but the 3-pin module already has one |
| 1 | Breadboard + jumpers, USB data cable | Doc | |

## 2. Rolled-up totals (arithmetic on the tables above, not counted)

| Part | Total | How |
|---|---|---|
| ESP32 DevKit | 8 | |
| Relay modules | 7 | 3 + 3 PowerBox, 1 HVAC |
| RGB LED | 5 | 2 FireAlarm, 2 Door, 1 HVAC |
| 220 ohm resistors | 15 | 3 per RGB LED |
| Push buttons | 5 | 1 per FireAlarm, 3 Elevator |
| Active buzzers | 2 | |
| Flame sensor modules | 2 | |
| Gas sensor modules | 2 | MQ-9 on BSH |
| RC522 readers / SG90 servos / IR modules / reed switches | 2 each | |
| 10 kohm resistors | 2 | Door pull-ups |
| RFID cards | 2+ | |
| L293D, DC motor, 16x2 I2C LCD | 1 each | Elevator |
| Temp/humidity sensor, DC fan, 1N4007 | 1 each | HVAC |
| Breadboards, USB data cables | 8 each | |
| Power supplies (motor, servo x2) | 3 | models not recorded |
| 1k + 2k divider pairs | 0-2 | only if the AO check demands it |

Not in any bring-up checklist: jumper wire counts, a shared bench supply, spares. The 3D-printed structures are listed in `Piranga_Workflow_v2.md` Section 8 (see 4d below). The Wi-Fi AP is in 4a.

## 3. To settle before this is final (Luis)

1. Gas module on FH: MQ-2, MQ-9 or other? And did the BSH AO voltage check (under 3.3 V) ever happen?
2. HVAC sensor: read the printed model off the part (DHT11 / DHT22 / other).
3. Which ESP32 boards you actually own: model, 30 vs 38 pin, spares.
4. Supplies: what feeds the elevator motor and the two servos.
5. Unit costs and sources: nothing in the folder records prices. Add them in a Cost column if you want a budget total.
6. Whether the retired parts in the sensor folders (DALI driver, HC-SR04, MQ-135, Wiegand) should be deleted, left alone, or listed as "not in BOM". They are not counted above.

## 4. Part two: servers, network, software

Sources: `Proxmox_Build_Log.md` (Oct 13-22), `Piranga_Workflow_v2.md` Section 8 (Hardware Checklist) and Section 6, `STATUS.md`. Same status key as above. **Not read on the machine itself**: the host's CPU and model are not in any doc.

### 4a. Physical hardware

| Qty | Item | Status | Notes |
|---|---|---|---|
| 1 | Proxmox host `pve` (192.168.50.143) | Doc | a dedicated former-Windows PC, wiped; operational Oct 9. **Make, model and CPU: not recorded.** 30 GB RAM |
| 1 | 120 GB SSD (`sdb`) | Doc | holds Proxmox: `local` 37.5 GB, `local-lvm` 68.5 GB (VM disks), swap 2 GB |
| 1 | 500 GB HDD (`sda`, storage `Big_Storage`) | Doc | ISOs, Kali disk, backups |
| 1 | Ethernet cable, `pve` to lab router SCT5 | Doc | wired link is required for macvlan; done Oct 9 |
| 1 | Lab router SCT5 | **Open** | subnet 192.168.50.0/24, gateway .1. Listed as "the lab router": whether it is yours or the lab's shared router is not recorded (a Kali note says "the lab router is shared") |
| 1 | Dedicated Wi-Fi access point / router | **Open** | the ESP32s need it; unticked in Section 8 yet checklists say "already broadcasting". Which device it is: not recorded |
| 1 | Laptop | Doc | Designer, browser HMI, SSH, Arduino IDE; model not recorded |
| 1 | 58 GB NTFS flash drive | Doc | spare copy of the 4 ISOs (Oct 13) |
| 0-1 | USB Ethernet adapter (second NIC) | Doc | optional, only if pfSense is built (stretch) |

### 4b. Virtual machines on `pve` (resources, not purchases)

| VM | Name | IP | vCPU | RAM | Disk | Status |
|---|---|---|---|---|---|---|
| 100 | `openplc-docker` (Ubuntu 26.04 desktop, 8 OpenPLC containers) | .10 | 2 | 6 GB | 40 GB `local-lvm` | Doc |
| 101 | `scada-hmi` (Ubuntu 24.04 desktop, Ignition 8.3.10) | .30 | 2 | 8 GB | 25 GB `local-lvm` | Doc |
| 102 | `Kali` (Kali 2026.2) | .40 | 2 | 4 GB | 40 GB `Big_Storage` | Doc |

RAM allocated: 18 GB of 30 GB. `local-lvm` is over-provisioned (about 115 GiB virtual on 68.5 GiB, Proxmox warned Oct 22; it filled to 100% on Oct 26). VM 100 root filesystem filled Oct 28 and is not yet protected by a log size limit (both in STATUS.md). Plan any new disk for `Big_Storage`.

### 4c. Software and licences

| Item | Version | Status | Notes |
|---|---|---|---|
| Proxmox VE | 9.2.2 | Doc | |
| Ignition (Standard) + Capstone license | 8.3.10 | Doc | **expires 2027-01-31**, rolling lease renews about every 72 h so VM 101 needs internet; the key has one activation (stays on this Gateway). The token is a secret and is not in the repo |
| OpenPLC Runtime v3 (Docker image `openplc:v3`) and OpenPLC Editor | v3 / v4 app | Doc | free; v3 kept through the demo |
| Kali Linux | 2026.2 | Doc | |
| Ubuntu desktop ISOs | 26.04.1, 24.04.5.1 | Doc | |
| Win10 / Win11 ISOs | - | Doc | on `Big_Storage`, not used |
| Arduino IDE + ESP32 core, ModbusIP_ESP8266, MFRC522 (miguelbalboa/rfid), Adafruit DHT libraries | - | Doc | exact versions not recorded |
| Tailscale (subnet router in a small VM/LXC) | - | Doc | chosen first for remote access; not built |
| pfSense | - | Doc | stretch, not built |

### 4d. From `Piranga_Workflow_v2.md` Section 8 that Part one did not carry

- **3D-printed structures (PLA):** BSH shell, Founder's Hall shell, campus base platform, crosswalk / street separator, smart traffic-light structure, wire-routing channels in the CAD. All unticked in Section 8. **PLA used: 29 kg total, of which about 4 kg went on failed prints, so about 25 kg ended up in the finished structures** (Luis-reported Nov 4, approximate; not weighed from spools by the agent). Printer: not recorded. Cost at an assumed $20-25/kg (agent estimate, not a purchase record): about $580-725 for the 29 kg, about $100 of it failures. Replace with the real spool prices.
- Section 8 itself is **stale** for several nodes (PowerBox still the old lights+fan design, Elevator display, HVAC fan "own supply"). This BOM follows the newer bring-up checklists and `STATUS.md`; Section 8 is frozen history in effect, do not edit it, supersede in `Logs/`.

## 5. Open questions added by part two

1. `pve`: make/model, CPU, and whether it has a free drive bay or port.
2. Is SCT5 your own router or a shared lab one, and what is the Wi-Fi AP the ESP32s use?
3. Laptop model, and which USB data cables you own.
4. Elevator display: the bring-up checklist says 16x2 I2C LCD, Section 8 of the workflow says an I2C **OLED** was substituted (Sep 25 note). Which is on the bench?
