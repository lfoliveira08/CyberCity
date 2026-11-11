# References — frameworks, advisories and CVEs (Deliverables 6 and 10)

Created Nov 7, 2026. The citation list for the attack-scenario deliverables: the MITRE
ATT&CK for ICS pages, the government/vendor advisories, the primary incident reports, and
**every CVE used, tied to its scenario**. Companions: `Scenarios.md` (the scenarios),
`Real_World_Incidents.md` (the attacks), `ATTACK_ICS_Mapping.md` (the ATT&CK table).

**Source labelling.** Links verified from search on Nov 7, 2026. CVE and incident details
come from **secondary sources and advisory summaries** — the primary CISA/ESET/Dragos/Mandiant
documents were **not opened** in full this session. Confirm each against the primary document
before the final report. (`CLAUDE.md` §7: verified / reported / assumed.)

## 1. MITRE ATT&CK for ICS (the framework — links Luis provided)

- Techniques: https://attack.mitre.org/techniques/ics/
- Tactics: https://attack.mitre.org/tactics/ics/
- Matrix: https://attack.mitre.org/matrices/ics/
- Mitigations: https://attack.mitre.org/mitigations/ics/

The per-scenario tactic/technique/mitigation mapping built from these is in `ATTACK_ICS_Mapping.md`.
The matrix page also drives the ATT&CK Navigator (for a highlighted coverage layer).

## 2. CVE register — every CVE we use, and the scenario it serves

| CVE | Affects | Type / CVSS | Advisory | Our scenario | Status for CyberCity |
|---|---|---|---|---|---|
| **CVE-2026-88020** | OpenPLC Runtime **v3** web UI | Reflected XSS, CWE-79, CVSS 3.1 **6.1** | CISA **ICSA-26-265-09** (Sep 22, 2026) | **#12** — crafted `:8080` URL hijacks the operator session (credential-less path) | **In use now** — our 8 containers run v3 |
| **CVE-2026-14480** | OpenPLC Runtime **v3** web UI program-upload | Authenticated arbitrary **file write → code execution**, CWE-73, CVSS 3.1 **9.9 (Critical)** | CISA **ICSA-26-190-01** (Jul 9, 2026) | **#12** — with the kept **default creds**, write files into the runtime core → RCE on the container (even stronger than the XSS) | **In use now** — our v3 + default login satisfy the "authenticated" precondition |
| CVE-2015-5374 | Siemens SIPROTEC protection relays (UDP 50000) | Denial of Service | (Industroyer; ESET-reported, Dragos-disputed) | **#9** — the real-world reference for the DoS/relay-disable idea | **Reference only** — not our hardware; our DoS exploits stack/library limits |
| CVE-2020-6111 | Allen-Bradley MicroLogix 1100, fw 10–16 | DoS → 8H hard fault | CISA **ICSA-21-047-02** | real-PLC phase (post Nov 13) — a DoS against the real PLC | **Later phase** — IRONCLAD's CVE; deferred per the professor |

**Scenarios with no CVE.** #4, #5, #7, #8, #11 exploit **Modbus's lack of authentication by
design**, not a software vulnerability — there is no CVE to cite; the "vulnerability" is the
protocol (and OpenPLC/ESP32 keeping port 502 open with no auth). Say this explicitly in the report.

## 2b. Background CVEs on comparable commercial products (from the partner survey — NOT CyberCity's stack)

These are real CVEs on **other vendors'** ICS/Modbus gear that show the **same weakness classes**
CyberCity demonstrates — useful report evidence that "unauthenticated Modbus," "cleartext,"
"default credentials," and "connection-exhaustion DoS" are recognized, filed vulnerabilities. None
are in OpenPLC / ESP32 / Ignition. Details from web search Nov 7, 2026 (secondary sources + NVD/CISA
summaries; verify before quoting).

| Product (survey ref) | CVE(s) | Weakness → matches our | Advisory |
|---|---|---|---|
| **Delta DVP-12SE** PLC (Modbus #1) | `CVE-2026-12819` (CWE-306 missing auth, Modbus read/write), `CVE-2026-12818` (CWE-770 resource-exhaustion DoS), `CVE-2025-62578` (CWE-319 cleartext Modbus/TCP) | #4/#5/#7/#11 no-auth writes; #9 DoS; cleartext finding | Delta-PCSA-2026-00011 |
| **Unitronics** Vision/Samba (ladder #9) | `CVE-2023-6448` (default password "1111", CVSS 9.8, in CISA KEV; CyberAv3ngers / Aliquippa) | #12 default-credential path | CISA ICSA-23-348-15; AA23-335A |
| **Schneider Modicon** M340/M580 (Modbus #5) | `CVE-2021-22779` (ModiPwn UMAS auth bypass) | #12 engineering-access theme (vendor-specific) | Armis / SecurityWeek |
| **Danfoss AK-SM 800A** (France #22) | `CVE-2025-41450` (auth bypass), `CVE-2025-41451` (post-auth command injection → RCE), `CVE-2025-41452` (web-server config) | #11 exposed-management + setpoint tamper | Claroty Team82; CERT-FR |
| **Copeland/Dixell XWEB** (France #23) | XWEB-500: `CVE-2021-45420` (unauth file write, 9.8), `CVE-2021-45421`; XWEB Pro (2026): `CVE-2026-25085` (auth bypass), `CVE-2026-21718` (predictable admin creds → root), +~21 more | #11; #12 (file-write→RCE parallels CVE-2026-14480) | CISA ICSA-26-057-10 |
| **MS Exchange** (SCADA #19 ShadowPad) | `CVE-2021-26855` (ProxyLogon) | IT entry vector, not OT | — |

**Important:** for the **French refrigeration incidents (#22–23)** the attack-to-CVE link is
**unconfirmed** — CERT-FR/Danfoss/Copeland have not named the exploited CVE, and many affected models
are end-of-life. Cite these as *background*, not as the proven mechanism.

## 3. Government / vendor advisories

- **CISA ICSA-26-265-09** — OpenPLC Runtime v3 (CVE-2026-88020, XSS): https://www.cisa.gov/news-events/ics-advisories/icsa-26-265-09
- **CISA ICSA-26-190-01** — OpenPLC v3 (CVE-2026-14480, file-write/RCE): https://www.cisa.gov/news-events/ics-advisories/icsa-26-190-01
- **CISA TA17-163A** — CrashOverride / Industroyer: https://www.cisa.gov/news-events/alerts/2017/06/12/crashoverride-malware
- **CISA ICSA-21-047-02** — Allen-Bradley MicroLogix (CVE-2020-6111). (From the IRONCLAD `ics-railroad-cybersecurity-lab` skill; URL not pulled this session.)
- **CISA AA21-042A** — "Compromise of U.S. Water Treatment Facility" (Oldsmar, CISA/FBI/EPA). (ID verified; URL not pulled.)
- **CISA AA22-103A** — "APT Cyber Tools Targeting ICS/SCADA Devices" (PIPEDREAM/INCONTROLLER; CISA/NSA/FBI/DOE). (ID verified; URL not pulled.)
- **DHS/ICS-CERT IR-ALERT-H-16-056-01** — "Cyber-Attack Against Ukrainian Critical Infrastructure" (Feb 25, 2016). (ID verified; URL not pulled.)

## 4. Primary incident reports (per attack)

- **Stuxnet** — Symantec, *W32.Stuxnet Dossier* (Falliere, Murchú, Chien; v1.4, 2011); Langner, *To Kill a Centrifuge* (2013).
- **Ukraine 2015** — E-ISAC / SANS, *Analysis of the Cyber Attack on the Ukrainian Power Grid* (Lee, Assante, Conway; Mar 18, 2016), e.g. `icscsi.org` mirror; DHS IR-ALERT-H-16-056-01.
- **Industroyer / CrashOverride (2016), Industroyer2 (2022)** — ESET, *Win32/Industroyer* (Jun 2017); Dragos, *CRASHOVERRIDE: Analysis of the Threat to Electric Grid Operations* (Jun 13, 2017); CISA TA17-163A. (ESET/Dragos disagree on the SIPROTEC DoS module and 2016 attribution.)
- **TRITON / TRISIS / HatMan (2017)** — Mandiant/FireEye TRITON analysis; Dragos TRISIS; CISA MAR *HatMan* + ICS Advisory *Schneider Electric Triconex Tricon*; FBI PIN (Mar 2022).
- **Oldsmar (2021)** — CISA/FBI/EPA Joint Advisory AA21-042A.
- **PIPEDREAM / INCONTROLLER (2022)** — CISA AA22-103A; Mandiant *INCONTROLLER*; Dragos *PIPEDREAM / CHERNOVITE*.
- **FrostyGoop (2024, Lviv district heating)** — Dragos incident assessment; Palo Alto Unit 42 technical analysis. No single CVE named.
- **French supermarket refrigeration (2026)** — CERT-FR alerts (Danfoss AK-SM/AK-SC; later Copeland/Dixell XWEB); Reuters + AP reporting; Claroty Team82 (Danfoss). Attack-to-CVE link unconfirmed.

## 5. Caveats

- URLs marked "not pulled this session" are cited by advisory ID/title only — look them up on
  `cisa.gov` before the final report rather than trusting a guessed URL.
- All incident/CVE technical details here are **reported** from secondary sources; promote to
  **verified** only after reading the primary document.
