# Real-world ICS/SCADA incidents — background for Deliverables 6 and 10

Created Nov 7, 2026. **This is background, not a capture procedure.** It exists so the
attack-scenario deliverables have a cited, real-world "why" behind each lab scenario:
every scenario in `Scenario_Capture_Plan.md` is a bench-scale version of something that
happened in the field. Use these for the motivation/threat-model section of the report
and cite the sources listed per incident. Each incident closes with **How we use it on our system** — the concrete scenario, node and action that reproduces it here (see `Scenarios.md`).

**The one throughline.** Every incident below worked for the same reason the lab's Modbus
write scenarios work: **industrial protocols trust whoever is on the wire.** Modbus,
IEC 60870-5-101/104, IEC 61850, OPC — none authenticate the sender or check message
integrity by design. The attackers did not "break" the protocol; they *spoke* it. The
testbed reproduces that root cause at the bench.

## How to read the source labels

Per the project's verified/reported/assumed rule (`CLAUDE.md` §7): the incident facts
here come from **secondary summaries of primary reports** found by web search on Nov 7,
2026 — the original ESET/Dragos/Mandiant PDFs and CISA pages were **not opened** in that
session. Treat specifics as **reported**, not independently verified, and confirm against
the primary document before quoting a number in the final report. Where two primary
reports disagree, that is called out.

## Summary — incident → what it teaches → lab scenario

| Incident (year) | Core mechanism | Lab scenario it motivates |
|---|---|---|
| Stuxnet (2010) | Reprogram the PLC; feed the HMI fake "normal" values | #12 logic injection; #11 false view |
| Ukraine grid, BlackEnergy (2015) | Abuse the *real* operator HMIs to open breakers | #7 power; #5 lockout |
| Industroyer / CrashOverride (2016) | Autonomous malware that **speaks grid protocols** to operate breakers, incl. looped re-open | #5 lockout (persistent variant); #4/#7 command injection; #9 DoS |
| TRITON / TRISIS / HatMan (2017) | Attack the **safety** controller itself | #4 fire alarm (a safety function) |
| Oldsmar water (2021) | Remote HMI access; push a setpoint to a dangerous value | #11 setpoint tamper (near 1:1) |
| PIPEDREAM / INCONTROLLER (2022) | A modular **toolkit** incl. a Modbus module to scan + command PLCs | The whole testbed |
| FrostyGoop (2024) | **Modbus** write to a heating controller → loss of heat for 600+ buildings | **#11 HVAC** — closest-protocol real incident |
| French supermarket refrigeration (2026) | Remote setpoint change on internet-exposed fridge controllers → spoiled stock | **#11 HVAC** — current, same mechanism |

---

## 1. Stuxnet (2010, Natanz, Iran)

**What happened (reported).** A worm targeted Siemens S7-300/400 PLCs (via Step7/WinCC
engineering software) controlling uranium-enrichment centrifuges. It altered centrifuge
rotor speeds to damage them while **replaying previously recorded normal process values
to the monitoring stations**, so operators saw nothing wrong.

**What it teaches.** Two things the lab reproduces: (a) the real damage is done by changing
the *control logic*, reached through the *engineering channel*, not the fieldbus; (b) an
attacker can make the HMI lie. This is the direct motivation for **Scenario 12 (PLC logic
injection)** — OpenPLC's web UI is our "engineering channel" — and for the "SCADA shows
green while reality is wrong" idea behind **Scenario 11**.

**Lab limit.** Forcing a false reading is easy here; *replaying* recorded normal values to
hide the change needs a man-in-the-middle and is out of scope.

**How we use it on our system.** **Scenario 12** is our Stuxnet: from Kali `.40` we reprogram a
container's ladder through the OpenPLC `:8080` web UI (worked example BSH-FireAlarm `.112`) —
upload a malicious `Program.st`, reload, and the node runs attacker logic; we capture the `:8080`
session and the changed `:502` behaviour, then restore the known-good `PLC_Logic/<System>/Program.st`.
The "HMI shows normal while reality differs" half we approximate with **Scenario 11/4** (force a
false reading or coil and watch Ignition display it).

**Sources.** Symantec, *W32.Stuxnet Dossier* (Falliere, Murchú, Chien; v1.4, Feb 2011);
Ralph Langner, *To Kill a Centrifuge* (2013). (Canonical titles; URLs not pulled this
session.)

## 2. Ukraine power grid — BlackEnergy (Dec 23, 2015)

**What happened (reported).** Spear-phishing delivered BlackEnergy3; attackers stole
operator credentials and remote-accessed distribution SCADA, then used the **operators'
own HMIs** to open breakers at substations. ~225,000 customers lost power. Follow-on:
the KillDisk wiper, firmware-bricking of serial-to-Ethernet converters, and a telephone
denial-of-service on the call center to hide the outage. Hands-on-keyboard, not autonomous.

**What it teaches.** The control path itself was the weapon — legitimate commands from a
stolen seat. Maps to **Scenario 7 (power box)** and **Scenario 5 (lockout)**; the recovery
sabotage (wiper, bricked converters, phone DoS) motivates **Scenario 9 (resilience)** and
the project's backup discipline.

**How we use it on our system.** **Scenario 7** is the bench version of opening a breaker: from
Kali `.40` an FC 5 write cuts a power-rail coil on a PowerBox container (`.114`/`.124`), dropping
power to a node — the control-path effect without the credential theft. **Scenario 5** (lockout)
is the same idea on the elevator. We reproduce the *effect on the process*, not the IT-side
intrusion.

**Sources.** DHS/ICS-CERT Alert **IR-ALERT-H-16-056-01**, "Cyber-Attack Against Ukrainian
Critical Infrastructure" (Feb 25, 2016, now on the CISA site); E-ISAC / SANS, *Analysis of
the Cyber Attack on the Ukrainian Power Grid* (Lee, Assante, Conway; Mar 18, 2016).
Customer count varies by source (~225,000–230,000). Attribution (Sandworm) is asserted by
some sources; the E-ISAC report avoids attribution.

## 3. Industroyer / CrashOverride (2016) and Industroyer2 (2022)

**What happened (reported).** In Dec 2016 a ~1-hour outage hit part of Kyiv. Unlike 2015,
this was **autonomous malware that understood the grid protocols itself** — the first
malware framework purpose-built to attack an electric grid. Named *Win32/Industroyer* by
ESET and *CrashOverride* by Dragos. It was modular:

- A backdoor (C2 over Tor) and a launcher.
- **Four protocol payload modules**, each speaking a real ICS protocol: **IEC 60870-5-101**
  (serial), **IEC 60870-5-104** (TCP, port 2404), **IEC 61850** (substation automation /
  MMS), and **OPC DA** — used to enumerate devices and operate circuit breakers. It could
  **toggle breaker states continuously** (reopen them in a loop to fight operators restoring
  power).
- A **data wiper** to render systems unbootable and hamper recovery.
- **(ESET-reported, Dragos-disputed)** a denial-of-service module against Siemens SIPROTEC
  protection relays exploiting **CVE-2015-5374** (crafted packets to UDP 50000, freezing the
  relay until a manual reboot). **Dragos stated it could not confirm this module** — so treat
  the SIPROTEC DoS as reported by ESET, not established.

**Industroyer2 (April 2022, Ukraine).** A leaner variant during the invasion: a single
**hardcoded IEC-104 module** aimed at one substation, with config baked into the binary.
Detected and blocked (CERT-UA / ESET / Microsoft).

**What it teaches — and why it matters here even though it is not Modbus.** The protocols
differ, but the lesson is identical: Industroyer did not exploit a bug to move the breakers,
it **spoke the unauthenticated protocol**, exactly as our Modbus write scenarios do. Four
concepts to fold into the deliverables:

1. **Protocol-aware command injection** → the Modbus equivalents are **#4 / #5 / #7 / #11**.
2. **Looped / persistent state changes** (reopening breakers to beat the operator) → the
   **persistent-lockout wrinkle added to Scenario 5**: re-assert the disable on a timer so
   each SCADA attempt to clear it is immediately overridden.
3. **Attack the protective layer and deny recovery** (the SIPROTEC DoS, if real; plus the
   wiper) → combine **Scenario 9 (DoS)** with another attack: blind SCADA or knock out the
   pull-station cascade *while* injecting.
4. **Autonomous, timed execution** (it ran on a schedule) → an attack script that fires on a
   timer/trigger rather than an interactive write — more realistic and easy to demonstrate.

**Lab limit.** The wiper is destructive and not replicable (we do not wipe our own VMs); name
it in the report as *why availability and backups matter*.

**How we use it on our system.** **Scenario 5's persistent variant** is the breaker-reopen loop:
we re-assert `ELEVATOR_DISABLE` (Coil 3 on `.111`) on a ~1 s timer so the operator's clear won't
stick, driven from a timed script rather than by hand (the autonomy angle). The recovery-denial
half maps to **Scenario 9** (DoS a node so SCADA goes blind); we name the wiper in the report but
do not run it.

**Sources.** CISA Alert **TA17-163A**, "CrashOverride Malware" (Jun 12, 2017; updated Jul 20,
2021); ESET, *Win32/Industroyer: A new threat for industrial control systems* (Jun 2017);
Dragos, *CRASHOVERRIDE: Analysis of the Threat to Electric Grid Operations* (Jun 13, 2017).
Note the ESET/Dragos disagreement on the SIPROTEC module and on how conclusive the 2016
attribution is.

## 4. TRITON / TRISIS / HatMan (2017, Middle East petrochemical)

**What happened (reported).** Malware targeted **Schneider Electric Triconex safety
instrumented system (SIS) controllers** — the last-line safety layer that trips a plant to
a safe state. It modified the controller's in-memory firmware to add code. In 2017 the SIS
detected an anomaly (caused by a bug in the malware) and **automatically tripped the plant to
a safe shutdown**, which is how the attack was discovered. Mandiant named it TRITON, Dragos
TRISIS, and DHS/CISA HatMan. Widely reported as a petrochemical facility in Saudi Arabia.

**What it teaches.** Attacking the *safety* function is the most dangerous class: the goal was
to disable the safety trip so a separate attack could cause physical harm. Your **fire alarm
(Scenario 4)** is a safety function — the scenario has two directions: *falsely trigger* it
(inject the alarm) and, the TRITON direction, *suppress/defeat* it so it will not fire when it
should. The second direction depends on the ladder logic and is worth studying even if not run.

**How we use it on our system.** **Scenario 4** is our safety-tamper: from Kali `.40` an FC 5
write to `ALARM_ACTIVATE` (Coil 0 on `.112`) sounds the fire alarm with no real pull — the
"manipulate the safety function" demo. The TRITON *suppress* direction (defeat the trip so it
won't fire when it should) we study on paper, pending a read of the ladder.

**Sources.** Mandiant/FireEye TRITON analysis (2017); Dragos *TRISIS* report; CISA/DHS **MAR
HatMan — Safety System Targeted Malware**, and CISA ICS Advisory *Schneider Electric Triconex
Tricon*; FBI PIN (Mar 2022). Attribution unconfirmed (state-sponsored suspected).

## 5. Oldsmar water treatment (Feb 5, 2021, Florida)

**What happened (reported).** An intruder used remote-access software (TeamViewer) to reach
the plant HMI and raised the **sodium hydroxide (lye) setpoint from ~100 ppm to ~11,100 ppm**
— about 100×. An operator watching the mouse cursor move on its own reverted it immediately;
authorities said the public was never at risk. Plant serves ~15,000 people.

**What it teaches.** This is the closest **1:1 analogue to Scenario 11 (HVAC setpoint tamper)**:
change a single setpoint to a dangerous value and let the process act on it. It also shows the
value of an operator-visible HMI (the campus map / alarm panel you built) in catching it.

**How we use it on our system.** **Scenario 11** is a near 1:1 Oldsmar: an FC 6/16 write pushes
`TARGET_TEMP_C` (holding register 0 on FH-HVAC `.121`) to an extreme value, and the
operator-visible catch is the Ignition campus map + `NodeOffline`/alarm panel you built.

**Sources.** CISA/FBI/EPA Joint Advisory **AA21-042A**, "Compromise of U.S. Water Treatment
Facility" (Feb 2021). Detail varies across secondary sources; confirm against the advisory.

## 6. PIPEDREAM / INCONTROLLER (2022)

**What happened (reported).** A modular, state-sponsored **ICS attack toolkit** discovered
before deployment — Mandiant calls it INCONTROLLER, Dragos calls it PIPEDREAM (attributed to
the group CHERNOVITE). It can scan for, command, and disrupt PLCs using native protocol
functionality (no product-specific vulnerability needed), with modules for **Modbus**, CODESYS,
OPC UA, and Omron FINS; named target PLCs include Schneider Modicon and Omron families.

**What it teaches.** This is the generic Modbus-write threat the whole testbed demonstrates,
packaged as reusable tooling: an nmap Modbus scan plus `modbus_coil_tool.py` is a
miniature of its scan-and-command capability. Good framing for "this is not just a 2010 story;
the capability is current and tool-based."

**How we use it on our system.** PIPEDREAM is our whole toolset, scaled down: an nmap Modbus
scan of `192.168.50.0/24` (recon) finds all eight `:502` servers, and `modbus_coil_tool.py`
issues the writes for **Scenarios 4 / 5 / 7**; **Scenario 12** is the PLC-command/logic-change
capability. We demonstrate the generic, tool-based Modbus-write threat on our own containers.

**Sources.** Joint Cybersecurity Advisory **AA22-103A** (CISA, NSA, FBI, DOE; Apr 2022);
Mandiant, *INCONTROLLER* report; Dragos, *PIPEDREAM / CHERNOVITE* report. Reported as not
observed deployed in the wild.

---

## 7. FrostyGoop (Jan 2024, Lviv, Ukraine — district heating)

**What happened (reported).** Malware tracked as **FrostyGoop**, together with internet-exposed
industrial devices, was assessed by Dragos to have facilitated a ~2-day interruption of municipal
**district heating** to 600+ apartment buildings in winter. The malware demonstrated **Modbus TCP
register read/write** and could send commands to the site's ENCO controllers. No specific CVE is
established as the initial entry point.

**What it teaches.** The first real incident where **Modbus** interaction with temperature control
produced a physical heating loss — i.e., exactly the protocol and effect CyberCity's HVAC scenario
models. Most grid cases use IEC-104/61850; this one is **Modbus**, making it the closest-protocol
real incident to the lab.

**How we use it on our system.** **Scenario 11** is the bench version: an FC 6/16 write to
`TARGET_TEMP_C` (HR 0 on FH-HVAC `.121`) drives the HVAC toward an attacker's setpoint. The
incident also motivates comparing **physical readings vs displayed tags** and checking **recovery**
after an unauthorized change — both already in our plan.

**Lab limit.** We have no heating-plant scale; the mechanism (networked Modbus write → temperature
effect) is identical.

**Sources.** Dragos incident assessment; Palo Alto Unit 42 technical analysis (2024). No single CVE
named by investigators.

## 8. French supermarket refrigeration attacks (Oct–Nov 2026)

**What happened (reported).** Over the nights of **Oct 11–13, 2026** and a follow-on **early-November**
wave, attackers changed refrigeration operating parameters at French supermarkets via
**internet-reachable refrigeration-management controllers**. Early reports cited ~150 Intermarché
stores (temperatures reportedly raised toward 50 °C for ~6 h); a later government estimate put it at
~700 supermarkets across chains. **CERT-FR** confirmed compromises of **Danfoss AK-SM/AK-SC** and,
later, **Copeland/Dixell XWEB** supervisory controllers.

**What it teaches.** A **current** (2026), large-scale example of the exact thing Scenario 11 models:
remote setpoint tampering on temperature control → physical spoilage and financial loss. It also
sharpens the **commanded-vs-measured** point — a malicious "150 °C" setpoint does not prove the
cabinet reached it, so independent readings matter.

**How we use it on our system.** **Scenario 11** (setpoint tamper) is the bench analogue; on the SCADA
side it motivates comparing the Ignition tag against an independent reading and doing **verified
recovery** (restore safe settings + close the exposed access, not just reboot).

**Lab limit.** Bench scale. Note: even in the real incidents, **no single CVE is confirmed as the
vector** — product CVEs (listed in `References.md`) are background, not proof.

**Sources.** CERT-FR alerts (Danfoss AK-SM/AK-SC; later Copeland/Dixell XWEB); Reuters and AP
reporting; Claroty Team82 (Danfoss CVEs); CISA ICSA-26-057-10 (Copeland XWEB). Attack-to-CVE link
unconfirmed.

## Caveats for the report

- These summaries are from **secondary sources** (news, case-study pages) read Nov 7, 2026;
  the primary CISA/ESET/Dragos/Mandiant documents were not opened. Pull the primary document
  before quoting any specific number.
- **Attribution varies** by source for most incidents; the lab/report does not need to take a
  side — the *mechanism* is the teaching point.
- The **Industroyer SIPROTEC DoS** module is ESET-reported and **Dragos-disputed** — present it
  as such.
