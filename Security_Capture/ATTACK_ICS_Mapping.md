# MITRE ATT&CK for ICS — scenario mapping (Deliverables 6 and 10)

Created Nov 7, 2026. Maps each lab scenario (`Scenario_Capture_Plan.md`) to the MITRE
ATT&CK for ICS framework: the **tactic** (why), the **technique ID(s)** (how), what the
lab demonstrates, and the **mitigation M-code(s)** (the defensive recommendation). The
technique → mitigation pairing is what makes this a complete red-team + blue-team
deliverable.

**Source.** IDs pulled from `attack.mitre.org` on Nov 7, 2026:
[Techniques](https://attack.mitre.org/techniques/ics/) ·
[Tactics](https://attack.mitre.org/tactics/ics/) ·
[Matrix](https://attack.mitre.org/matrices/ics/) ·
[Mitigations](https://attack.mitre.org/mitigations/ics/). The scenario-to-technique
mapping is **our analysis** — confirm each ID against the framework before the final
report. The matrix page also drives the **ATT&CK Navigator**, which can render a
highlighted "coverage" layer of the techniques below for a report/slide.

**ID reconciliation (important).** The IRONCLAD reference project (`ics-railroad-cybersecurity-lab`
skill) uses the older IDs **T0855 (Unauthorized Command Message)** and **T0856 (Spoof
Reporting Message)**. MITRE has reorganized these under **T1692 Unauthorized Message**
(`.001` Command Message, `.002` Reporting Message). Same concepts, renamed — this doc uses
the current IDs and notes the old ones so the two projects line up.

## Tactics this lab exercises (the matrix columns)

| Tactic | ID | In this lab |
|---|---|---|
| Discovery | TA0102 | Recon: scan, tag identification, sniffing |
| Execution | TA0104 | Scenario 12 (upload/run attacker logic) |
| Inhibit Response Function | TA0107 | DoS; defeating a safety response |
| Impair Process Control | TA0106 | #4, #5, #7, #11 — the Modbus writes |
| Impact | TA0105 | The consequence: Loss of Safety / Control / Availability / View |

(The full framework has 12 tactics; the others — Initial Access TA0108, Persistence TA0110,
Privilege Escalation TA0111, Evasion TA0103, Lateral Movement TA0109, Collection TA0100,
Command and Control TA0101 — are not the focus of this closed-lab testbed.)

## Scenario → technique → mitigation

| # | Scenario | Tactic | Technique ID(s) | What the lab shows | Mitigation M-code(s) |
|---|---|---|---|---|---|
| 1 | Normal operation | — | — (baseline) | The steady-state polling the attacks are compared against | — |
| 2 | Authorized door access | — | — (physical, legitimate) | A real badge event as the comparison baseline for #3 | — |
| 3 | Unauthorized remote door-open (Modbus injection) *(revised Nov 8, 2026, replaces RFID clone)* | Impair Process Control / Impact | **T1692.001** (Unauthorized Message: Command Message; old **T0855**) · **T0831** Manipulation of Control · impact **T0827** Loss of Control | FC 5 write to `DOOR_OPEN_CMD` (coil 0, `.113`) from a rogue host — gate opens with no `ACCESS_GRANTED` pulse (contrast #2). *Physical-layer cousin (RFID UID cloning) kept as a report-only note.* | **M0802** Communication Authenticity · **M0800** Authorization Enforcement · **M0807** Network Allowlists · **M0937** Filter Network Traffic |
| 4 | False fire alarm injection | Impair Process Control / Impact | **T1692.001** (Unauthorized Message: Command Message; old **T0855**) · **T0831** Manipulation of Control · impact **T0880** Loss of Safety | FC 5 write to `ALARM_ACTIVATE` from a rogue host, alarm sounds with no real pull | **M0802** Communication Authenticity · **M0800** Authorization Enforcement · **M0807** Network Allowlists · **M0937** Filter Network Traffic · **M0812** Safety Instrumented Systems |
| 5 | Elevator lockout (+ persistent variant) | Impair Process Control / Impact | **T1692.001** (old **T0855**) · **T0831** Manipulation of Control · impact **T0827** Loss of Control | Setting `ELEVATOR_DISABLE`; the persistent variant re-asserts it on a timer to beat the operator (Industroyer-style) | **M0802** Communication Authenticity · **M0800** Authorization Enforcement · **M0807** Network Allowlists · **M0930** Network Segmentation |
| 6 | Building-wide power cut (FC 15) *(repurposed Nov 15, 2026)* | Impair Process Control / Impact | **T1692.001** (old **T0855**) · **T0831** Manipulation of Control · impact **T0826** Loss of Availability | One FC 15 Write Multiple Coils frame cuts all three PowerBox rails at once — a coordinated multi-point outage | **M0802** Communication Authenticity · **M0807** Network Allowlists · **M0937** Filter Network Traffic · **M0930** Network Segmentation |
| 7 | Power box manipulation | Impair Process Control / Impact | **T1692.001** (old **T0855**) · **T0831** Manipulation of Control · impact **T0826** Loss of Availability | A coil write flips a power rail from an unexpected IP | **M0802** Communication Authenticity · **M0807** Network Allowlists · **M0937** Filter Network Traffic |
| 8 | Cross-building cascade | Impair Process Control | **T1692.001** (old **T0855**) · **T0831** Manipulation of Control | A door event drives SCADA writes to both Fire Alarm nodes — the order/timing of the writes | **M0802** Communication Authenticity · **M0930** Network Segmentation · **M0931** Network Intrusion Prevention |
| 9 | Node isolation / resilience (DoS) | Inhibit Response Function / Impact | **T0814** Denial of Service · impact **T0826** Loss of Availability, **T0829** Loss of View | A container/node stops or is flooded; the other seven keep answering; SCADA flags it | **M0815** Watchdog Timers (the heartbeat/fail-safe design) · **M0811** Redundancy of Service · **M0931** Network Intrusion Prevention · **M0937** Filter Network Traffic |
| 10 | Red / Blue exercise | (composite) | Combines #3, #4, #5, #7, #11 | One timeline the Blue Team reviews afterward | The union of the above |
| 11 | HVAC spoofing / setpoint tamper | Impair Process Control / Impact | **T0836** Modify Parameter · **T0831** Manipulation of Control · (if the shown value is forged) **T1692.002** Reporting Message (old **T0856**) / **T0832** Manipulation of View | A holding-register write to `TARGET_TEMP_C` from a foreign IP (Oldsmar analogue) | **M0802** Communication Authenticity · **M0800** Authorization Enforcement · **M0807** Network Allowlists · **M0818** Validate Program Inputs |
| 12 | PLC logic injection (OpenPLC `:8080`) | Execution / Impact | **T0843** Program Download (`.001` Download All / `.002` Online Edit / `.003` Program Append) · **T0889** Modify Program · **T0842** Network Sniffing (cleartext creds) | Upload/compile/reload attacker logic via the web UI; **CVE-2026-88020** (XSS) gives a credential-less path | **M0818** Validate Program Inputs (the XSS fix) · **M0808** Encrypt Network Traffic · **M0804** Human User Authentication · **M0927** Password Policies · **M0932** MFA · **M0951** Update Software (v4) · **M0953** Data Backup (restore) |
| Recon | Pre-attack reconnaissance | Discovery | **T0846** Remote System Discovery (`.001` Port Scan) · **T0861** Point & Tag Identification · **T0842** Network Sniffing · **T0888** Remote System Information Discovery | nmap finds all eight :502 servers; sniffing reveals the cleartext `:8080` login | **M0930** Network Segmentation · **M0937** Filter Network Traffic · **M0808** Encrypt Network Traffic |
| 13 | ESP32 bricking (spare board; destructive, last) | Impact (+ firmware) | **T0857** System Firmware / **T0839** Module Firmware · impact **T0826** Loss of Availability (+ T0809 Data Destruction) | erase a spare node's flash → it won't boot; recover by reflash (eFuse permanent brick documented, not performed) | **M0953** Data Backup (keep per-node firmware images) · physical access control to the USB/flash port (**M0805** Mechanical Protection Layers) |
| 14 | IRONGATE-style combined MITM (false view) *(added Nov 8, 2026; gated on the Nov 9 network check)* | Collection / Impair Process Control / Impact | **T0830** Adversary-in-the-Middle · **T0832** Manipulation of View · **T0856** Spoof Reporting Message · (suppress direction) impact **T0880** Loss of Safety | ARP-spoof MITM at both links (ESP32↔container A, container↔Ignition B): feed the PLC false field data and forge the operator's view / swallow operator commands — the pcap tell is two Modbus streams that disagree | **M0814** Static Network Configuration (anti-ARP-spoof) · **M0802** Communication Authenticity · **M0931** Network Intrusion Prevention (Dynamic ARP Inspection) · **M0937** Filter Network Traffic · **M0930** Network Segmentation |

## Mitigation glossary (M-codes used above)

| M-code | Name | Why it applies here |
|---|---|---|
| M0800 | Authorization Enforcement | Modbus obeys any sender; enforce who may issue commands |
| M0802 | Communication Authenticity | **Root-cause fix:** Modbus/TCP has no sender authenticity |
| M0804 | Human User Authentication | Replaces the kept default OpenPLC web login |
| M0807 | Network Allowlists | Only each PLC may speak Modbus to its own ESP32 — blocks the rogue Kali write |
| M0808 | Encrypt Network Traffic | Fixes the cleartext `:8080` credentials (CVE-2026-88020) |
| M0811 | Redundancy of Service | Limits the impact of a single-node DoS |
| M0812 | Safety Instrumented Systems | The fire alarm is a safety function (TRITON lesson) |
| M0814 | Static Network Configuration | Defends against ARP spoof / MITM |
| M0815 | Watchdog Timers | **Already in the design** — the HEARTBEAT and fail-safe timeout |
| M0818 | Validate Program Inputs | The XSS (CVE-2026-88020) is an input-validation failure |
| M0927 | Password Policies | Default-credential problem on the web UI |
| M0930 | Network Segmentation | Isolate OT; cut the attacker's reach |
| M0931 | Network Intrusion Prevention | Detect the Modbus writes / scans |
| M0932 | Multi-factor Authentication | Hardens the OpenPLC web login |
| M0937 | Filter Network Traffic | Block non-PLC hosts from port 502 / 8080 |
| M0951 | Update Software | The CVE-2026-88020 remediation (OpenPLC v4) |
| M0953 | Data Backup | Recovery — the Gateway backups and `Program.st` copies |

## Caveats

- **This mapping is our analysis**, built from the Nov 7, 2026 framework pages; confirm each
  ID against `attack.mitre.org` before the final report.
- Several mitigations above **cannot be fully applied without changing the architecture**
  (Modbus has no authentication); for the pure-Modbus case MITRE even lists **M0816
  "Mitigation Limited or Not Effective."** State honestly which mitigations are feasible on
  this testbed vs. which need a redesign.
- Scenario 2 is a physical/legitimate event with no clean ATT&CK-ICS *network* technique — do not
  force a tag. *(Scenario 3 was revised Nov 8, 2026 from the reader-level RFID clone to a Modbus
  door-open injection, so it now carries T1692.001 · T0827 like the other command-injection cases.)*
