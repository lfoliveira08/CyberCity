# CyberCity — rules for any agent working in this folder

This file loads automatically. Read it fully before your first tool call. It is the contract; the longer docs are the detail.

## 0. Orient first, in this order

1. `STATUS.md` — what is done, what is next, as of today. **Read this before answering any "where are we" question.**
2. `00_START_HERE.md` — what the project is, what lives where.
3. `AGENT_HANDOFF.md` — open repairs to the workspace itself (connections, contradictions, duplicated status). Check it before you start; it may contain a task that changes how you should work.
4. The one doc for the layer you are touching (see §3).

Four files, four jobs, no overlap: `CLAUDE.md` = rules · `STATUS.md` = where the project stands · `Next_Steps.md` = the next build step · `AGENT_HANDOFF.md` = what is broken about the workspace. Do not restate one inside another.

Do not read `Piranga_Workflow_v2.md` whole (94 KB). It has a Table of Contents at the top — read only the section you need.

## 1. Precedence — when two documents disagree

1. **This folder** beats the Claude project ("Cyber CIty") mirror, always. Never copy project content into this folder.
2. **`Piranga_Workflow_v2.md`** — architecture, networking, protocols, scenarios, deliverables. Its Section 14 decisions log is frozen as history through Oct 14, 2026.
3. **`Firmware/WORKFLOW.md`** — firmware process, build order, phase definitions, status.
4. **`Logs/`** — decisions and lessons from Oct 15, 2026 on; continues Section 14. Newer entry wins over an older one.
5. **`References/Ignition/`** — how Ignition itself works. Beats your memory, always (see §4).
6. Per-system `README.md` / `Build_Log.md` — the detail for that one system.

A doc that states a project *decision* (addresses, names, IPs) beats the vendor manual. The vendor manual beats any doc about how the *product* behaves.

## 2. Hard facts — do not re-derive these, and do not accept a doc that contradicts them

- **Eight ESP32 nodes**, two buildings, four subsystems each. **Not ten.** Older docs, the `References/GitHub/` notes, `Archive/`, and the claude.ai project *description* field still say ten; that is wrong and known. FH has **no** elevator and **no** perimeter system.
- Slots: BSH = Elevator, FireAlarm, Door, PowerBox · FH = **HVAC**, FireAlarm, Door, PowerBox.
- One ESP32 ⇄ one dedicated OpenPLC Docker container ⇄ one Ignition Gateway. No protocol bridges.

| | ESP32 | OpenPLC container |
|---|---|---|
| BSH-Elevator | .11 | .111 |
| BSH-FireAlarm | .12 | .112 |
| BSH-Door | .13 | .113 |
| BSH-PowerBox | .14 | .114 |
| FH-HVAC | .21 | .121 |
| FH-FireAlarm | .22 | .122 |
| FH-Door | .23 | .123 |
| FH-PowerBox | .24 | .124 |

All `192.168.50.x`. Infrastructure: `pve` .143 (Proxmox) · VM 100 `openplc-docker` .10 · VM 101 `scada-hmi` .30 (Ignition 8.3.10 Gateway). Containers use ports 8080 (web) and 502 (Modbus).

- Ignition talks **only** to the OpenPLC containers, never to an ESP32.
- Every node exposes `HEARTBEAT` and `RECONNECT_COUNT` as its last two Input Registers.

## 3. Which doc owns which layer

| Working on | Owner folder | Never edit instead |
|---|---|---|
| ESP32 firmware, wiring, bench tests | `Firmware/<System>/` + `Firmware/WORKFLOW.md` | — |
| OpenPLC ladder/ST, register maps | `PLC_Logic/<System>/` | the Editor's `build/` output |
| Ignition tags, screens, cascade | `SCADA_HMI/<System>/` (+ its `CLAUDE.md`) | `References/Ignition/` |
| Sensors, components | `BSH_Sensors/`, `FH_Sensors/` | — |
| Attack scenarios, captures | `Security_Capture/`, `Kali/` | — |
| Bill of materials, costs | `BOM/` (summarises the docs above; the source docs win) | — |

## 4. Ignition: the manual is ground truth

The full Ignition 8.3 User Manual is at `References/Ignition/` (1,571 pages). **Before any Ignition setup step, setting name, menu path, scripting function or tag/UDT/Perspective instruction: find the page, read it, cite its path.** Never answer Ignition how-to from memory. If it is not covered, say "not in the manual" — do not guess. Start at `References/Ignition/README.md` (topic → folder map). Version is 8.3 only; the Gateway runs 8.3.10. Never edit that folder.

## 4b. ESP32 hardware: the ESP-IDF guide is ground truth

The ESP-IDF Programming Guide v5.3.2 (ESP32 target) is at `References/ESP-IDF/` as 190 Markdown files. **Before stating an ESP32 hardware fact — ADC behavior, which GPIOs lack pull-ups, PWM channels, Wi-Fi reconnect, watchdogs — look it up there and cite the file path.** Start at `References/ESP-IDF/README.md` (topic map). Two limits: it is **ESP-IDF, not the Arduino core**, so hardware facts apply but code examples do not paste into our sketches; and it has **no diagrams** — open the PDF in `References/Guides/` for any figure. The Technical Reference Manual and the WROOM-32 datasheet are not in the folder. Never edit it.

## 5. Searching this folder

`References/Ignition/` (1,571 files) and `References/ESP-IDF/` (190 files) will swamp any keyword search. **Exclude both unless the question is Ignition how-to or an ESP32 hardware question:**

```bash
grep -rIn "pattern" . --include=*.md --exclude-dir=Ignition --exclude-dir=ESP-IDF --exclude-dir=Archive
```

**Never read, and never build from:** `Archive/` (retired by decision, kept only as history) · `Claude outputs/` · `PLC_Logic/*/*-ESP32/build/` (regenerable) · `Prototype_*/` (unapproved ideas, not the design; see its README). Read them only if Luis names them.

## 6. Git is not reachable from an agent shell

`.git` here is a gitlink to `C:/Users/lfoli/git-data/cybercity.git`, outside the connected folders, so `git` commands fail from the Linux shell and **you cannot see history or diffs**. `Logs/` is the substitute — that is why the log rules are strict. Do not try to re-init a repo here. (To make git work, Luis connects `C:\Users\lfoli\git-data` as a folder too; then use `git --git-dir=$HOME/mnt/git-data/cybercity.git --work-tree=$HOME/mnt/CyberCity <cmd>`.)

## 7. How to work

- **Teach, don't do.** Luis builds the hardware, the ladder logic and the SCADA himself so he learns it. Give one step at a time, say why, cite the page or doc, and let him do the clicking and the wiring. Do not hand over a finished artifact he did not ask for.
- **Verified vs reported vs assumed.** Label every claim. "Bench-confirmed", "Luis reported", "not verified". Never promote a reported result to confirmed.
- **Say what you did not check.** An open item written down is worth more than a confident summary.
- **One system at a time.** Only one system is "active"; do not drift into the next one.
- **Change the pair, not the half.** A register change touches firmware, the system doc, the bring-up checklist and the PLC Input Register count — all four or none. A tag change touches `Tag_Map.md` and `Tags.json` in the same sitting.
- **Log the same day.** A session that changes anything ends by writing `Logs/YYYY-MM-DD.md` and a line in `Logs/INDEX.md`. Never rewrite an old entry; supersede it with a new one that names what it replaces.
- **Update `STATUS.md`** when status changes. It is the only place status lives.
- **File names carry no version.** No `_v2`, `_final`, `_new`. Exports carry a date: `CyberCity_project_2026-10-26.zip`.
- **Dates:** today's real date, never a guessed one. State the date of any status you report.

## 8. Do not

- Invent a value, a pin, a register, a setting name or a menu path. Say you do not know and where to look.
- Change the architecture to make something easier. Raise it with Luis instead.
- Keep the only copy of anything in the Gateway or a container — it gets written down here.
- Put credentials, keys or passwords in any file in this folder.
