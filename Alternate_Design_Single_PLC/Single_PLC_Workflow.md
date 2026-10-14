# Automated Cyber-Physical Campus System Design -- Parallel Workflow
## Alternate Architecture: One Real PLC for One System (FH-PowerBox)

**Status:** Exploratory draft for professor review. Direction agreed with the
professor on Sep 28, 2026: scope this to **one** system rather than all ten.
Does not supersede `Piranga_Workflow_v2.md`, which remains the authoritative
plan.
**Hardware:** Allen-Bradley MicroLogix 1400, CAT 1766-L32BXB, Series B,
FW 11 -- confirmed Modbus TCP-capable (Series A units are not).
**Scope:** Only **FH-PowerBox** moves from its OpenPLC Docker container to
the real PLC. **The other nine systems (BSH-Elevator, BSH-FireAlarm,
BSH-Door, BSH-PowerBox, BSH-Perimeter, FH-HVAC, FH-FireAlarm, FH-Door,
FH-Perimeter) are unchanged from `Piranga_Workflow_v2.md`** -- they keep
their own dedicated OpenPLC Docker containers exactly as designed.

**Revision note:** the first draft of this folder explored replacing all 10
OpenPLC instances with one PLC. That version is superseded by this one --
see Section 12 for why the scoped-down version is a meaningfully better fit,
not just a smaller version of the same idea.

---

## Table of Contents
1. [What Changes and What Doesn't](#1-what-changes-and-what-doesnt)
2. [Why FH-PowerBox](#2-why-fh-powerbox)
3. [System Architecture](#3-system-architecture)
4. [The PLC Itself: What It Can and Can't Do](#4-the-plc-itself-what-it-can-and-cant-do)
5. [Network / IP & Register Map](#5-network--ip--register-map)
6. [The Polling Task](#6-the-polling-task)
7. [Cascade Event Flow](#7-cascade-event-flow)
8. [Hardware Checklist Changes](#8-hardware-checklist-changes)
9. [Firmware -- No Changes, But One Known Risk](#9-firmware----no-changes-but-one-known-risk)
10. [Dashboard & Integration Workflow Changes](#10-dashboard--integration-workflow-changes)
11. [Cybersecurity Scenarios -- What Changes](#11-cybersecurity-scenarios----what-changes)
12. [Trade-off Summary](#12-trade-off-summary)
13. [Suggested Bring-up Plan & Timeline](#13-suggested-bring-up-plan--timeline)
14. [Open Questions for the Professor](#14-open-questions-for-the-professor)

---

## 1. What Changes and What Doesn't

**Changes (FH-PowerBox only):**
- `OpenPLC-FH-PowerBox` (the Docker container at 192.168.1.124) is retired
  for this one system and replaced by the real MicroLogix 1400 on its own IP.
- That one system's ladder logic moves from OpenPLC's web UI to RSLogix 500.
- Node-RED's Modbus connection list swaps one entry (FH-PowerBox's
  controller) from the old container IP to the PLC's IP -- everything else
  about the SCADA layer is untouched.

**Does not change -- for the other nine systems, or anything else:**
- BSH-Elevator, BSH-FireAlarm, BSH-Door, BSH-PowerBox, BSH-Perimeter,
  FH-HVAC, FH-FireAlarm, FH-Door, and FH-Perimeter all keep their own
  dedicated OpenPLC Docker containers, unchanged, exactly as
  `Piranga_Workflow_v2.md` describes them.
- The Raspberry Pi / Docker hub keeps running (now 9 OpenPLC containers
  instead of 10, plus Node-RED) -- it is **not** removed, unlike the earlier
  full-replacement draft assumed.
- Every ESP32's firmware, including FH-PowerBox's own sketch
  (`FH_PowerBox_Node.ino`) -- it's already a plain Modbus TCP slave and needs
  no changes to be polled by a real PLC instead of a software one.
- The cascade rules, the dashboard layout, and all 10 cybersecurity
  scenarios' teaching goals (mechanics for Scenario 7 shift slightly --
  Section 11).
- The project's per-system-controller design principle (Section 2 of the
  real workflow) -- if anything, this hybrid version makes that principle
  *more* real, not less: FH-PowerBox now runs on a genuinely different
  vendor's controller than its nine siblings, which is exactly the
  "distinct subsystems, distinct controllers" story the real project is
  already telling.

---

## 2. Why FH-PowerBox

This was Luis's choice, made with the professor. Worth noting for the record
(not to relitigate the choice, just so the trade-offs are visible):

- **In favor:** Power Box is a strong demo system -- DALI-dimmed lighting and
  a fan under a real PLC's control is a visibly compelling "this is a real
  industrial controller doing real work" moment, arguably more so than an
  alarm buzzer or a status LED. Scenario 7 (unauthorized Modbus write
  flipping the dim level or fan with no logged source) becomes a genuinely
  strong live red-team demo against real hardware. FH was chosen over BSH,
  which doesn't change any technical risk -- the two Power Box nodes are
  identical except for IP and building label.
- **Worth flagging, not a blocker:** FH-PowerBox's firmware
  (`FH_PowerBox_Node.ino`) is documented in `Piranga_Workflow_v2.md` Section
  9 as first-pass complete but **not yet bench-tested**, and the DALI-ESP32
  library it depends on is separately flagged there as "the piece of
  firmware with the least precedent in this project." That risk exists
  whether Power Box runs under OpenPLC or a real PLC -- it isn't created by
  this option -- but choosing Power Box as the one system to build a new PLC
  integration around does mean the project's two least-proven things (a
  real-PLC integration nobody's done before, and a DALI library nobody's
  bench-tested yet) land on the same system at the same time. See Section 13
  for a suggested sequencing that de-risks this.

---

## 3. System Architecture

```
                         +---------------------------------------------+
                         |           SCADA / Dashboard Layer            |
                         |         (Node-RED, on hub 192.168.1.10)      |
                         |   Polls all 10 controllers via Modbus TCP -- |
                         |   9 software, 1 real, all look the same to   |
                         |   Node-RED                                    |
                         +---+---+---+---+---+---+---+---+---+----------+
                             |   |   |   |   |   |   |   |   |
                    (9x OpenPLC, unchanged)     |   (1x REAL PLC)
                             |   |   |   |   |   |   |   |   |
                        +----v-+ ... +---+   +---v----------------+
                        |OpenPLC|          |  MicroLogix 1400      |
                        |-BSH-  |          |  (1766-L32BXB)        |
                        |Elev.  |          |  ONE ESP32 to poll:   |
                        |(.111) |          |  FH-PowerBox (.24)    |
                        +---+---+          |  1 read MSG + 1 write |
                            |               |  MSG per scan -- no   |
                       (unchanged,          |  round-robin needed   |
                        Modbus TCP)         +---+--------------------+
                            |                   |
                        +---v---+           Modbus TCP
                        | ESP32 |               |
                        | Elev. |           +---v---+
                        +-------+           | ESP32 |
                                             |FH-Pwr |
                                             |(DALI) |
                                             +-------+
```

The whole point of this diagram versus the earlier all-10 draft: **from
Node-RED's point of view, nothing looks different.** It still polls 10
Modbus TCP servers on 10 IPs. Only one of those IPs now happens to be a real
PLC instead of a Docker container -- and that PLC only has to talk to one
downstream ESP32, not ten.

---

## 4. The PLC Itself: What It Can and Can't Do

Unchanged from the first draft -- restated here since it's still the basis
for everything else:

| Capability | Status |
|---|---|
| Modbus TCP server (SCADA polls the PLC) | Yes, native, port 502 |
| Modbus TCP client/master (PLC polls its ESP32) | Yes, via MSG instructions |
| EtherNet/IP | Yes, runs simultaneously with Modbus TCP, independent sessions |
| Physical I/O terminals (20 DI / 12 DO on this unit) | **Not used.** FH-PowerBox's ESP32 is polled over the network, same as every OpenPLC instance does today -- nothing needs to be wired to the PLC's own terminal blocks |
| Program memory | 20 KB -- comfortably enough for one system's worth of safe-state/pass-through logic |
| Programming software | RSLogix 500 v8.3+ required -- **still needs to be confirmed as available/licensed**, though now only one system's worth of ladder needs to be written, a much smaller ask than before |

---

## 5. Network / IP & Register Map

### 5.1 IPs

Only one row changes from `Piranga_Workflow_v2.md` Section 6:

| System | ESP32 IP (unchanged) | Controller | Controller IP |
|---|---|---|---|
| BSH-Elevator | 192.168.1.11 | OpenPLC-BSH-Elevator | 192.168.1.111 (unchanged) |
| BSH-FireAlarm | 192.168.1.12 | OpenPLC-BSH-FireAlarm | 192.168.1.112 (unchanged) |
| BSH-Door | 192.168.1.13 | OpenPLC-BSH-Door | 192.168.1.113 (unchanged) |
| BSH-PowerBox | 192.168.1.14 | OpenPLC-BSH-PowerBox | 192.168.1.114 (unchanged) |
| BSH-Perimeter | 192.168.1.15 | OpenPLC-BSH-Perimeter | 192.168.1.115 (unchanged) |
| FH-HVAC | 192.168.1.21 | OpenPLC-FH-HVAC | 192.168.1.121 (unchanged) |
| FH-FireAlarm | 192.168.1.22 | OpenPLC-FH-FireAlarm | 192.168.1.122 (unchanged) |
| FH-Door | 192.168.1.23 | OpenPLC-FH-Door | 192.168.1.123 (unchanged) |
| **FH-PowerBox** | 192.168.1.24 (unchanged) | **the real MicroLogix 1400** | **192.168.1.100 (proposed -- was 192.168.1.124)** |
| FH-Perimeter | 192.168.1.25 | OpenPLC-FH-Perimeter | 192.168.1.125 (unchanged) |

`.124` is freed up when the Docker container is retired for this system; the
PLC gets a new address outside the `.111`-`.125` container block so it's
visually obvious in network logs which node is the real hardware.

### 5.2 FH-PowerBox register map inside the PLC

Only one system's points need mapping -- far simpler than the all-10 flat
map the first draft needed. Based on the Power Box register map documented
in `FH_Systems/04_FH-PowerBox.md` / `BSH_Systems/04_BSH-PowerBox.md` (DALI
dim level, fan relay coil, and whatever status/fault bits that system
defines):

| Point | Direction | Proposed PLC address |
|---|---|---|
| Fan relay coil | PLC -> ESP32 | B10:0/0 |
| DALI dim-level command | PLC -> ESP32 | N15:0 (holding register) |
| DALI/fan status or fault bits | ESP32 -> PLC | B9:0/0-3 |
| Any reported analog feedback | ESP32 -> PLC | N7:0 |

Confirm this against the actual point list in `FH_Systems/04_FH-PowerBox.md`
before programming -- this is a proposed starting point, not the verified
map.

---

## 6. The Polling Task

This is the section that changes the most from the first draft, and for the
better. With all 10 systems on one PLC, the project needed a hand-built
round-robin MSG sequencer -- genuinely hard, unproven-timing new engineering
(see the retired all-10 draft if you want the details). **With only
FH-PowerBox on the real PLC, that problem mostly goes away.**

The PLC now needs, at most, two MSG blocks total: one reading FH-PowerBox's
ESP32 (status bits + any analog feedback) and one writing to it (fan coil +
dim level). No index register, no round-robin, no "which node am I polling
right now" logic. This is close to the simplest possible real-PLC
integration this project could have chosen, independent of whether
Power Box was the right system pick otherwise (Section 2).

**Still worth bench-measuring, not assuming:** how long that one read + one
write MSG pair takes end-to-end, and whether it comfortably clears the
project's existing 3-5 second communication watchdog -- but with only one
node in the loop instead of ten, this is a much smaller and much faster
thing to verify.

---

## 7. Cascade Event Flow

No changes needed to `Piranga_Workflow_v2.md` Section 7's design at all.
Node-RED already treats every controller uniformly through Modbus TCP,
regardless of what implements it -- a cascade rule that reads FH-PowerBox's
status and writes into, say, FH-FireAlarm's coil works exactly the same
whether FH-PowerBox's controller is a Docker container or a real PLC. This
is the main structural benefit of scoping this down to one system: it slots
into the existing cascade design with zero special-casing, where the all-10
version would have forced a real decision about where cascade logic lives
(see the retired draft's Section 7 for why).

---

## 8. Hardware Checklist Changes

Relative to `Piranga_Workflow_v2.md` Section 8:

**Keep, unchanged:**
- Raspberry Pi 4 (or laptop) running the Docker hub -- now 9 OpenPLC
  containers instead of 10, plus Node-RED. (The first draft of this folder
  incorrectly assumed the Pi/Docker hub could be removed entirely -- that
  was only true for the full-replacement version, not this one.)
- Every other system's hardware, wiring, and 3D-printed structure --
  untouched.

**Add:**
- The MicroLogix 1400 unit (1766-L32BXB or equivalent), confirmed available.
- 24VDC power supply for the PLC.
- DIN rail / mounting hardware if it needs to be panel-mounted for the demo.
- A network connection for the PLC into the same LAN as everything else
  (one more switch port, not a new network segment).
- RSLogix 500 software license/seat -- needs confirmation, though now only
  needed for whoever programs this one system.

**Unchanged:** FH-PowerBox's own field hardware (ESP32, DALI transceiver,
DALI-addressable fixture, relay module) -- same BOM as documented in Section
8 of the real workflow, same as its BSH mirror.

---

## 9. Firmware -- No Changes, But One Known Risk

`FH_PowerBox_Node.ino` needs no code changes to be polled by a real PLC
instead of OpenPLC -- it's already a Modbus TCP slave, and a slave doesn't
know or care what kind of master is talking to it. This holds for all nine
other systems too, which are entirely unaffected.

The one thing worth being deliberate about (raised in Section 2): this
firmware is first-pass complete but not yet bench-tested, and its DALI
library is the project's single highest-risk dependency per
`Piranga_Workflow_v2.md` Section 9. Section 13 below suggests bench-testing
FH-PowerBox through its normal Phase A/B steps -- standalone, against
OpenPLC even, if that's faster to stand up -- before wiring the real PLC
into the picture, so that a DALI bring-up problem and a PLC-integration
problem don't get debugged at the same time.

---

## 10. Dashboard & Integration Workflow Changes

Relative to `Piranga_Workflow_v2.md` Section 10, this is now a small,
contained change:

1. Hub setup: 9 OpenPLC containers instead of 10 (drop `OpenPLC-FH-PowerBox`
   from the Docker deployment), Node-RED unchanged.
2. The real PLC gets configured once: the register map from Section 5.2,
   the two MSG blocks from Section 6, and whatever safe-state/fail-direction
   logic FH-PowerBox's own system doc specifies (same behavior OpenPLC's
   ladder would have implemented for it).
3. Node-RED: swap one Modbus connection node's target IP from `.124` to the
   PLC's IP (`.100` proposed). Nothing else about the dashboard, cascade
   rules, or event log changes.

---

## 11. Cybersecurity Scenarios -- What Changes

Only Scenario 7 (Power Box Manipulation) is affected, and only in a good
way: an unauthorized Modbus write against FH-PowerBox now lands on a real
Allen-Bradley PLC's Modbus server instead of a software one -- about as
authentic an OT attack demo as a student testbed can offer, and a strong
candidate for the project's live red-team demonstration (Scenario 10). The
other nine scenarios are untouched, since their systems don't change at all.
The "10 separate IPs on port 502" property from Section 6 of the real
workflow is fully preserved -- FH-PowerBox just happens to be a different
kind of device behind its IP now.

---

## 12. Trade-off Summary

This compares three states: the current all-OpenPLC plan, the retired
all-10-real-PLC draft, and this one-system hybrid.

| | 10 OpenPLC (current plan) | 1 real PLC for all 10 (retired draft) | 1 real PLC for FH-PowerBox only (this doc) |
|---|---|---|---|
| Real, tangible PLC in the demo | No | Yes | **Yes** |
| Matches "separate vendor controllers per subsystem" narrative | Yes | No -- collapses to one shared controller | **Yes, arguably strengthened** -- one system is now on a genuinely different vendor's hardware than the rest |
| Single point of failure | No | Yes -- losing the PLC takes down all 10 systems | **No** -- losing the PLC only takes down FH-PowerBox, same blast radius as losing any one OpenPLC container |
| Scenario 9 (Node Isolation/Resilience) | Demonstrable | Not demonstrable as written | **Demonstrable, unchanged** |
| New engineering required | None | A full 10-node MSG round-robin sequencer, unproven timing | **One read MSG + one write MSG**, much smaller and easier to bench-test |
| Firmware impact | None | None | None |
| Programming environment | OpenPLC web UI only | RSLogix 500 for all 10 systems' logic | RSLogix 500 for **one** system's logic |
| Parallel team work | Yes, fully | No -- one physical unit is a bottleneck for all 10 systems | Mostly yes -- only FH-PowerBox's work funnels through the one PLC; the other nine stay parallel |
| Hub hardware | Pi/Docker only | Pi/Docker removed, replaced by the PLC | **Both** -- Pi/Docker for nine systems, PLC for one |
| Biggest remaining risk | (none new) | Sequencer timing vs. watchdog, single point of failure, licensing | FH-PowerBox's own DALI library is unproven, independent of the PLC swap (Section 2, Section 9) |

The scoped-down version keeps essentially everything that made the original
per-system design good, while still delivering what the professor actually
asked for: a real, physical PLC the project can point to and demonstrate.

---

## 13. Suggested Bring-up Plan & Timeline

Today is Sep 28, 2026; the deadline is Dec 11, 2026 (~10.5 weeks). Nothing
built so far needs to be redone under this option -- Phase C hasn't started
for any system yet, including FH-PowerBox.

Because FH-PowerBox's own firmware is unproven (Section 2, Section 9),
suggested order:

1. Bring FH-PowerBox through Phase A (standalone hardware bring-up) and
   Phase B (Modbus connectivity) the same way every other system does,
   independent of which kind of PLC will eventually poll it. This is where
   the DALI library risk gets found and dealt with, on its own, without a
   PLC integration complicating the debugging.
2. Once FH-PowerBox is Modbus-TCP-confirmed and bench-stable, confirm
   RSLogix 500 access and stand up the PLC on its own IP.
3. Build the two MSG blocks (Section 6) and the register map (Section 5.2)
   against the now-proven ESP32 node, and bench-measure timing against the
   watchdog.
4. Wire FH-PowerBox into Node-RED via the PLC (Section 10) and confirm
   dashboard + cascade behavior end-to-end.

Because this option only touches one system and needs a much smaller new
engineering piece than the retired all-10 draft, it should fit comfortably
inside the existing build-order timeline without displacing the other nine
systems' work.

---

## 14. Open Questions for the Professor

- Is FH-PowerBox's not-yet-bench-tested firmware (and its unproven DALI
  library specifically) an acceptable risk to pair with the first-ever
  real-PLC integration in this project, or would the professor rather see
  FH-PowerBox proven out under OpenPLC first, then swapped to the real PLC
  once it's a known-good system?
- Is RSLogix 500 available (licensed) for the one system's worth of ladder
  logic this now requires?
- Does the professor want the real PLC to *replace* FH-PowerBox's OpenPLC
  container outright, or would a side-by-side (both exist, demo can toggle
  which one is live) be more useful for the presentation?
- Should the live red-team demo (Scenario 10) be built specifically around
  attacking FH-PowerBox now, given it's the one system running on real
  industrial hardware?

---

*This document is a parallel exploration, not a plan of record. It does not
edit or replace anything in `Piranga_Workflow_v2.md` or `Firmware/WORKFLOW.md`.*
