# START HERE — what this project actually is, as of Oct 5, 2026

> **Read `Next_Steps.md` right after this file.** It is the current handoff: what was decided, what is waiting, and what to do next (updated Oct 12, 2026). The SCADA/HMI software is Ignition.

## 1. The main folder is the single source of truth

Everything authoritative lives in this folder:

```
C:\Users\lfoli\OneDrive\Área de Trabalho\CyberCity
```

**The Claude project ("Cyber CIty") is a read-only mirror of the design
documents in this folder.** When the two disagree, this folder wins. Never copy
anything from the Claude project back into this folder, and never treat a
document in the project as current without checking it against this folder
first.

Two files here are authoritative over everything else:

- `Piranga_Workflow_v2.md` — architecture, networking, protocols, scenarios,
  deliverables, and the decisions log. (In the Claude project it is mirrored at
  `claude/Piranga_Workflow_v2.md`. An older copy under the name
  the old v2 workflow copy was deleted on Sep 26, 2026 — it caused months of
  broken cross-references because the folder and the project used two different
  names for the same document.)
- `Firmware/WORKFLOW.md` — build order, single-focus rule, phase definitions,
  status tracker, and Section 8, which says where every kind of file belongs.

## 2. This particular file is the one exception

This file did not originate here — it was first written directly inside the
Claude project as an orientation note, then copied into this folder on Sep 28,
2026 so it's available without opening Claude. From now on, treat **this copy,
in the folder, as the current one** if the two ever drift, the same as every
other document. Update it here first, and let the Claude project's copy be
refreshed from it, not the other way around.

## 3. The project description and custom instructions

Those two fields live only in claude.ai and can only be edited by hand there —
neither this file nor any other document can change them. The custom
instructions field already points to this file and names the two authoritative
docs above, so it's current. The description field is out of date in two ways:
it says 10 ESP32 nodes (the project has eight) and it names the old SCADA
software (the SCADA/HMI is Ignition), and it may still carry the leftover
"NSF LSAMP" label, which isn't important to what this project actually is.
Fix these by editing the field directly in claude.ai project settings.

## 4. What the project is now

A 3D-printed smart-campus cyber-physical security testbed, running **Aug 24 –
Dec 11, 2026**.

Two buildings — **BSH** and **Founder's Hall (FH)** — with **four subsystems
each, eight ESP32 nodes in total**. Each node is a plain Modbus TCP slave polled
directly by **its own dedicated OpenPLC instance**, each instance running as its
own Docker container on its own LAN IP (`192.168.50.111`–`.124`, macvlan) using
the default ports 8080 and 502. All eight containers run in one Proxmox VM on the Proxmox server (`pve`, 192.168.50.143, operational Oct 9, 2026), and the SCADA/HMI layer (Ignition; license confirmed Oct 19, 2026) runs in its own VM there. One SCADA layer polls all eight and
evaluates the cross-system cascade rules. There are **no protocol-translation
bridges anywhere** — OpenPLC's polling engine only speaks Modbus, so protocol
variety lives at the field-bus layer instead (SPI/RFID at the Door — Power Box
dropped its own field-bus library, DALI, on Sep 28, 2026, in favor of a plain
relay).

The five systems, and the one asymmetry that trips people up:

| Slot | BSH | FH |
|---|---|---|
| 1 | Elevator (3-floor, L293D motor, buttons, display) | **HVAC zone unit** (DHT11, relay, setpoint) |
| 2 | Fire Alarm (buzzer, RGB, pull station, flame, MQ-2 gas) | same, mirrored |
| 3 | Door/Gate (RC522 RFID, servo, IR, reed switch) | same, mirrored |
| 4 | Power Box (relay, lights + fan) | same, mirrored |

**Founder's Hall is not a full mirror of BSH.** Slot 1 differs: BSH has the
Elevator, FH has an HVAC zone unit (adopted Sep 26, 2026, replacing a
previously-planned FH-Elevator). The other four systems are mirrored pin-for-pin
and differ only in their static IP.

Five earlier designs are retired and should never be reintroduced from an old
document: MQTT or BACnet as the backhaul (archived in
`Alternate_Design_BACnet_MQTT_Option.md`), the Air Quality / MQ-135 system
(replaced by Perimeter/Motion, Sep 14), the Wiegand card reader (replaced by
the RC522 RFID module, Sep 15), and the Perimeter/Motion Intrusion Zone system
itself (replaced by a Backup Power / ATS Monitor system, Sep 28), and that
Backup Power / ATS Monitor system (dropped without a replacement on Oct 5,
2026, by Luis together with the course professor; its files are in
`Archive/BackupPower/`).

## 5. Where the build actually stands

Phase B (WiFi + Modbus) is done on every node (bench-confirmed Oct 6-9, 2026). The phases are
A0 Wokwi simulation → A bench hardware → B WiFi + Modbus → C OpenPLC + SCADA.
**Phase B is done on all eight nodes. Phase C (OpenPLC + SCADA) started Oct 13, 2026** with the infrastructure: the OpenPLC Docker VM (`openplc-docker`, 192.168.50.10, Ubuntu 26.04 desktop) is built and running Docker (verified Oct 13, 2026). Later that evening the `openplc:v3` image was built, the macvlan network `cybercity_lan` was created, and the first container (`OpenPLC-BSH-FireAlarm`, 192.168.50.112) started. On Oct 14 the other 7 containers were started with `run_containers.sh`; all 8 are running and their web UIs answer from the laptop. No program or ESP32 slave device is loaded in any of them yet, and the SCADA/HMI VM (VM 101 `scada-hmi`, Ubuntu 24.04 LTS desktop) was built on Oct 14 and has static IP 192.168.50.30 (set Oct 15); it has no SCADA software yet; the ladder programs are drafted in `PLC_Logic/` but have not been compiled or run. See `Next_Steps.md` and `Proxmox_Build_Log.md`.

| System | State |
|---|---|
| BSH-FireAlarm | Phase A and Phase B complete (bench-confirmed 2026-10-06, 192.168.50.12) |
| BSH-Elevator | Phase A complete (2026-09-25); Phase B complete (2026-10-07, 192.168.50.11) |
| BSH-Door | Phase A reported fine by Luis (2026-10-09, polarity flags/pull-up not recorded); Phase B bench-confirmed 2026-10-09 (192.168.50.13) (all registers confirmed by Luis) |
| BSH-PowerBox | Phase A complete (2026-10-05); Phase B complete (2026-10-06, 192.168.50.14); `SYSTEM_REPORT.md` written |
| FH-FireAlarm | Phase B complete (2026-10-06, 192.168.50.22), built ahead of order at Luis's request |
| FH-PowerBox | Phase B complete (2026-10-07, 192.168.50.24), built ahead of order at Luis's request |
| FH-HVAC | Phase A and Phase B complete (2026-10-07, 192.168.50.21), built ahead of order at Luis's request |
| FH-Door | Phase A reported fine by Luis (2026-10-09); Phase B bench-confirmed (2026-10-09, 192.168.50.23); `Firmware/WORKFLOW.md` has the live table and wins |

Build order, for both buildings: Fire Alarm → Elevator (BSH) / HVAC (FH) → Door
→ Power Box. Only one system is "active" at a time.

## 6. What lives where

**`PLC_Logic/` (added Oct 11, 2026):** the OpenPLC side of every system, one subfolder each: `README.md`, `Register_Map.md`, `Ladder_Diagram.md`, `Program.st`, `Build_Log.md`. Shared pieces are in `PLC_Logic/Templates/` and `PLC_Logic/Setup/`. All 8 are drafts, untested. It sits beside `Firmware/` and follows the same one-folder-per-system layout.

**Only in this folder, not in the Claude project:** the two `SYSTEM_REPORT.md`
files, the Wokwi simulation folders, the bench-confirmed firmware, the
per-component `BenchTest.ino` sketches, `Archive/`, and the
`References/GitHub/` notes.

**Only in the Claude project, not in this folder:** ten first-pass WiFi+Modbus
`.ino` sketches at `Firmware/*_Node.ino`, the v1 workflow
(the original v1 workflow file in the Claude project), `Elevator_Motor.ino` (the original blocking
elevator prototype), and `Firmware/BSH-Door/Stepper_Motor_Reference.ino`. These
are **older than** this folder's own firmware and were deliberately left in the
project rather than copied here — this folder's versions are the ones being
built. Do not push them into this folder.

**Mirrored in both, this folder wins on conflict:** this file, the master
workflow, `Firmware/WORKFLOW.md`, both architecture docs, all eight system docs,
the sensor reference sheets, `References/INDEX.md`, the bring-up checklists and
the system READMEs.

## 7. Wireshark / network capture (added Oct 14, 2026)

The team runs Wireshark on a Linux machine to capture the network, and what is
done in the systems, while performing the Section 11 vulnerability scenarios.
Everything about that -- where the capture runs, filters, naming, the per-run
log -- lives in `Security_Capture/`. See `Security_Capture/README.md` first.

**`Kali/` (added Oct 20, 2026)** will hold the attack scripts and attack reports run from the Kali VM (VM 102, 192.168.50.40). It exists but is not in use until the attack-scenario session; see `Kali/README.md`. The pcap evidence stays in `Security_Capture/`.

## 8. Conventions worth knowing before touching anything

- **Plain ASCII only in `.ino` files.** Non-ASCII characters (em dashes
  especially) fail to compile on this project's Windows Arduino IDE setup with
  "extended character is not valid in an identifier", and the error points at the
  wrong line. Use `--`, never `—`.
- **GPIO 34–39 are input-only with no internal pull resistors.** `INPUT_PULLUP`
  there compiles and silently does nothing. External resistor required.
- **GPIO 0, 2, 5, 12, 15 are strapping pins**, sampled at reset to configure
  boot mode / flash voltage / console baud. GPIO 12 (MTDI) is the sharpest
  trap: if it's pulled HIGH at reset (e.g. by a relay module's own IN
  pull-up, before your sketch's `setup()` runs) the chip mis-detects the
  flash voltage and downstream behavior on that pin looks inverted/broken —
  discovered on BSH-PowerBox's lights relay, Sep 28, 2026 (see
  `Piranga_Workflow_v2.md` Section 14). Don't put a pulled-up peripheral on
  a strapping pin without checking this first.
- **Any 5V digital output into a 3.3V GPIO needs a 1kΩ/2kΩ divider** (the
  HC-SR04's ECHO pin, and possibly the MQ-2's analog out — measure it first).
- **Prefer ADC1 pins (32–39) for analog reads;** ADC2 becomes unreliable once
  WiFi is active.
- **No blocking code.** `mb.task()` must run every loop iteration, so `delay()`,
  long `pulseIn()` calls and bit-banged sensor reads all have to be bounded or
  put on a `millis()` timer.
- **One home per file, no duplicate sketches.** `Firmware/WORKFLOW.md` Section 8
  is the map. A superseded variant goes to `Archive/` with a stated reason, never
  beside the live file under a name like `..._Node-1.ino`.

## 9. Daily log (added Oct 15, 2026)

`Logs/` holds the decision log and lessons learned, one file per working day (`Logs/YYYY-MM-DD.md`), with `Logs/INDEX.md` listing the days and `Logs/README.md` holding the rules and the template. It continues `Piranga_Workflow_v2.md` Section 14, which is frozen as history up to Oct 14, 2026.

**Standing instruction for every session:** before ending a session that changed anything, write or update today's file in `Logs/` (done, decisions with rationale, lessons with resolution, open items) and add the day to `Logs/INDEX.md`. Never rewrite an old entry; add a new one that supersedes it. Read the latest `Logs/` file after `Next_Steps.md` to see what happened most recently.
