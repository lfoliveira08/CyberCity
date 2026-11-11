# Threat model and scope (Deliverables 6 and 10)

Created Nov 7, 2026. The framing doc for the attack work: what adversary we assume, what we
demonstrate, what we deliberately do **not** do, and why. Companions: `Scenarios.md` (what we do),
`Real_World_Incidents.md` (the real attacks), `ATTACK_ICS_Mapping.md` (ATT&CK/mitigations),
`References.md` (citations/CVEs).

## Authorization

Authorized testing on **Luis's own isolated lab** (`192.168.50.0/24`), a university capstone;
the professor is aware of the full project. Destructive tests use **spare/expendable hardware**
only. Nothing here is run outside the closed lab.

## Threat model — the assumption we start from

We assume the adversary is **already present on the OT network** — via an insider, a compromised
vendor laptop, or a breached IT/OT boundary. The project then asks the capstone's real question:

> **Once an attacker can reach the control network, what can they do to the physical process, and
> how do we detect and mitigate it?**

This is a standard, defensible scope: it isolates the **OT/ICS problem** (unauthenticated field
protocols) from the enterprise-IT intrusion problem, which is a different domain.

## Attack tier (what "level" these are)

The scenarios are modelled on **nation-state / APT** incidents (Stuxnet, Industroyer, TRITON,
PIPEDREAM — see `Real_World_Incidents.md`), the same tier the IRONCLAD reference project was framed
around. But the **execution is low-skill**: commodity Modbus writes, an nmap scan, a default-login
web upload — no zero-days, no custom malware, no evasion. The lesson to state plainly:

> **Nation-state-level *impact* is achievable with script-kiddie-level *effort***, because OT
> protocols (Modbus here; IEC-104/61850/OPC in the grid cases) trust whoever is on the wire.

## In scope — the tactics we demonstrate

The OT-impact half of MITRE ATT&CK for ICS:

| Tactic | ID | In this lab |
|---|---|---|
| Discovery | TA0102 | recon: subnet/Modbus scan, point & tag identification |
| Execution | TA0104 | Scenario 12 (upload/run attacker logic) |
| Impair Process Control | TA0106 | #4, #5, #7, #11 — the Modbus writes |
| Inhibit Response Function | TA0107 | #9 DoS; defeating a safety response |
| Impact | TA0105 | the consequence: Loss of Safety / Control / Availability / View |

## Out of scope — and why (per phase)

These are the **intrusion chain** (how a real crew gets to the wire) plus the **stealth,
intelligence and destruction** wrapper. "Out of scope" does not mean "impossible" or "not
understood" — several are feasible against this architecture; we choose not to perform them.

| Phase (ID) | What it is | Why we skip it |
|---|---|---|
| Initial Access (TA0108) | first foothold: phishing, malicious doc, stolen VPN, bad USB, vendor compromise | IT/human attack, needs phishing/malware we won't do; our threat model assumes the attacker is already in |
| Persistence (TA0110) | survive reboots/remediation (backdoors, implanted firmware) | needs planted malware; our demos are one-shot + restored. *Partial overlap:* an uploaded malicious `Program.st` (Scenario 12) persists until restored — logic-level persistence, no implant |
| Privilege Escalation (TA0111) | gain higher rights on a host | our Modbus writes need **no** privileges; OpenPLC's web UI uses default creds — nothing to escalate. The absence *is* the finding |
| Evasion (TA0103) | hide from AV/EDR, clear logs, anti-forensics | we want the opposite — full Wireshark visibility and captured evidence. (ICS often has little monitoring to evade — itself a finding) |
| Lateral Movement (TA0109) | pivot host-to-host, cross the IT/OT boundary | the hard real-world barrier; our Kali sits on the OT subnet **by design** — that placement *is* the threat-model assumption |
| Collection (TA0100) | gather process data, HMI/engineering files, maps | we already know our own process — nothing to collect; exfil would need C2 |
| Command & Control (TA0101) | hidden remote channel (Tor/HTTPS) to drive the implant and exfiltrate | our attacker is a **local, hand-operated** box on an isolated LAN; adding C2 means internet egress we deliberately don't want |

**Credential handling** is not an ICS-ATT&CK tactic of its own, but note: we do **not** actively
steal credentials. We only *observe* that the OpenPLC `:8080` login crosses in **cleartext**
(sniffable) — a finding surfaced passively, not an active credential-theft step.

### Destructive techniques — understood, bounded

Destructive "cleanup" (wipers, bricking) is the real-world final stage that blocks recovery
(Ukraine 2015's KillDisk + bricked converters). It is **feasible against this architecture** — a
node, container, VM, Gateway, or a PLC program could be wiped. We bound it as follows:

- **Non-destructive scenarios** already prove the thesis; destruction adds risk, not evidence.
- **Scenario 13 (extra, final, destructive)** — ESP32 bricking on a **spare/expendable board**,
  at the professor's request — is centered on the **recoverable soft-brick**: erasing/corrupting
  flash stops the node booting, and re-flashing recovers it (standard dev tooling). This
  demonstrates the destructive attack **and** the recovery/backup mitigation. A truly *permanent*
  brick needs irreversible **eFuse** changes (specifics to verify in
  `References/ESP-IDF/05-security-guides.md`, `CLAUDE.md` §4b); that is **documented as feasible but
  not performed** in the written procedure — it only wastes a board and proves nothing the
  soft-brick does not.
- **Recoverability is the mitigation**, and the project already practices it: Gateway backups,
  `Program.st` copies, VM snapshots.

## One-paragraph statement for the report

> This project scopes to the OT-impact phases of an ICS attack. It assumes an adversary already
> present on the OT network (insider, compromised vendor device, or breached IT/OT boundary) and
> demonstrates what such an adversary can do to the physical process over unauthenticated Modbus,
> plus the detection and mitigation. The intrusion, stealth, intelligence and destruction phases
> are understood and, in several cases, feasible against this architecture, but are excluded —
> some as out-of-domain (enterprise IT), some as destructive. The techniques shown are
> nation-state-*class* in method but low-skill in execution, underscoring that the barrier to
> OT impact is the protocol's lack of authentication, not attacker sophistication.

## Caveats

- Tactic IDs are from MITRE ATT&CK for ICS (pulled Nov 7, 2026; see `References.md`); confirm
  against `attack.mitre.org` for the final report.
- ESP32 hardware specifics (flash/eFuse behavior) are **to verify** in `References/ESP-IDF`
  (`CLAUDE.md` §4b) before asserting them as fact.
