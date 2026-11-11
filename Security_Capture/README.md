# Security_Capture -- Wireshark evidence for the attack scenarios

Created Oct 14, 2026. This folder holds everything about **capturing network
evidence while the Section 11 scenarios (`Piranga_Workflow_v2.md`) are
performed**. It is the one home for this activity; nothing else in the project
tree describes the capture procedure.

## The setup, as stated by Luis (Oct 14, 2026)

Wireshark runs on a **Linux machine** and records the network, and what the team
does in the systems, while each vulnerability is performed. Each run produces
one evidence package: a `.pcapng` file plus a short log of what was done and
when.

## Files

| File | What it is |
|---|---|
| `Capture_Setup.md` | Where the Linux machine sits, what it can and cannot see, interface, capture filter, display filters, clock, ring-buffer settings |
| `Scenario_Capture_Plan.md` | Per scenario: what to capture, what the traffic should show, which filter proves it |
| `Threat_Model_Scope.md` | The report's threat model and scope: the assumed attacker, in-scope vs out-of-scope ATT&CK tactics with rationale, and the attack-tier statement |
| `Real_World_Incidents.md` | Background for Deliverables 6/10: documented real-world ICS incidents, what each teaches, the lab scenario it maps to, and sources to cite |
| `ATTACK_ICS_Mapping.md` | MITRE ATT&CK for ICS mapping for Deliverables 6/10: each scenario → tactic → technique ID → mitigation M-code, with a mitigation glossary |
| `Scenarios.md` | The detailed scenario catalog: each scenario's target, Modbus object/FC, action, expected effect, the real-world attack it models, ATT&CK IDs, CVEs and capture filter |
| `References.md` | Citations for Deliverables 6/10: the MITRE ATT&CK links, CISA advisories, primary incident reports, and every CVE tied to its scenario |
| `Kali/` | Kali-side work (moved here Nov 7, 2026): attack runbooks and reports, plus `Kali/Scripts/` — the lab-only flash-drive script policy + manifest (scripts never committed; a `.gitignore` blocks them) |
| `Capture_Log_Template.md` | Copy this for every run; it ties the pcap to the actions and timestamps |
| `Captures/` | The `.pcapng` files and filled-in logs (see `Captures/README.md` for naming) |

## Rules

1. **Closed lab only.** Capture and attack only the testbed subnet
   `192.168.50.0/24` (ESP32 nodes `.11`-`.24`, OpenPLC containers
   `.111`-`.124`). Keep the capture filter on that subnet so nothing else on the
   lab router is recorded.
2. **One run = one pcap + one log.** Start the capture *before* the first action
   and stop it *after* the last one, with a few seconds of normal traffic on
   each side as a baseline.
3. **Record every action with a wall-clock time** in the run log. A pcap alone
   does not say what the team did.
4. **The clock must be right.** `pve` has no working NTP on the lab network
   (see `Proxmox_Build_Log.md`). Read the clock of the Linux machine and of the
   machine performing the action at the start of each session and note the
   difference in the log.
5. **Do not push large captures to the Claude project.** Keep them here;
   the project is a read-only mirror of the design documents.

## Related folder

Attack scripts and attack reports (what was run from the Kali VM) go in the `Kali/` subfolder (created Oct 20, 2026; **moved under `Security_Capture/` on Nov 7, 2026**; not in use until the attack-scenario session). A report there refers to its pcap here by file name.
