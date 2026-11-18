# Scenario capture plan

For each scenario in `Piranga_Workflow_v2.md` Section 11: what to record, what
the pcap should show, and the filter that proves it. Register names and
addresses come from the system docs (`BSH_Systems/`, `FH_Systems/`) and
`PLC_Logic/<System>/Register_Map.md`; check them there before a run rather than
trusting the examples below. Setup, placement and general filters:
`Capture_Setup.md`.

Every run: baseline (30-60 s of normal polling) -> action -> a few seconds
after -> stop. Log with `Capture_Log_Template.md`.

**Why these scenarios — real-world background:** each scenario below is a bench-scale
version of a documented ICS incident. The motivation, sources (ESET/Dragos/CISA/Mandiant)
and the incident-to-scenario mapping are in `Real_World_Incidents.md`, for the Deliverable
6/10 report. The MITRE ATT&CK for ICS mapping (tactic -> technique ID -> mitigation M-code,
per scenario) is in `ATTACK_ICS_Mapping.md`.

| # | Scenario | Capture on | What the pcap should show | Filter |
|---|---|---|---|---|
| 1 | Normal operation | Both captures if possible | The baseline: steady Modbus reads on port 502 from each container to its ESP32 (and SCADA to each container once it exists); HEARTBEAT incrementing; no writes except those the PLC program makes | `mbtcp` |
| 2 | Authorized door access | PLC side | A badge is a physical event, not a network one. The pcap shows its *effect*: `ACCESS_GRANTED` (DI2) going to 1 for about 2 s in the polled response, then the gate position changing. No Modbus write from a foreign host | `ip.addr == 192.168.50.13 && mbtcp` |
| 3 | Unauthorized remote door-open (Modbus injection) — *revised Nov 8, 2026, replaces RFID clone* | Attacker + PLC side | An FC 5 **Write Single Coil** to Coil 0 (`DOOR_OPEN_CMD`) from the attacker's IP with **no preceding `ACCESS_GRANTED` pulse** — contrast Scenario 2's authorized-access baseline. The gate opens with no card. `DOOR_POSITION` is unreliable (reads true open and closed, STATUS.md), so the evidence is the write packet + the physical gate, not the position bit | `modbus.func_code == 5 && ip.addr == 192.168.50.113` (or `.13` direct) |
| 4 | False fire alarm injection | Attacker + PLC side | A Modbus **Write Single Coil (FC 5)** to Coil 0 (`ALARM_ACTIVATE`) from the attacker's IP, no matching `MANUAL_PULL_TRIGGERED` input in the surrounding polls, then the alarm outputs active. Compare the write's source IP with the legitimate Modbus clients | `modbus.func_code == 5 && ip.addr == 192.168.50.112` (or `.12` if aimed directly at the ESP32) |
| 5 | Elevator lockout (BSH) | Attacker + PLC side | A write that sets `ELEVATOR_DISABLE` with no accompanying fault bit, from a non-PLC source. **Persistent-lockout variant (added Nov 7, 2026, Industroyer-inspired — see `Real_World_Incidents.md` §3):** the same write repeated on a short timer, so each SCADA/operator attempt to clear the disable is immediately re-asserted; the pcap tell is a **periodic** FC 5 write to the same coil from the attacker IP overriding the PLC/SCADA clear | `modbus.func_code == 5 && ip.addr == 192.168.50.111` |
| 6 | Building-wide power cut (FC 15 multi-coil) — *repurposed Nov 15, 2026 (was the dropped Backup power ATS)* | Attacker + PLC side | A single **FC 15 Write Multiple Coils** frame from a foreign IP setting the three PowerBox rail coils (0–2) = 0 — all BSH nodes lose power **together**, heartbeats freeze at once. Distinct from #7 (one rail, FC 5). `restore` powers them back | `modbus.func_code == 15 && (ip.addr == 192.168.50.114 || ip.addr == 192.168.50.124)` |
| 7 | Power box manipulation | Attacker + PLC side | A write flipping the lights or fan coil from an unexpected IP, with no schedule or PLC logic behind it | `modbus.func_code == 5 && (ip.addr == 192.168.50.114 || ip.addr == 192.168.50.124)` |
| 8 | Cross-building cascade | Mirror or both sides | A Door event in one building, then SCADA writes to both Fire Alarm nodes. Needs the SCADA layer, which does not exist yet (Ignition license confirmed Oct 19, 2026; gateway not installed yet); plan the capture for when it does. The pcap order and timing of the writes is the evidence | `modbus.func_code == 5 || modbus.func_code == 15` |
| 9 | Node isolation / resilience | Mirror or PLC side | One container stops: the TCP session to it ends (FIN or RST), polls to that IP go unanswered or are reset, while the other 7 keep answering. Time between the stop and SCADA flagging it can be read directly | `tcp.flags.reset == 1 || tcp.analysis.retransmission` and `ip.addr == <stopped container>` |
| 10 | Red / Blue exercise | Mirror (preferred) | Contains scenarios 3-5, 7 and 11 in one session; the Blue Team can use the capture afterwards to check what they missed. Keep a single running timeline in the log | all of the above |
| 11 | HVAC spoofing / setpoint tampering (FH) | Attacker + PLC side | A write to `TARGET_TEMP_C` (HR0) from a foreign IP (FC 6 or 16), or the fabricated `TEMP_C` value, followed by `COMPRESSOR_RUNNING` not matching the temperature trend. Modbus input registers cannot be written with a standard FC; confirm in `FH_Systems/` how the spoof is delivered (holding-register write, or a PLC-side value) and capture that path | `modbus.func_code == 6 || modbus.func_code == 16` with `ip.addr == 192.168.50.121` |
| 12 | PLC logic injection (OpenPLC web UI) — *added Nov 4, 2026; new, not in Piranga §11* | Attacker + PLC side | **Not a Modbus attack — the engineering channel, not the fieldbus.** A cleartext HTTP session on a container's port 8080 from the attacker IP: a login (the kept default OpenPLC credentials, readable with Follow HTTP Stream) and a program **upload + compile/reload**. On port 502 the normal poll **stops for a few seconds while the runtime reloads**, then the node runs the injected logic (e.g. a rung forcing `ALARM_ACTIVATE` on, or dropping a safety interlock). The Stuxnet analogue: reprogram the controller through its engineering interface. **Backed by a named CVE — `CVE-2026-88020`** (reflected XSS in the OpenPLC **v3** web UI; CISA ICSA-26-265-09): a **credential-less** second path — a crafted `:8080` URL reflects script that hijacks the operator's session cookie and issues the same state-changing requests. See the detail section below | `tcp.port == 8080 && ip.addr == 192.168.50.112` for the management session (use **Follow HTTP Stream** for the cleartext login and the upload, and to spot the XSS payload / a session cookie replayed from the attacker IP); `mbtcp && ip.addr == 192.168.50.112` for the reload gap and the changed behaviour |
| 13 | ESP32 bricking (spare board) — *extra, final, destructive; added Nov 7, 2026* | Bench (local flash) | **Little network evidence — not a Modbus attack.** Bricking is via the device's USB/flash interface. On the subnet the only sign is the spare node dropping off (heartbeat gone, same as #9); recovery is a reflash. Procedure: `Kali/Scenario13_ESP32_Bricking_Runbook.md` | n/a on Modbus; `ip.addr == <spare node>` just shows it going silent |
| 14 | IRONGATE-style combined MITM (false view) — *added Nov 8, 2026; **gated** on the Nov 9 Docker/L2 network check* | **Two points** (ESP32-side + SCADA-side) | **A man-in-the-middle, not a one-shot write.** ARP takeover first (gratuitous ARP / changed MAC for the target IPs), then **two Modbus streams that disagree**: what the ESP32 really sends vs what the container/Ignition receives. Position A (ESP32 `.12` ⟷ container `.112`) feeds the PLC false field data; position B (container `.112` ⟷ Ignition `.30`) forges the operator view and swallows operator commands. The evidence is the divergence between the two capture points. Restore = stop the spoof, ARP tables re-converge. Runbook TBD after Nov 9 | `arp` for the takeover **plus** `mbtcp && ip.addr == 192.168.50.112` at both capture points |

## Scenario 12 detail — PLC logic injection (added Nov 4, 2026)

**New scenario, not in `Piranga_Workflow_v2.md` Section 11.** Added here Nov 4, 2026
after Luis asked whether the OpenPLC ladder logic can be manipulated. It can — but
through the OpenPLC **web management interface on port 8080**, not through Modbus. This
is the highest-impact scenario in the set: full control-logic compromise, the direct
analogue of how Stuxnet reprogrammed PLCs.

**Why Modbus can't do it.** Standard Modbus TCP has no function to change a program;
over port 502 an attacker only reads/writes the IO points the program already exposes
(scenarios 4, 5, 7, 11). Changing the *logic* needs the engineering channel — OpenPLC's
web UI. (`Piranga_Workflow_v2.md:305`: "Modbus has no authentication" is about data,
not logic.)

**The vector (from Kali `.40`):**
1. Reach a container's `http://<container>:8080` (every instance exposes 8080 —
   `Piranga_Workflow_v2.md:114-131`).
2. Log in with the **kept default OpenPLC credentials** (default login kept per the
   "OpenPLC web UI" note below and `Next_Steps.md`, Oct 13). Because it is plain HTTP,
   the credentials can also be **sniffed** off the wire, so even a changed password
   would not save it.
3. Upload a malicious program, **Compile**, and the runtime reloads running it.
4. The node now runs attacker logic until the original is restored.

**Named CVE: `CVE-2026-88020` (added Nov 7, 2026).** OpenPLC Runtime **v3** — the exact
runtime this project runs on all eight containers (`PLC_Logic/CLAUDE.md`) — has a
**reflected cross-site scripting** flaw (CWE-79): the web interface **routes programs by a
query-string parameter without encoding it**. CISA advisory **ICSA-26-265-09** (published
Sep 22, 2026); CVSS 3.1 **6.1** / 4.0 5.3 (medium). Per CISA, exploitation lets an attacker
**hijack the operator's session cookie and send state-changing requests as that operator** —
i.e. reach the stop/upload/restart capability above **without needing the default password**.
This gives Scenario 12 a second, credential-less vector and a citable vulnerability for the
report: a crafted `:8080` URL carrying the XSS payload, then operator-authenticated requests
replayed from the attacker IP. A second OpenPLC v3 CVE, **`CVE-2026-14480`** (authenticated
arbitrary file write → code execution, CWE-73, CVSS 9.9, CISA ICSA-26-190-01), gives the same
outcome via the default-credential path; see `References.md`.

**The v3/v4 point (state it in the report).** The vendor fix is "upgrade to OpenPLC v4"; v3
is end-of-life and unpatched. This project **deliberately stays on v3** through the Dec 11
demo (`PLC_Logic/CLAUDE.md`), so the lab **knowingly carries CVE-2026-88020** — an accepted
condition of the testbed for demonstration, not an oversight. Not in CISA's KEV catalog and
no confirmed public exploit as of Nov 7, 2026.

**Evidence to capture:** the cleartext login and the program-upload POST on port 8080
(Follow HTTP Stream), plus — on port 502 — the brief poll outage during the reload and
the node's changed behaviour afterwards. A good report pair: one shot of the HTTP stream
showing credentials/upload with no encryption, one of the Modbus polling before/after.

**Required restore step (not optional).** The upload **overwrites the running logic** on
that container and reloads it (Modbus server down for a few seconds — an availability
side effect worth noting). The known-good program is kept at
`PLC_Logic/<System>/Program.st`; re-uploading and reloading it is a mandatory final step
of this run. Pick **BSH-FireAlarm (container `.112`)** as the worked example, consistent
with the learning slice; the attack applies to all eight containers.

**Accuracy / not verified.** There is **no OpenPLC manual in `References/`** (only Ignition
and ESP-IDF are vendored), so the Programs → upload → Compile → reload flow above is
well-known OpenPLC v3 behaviour, **not cited from a project doc**. Confirm the exact page
flow and the HTTP request shape against the live `:8080` during the run; do not script it
blind. Closed lab only; time-box it and restore.

## Also worth capturing

- **Reconnaissance** that precedes any attack: a ping sweep or a port scan of
  `192.168.50.0/24`. Use the broad capture filter from `Capture_Setup.md`. Shows
  the project's decision to keep real port 502 open on every container
  (`Piranga_Workflow_v2.md` Section 6): one default Modbus scan finds all eight.
- **The unauthenticated nature of Modbus TCP**: open any write packet and show
  there is no credential field in the frame (MBAP header: transaction ID,
  protocol ID, length, unit ID, then function code and data). This is a good
  screenshot for the report.
- **OpenPLC web UI traffic (port 8080)**: the default login is kept
  (`Next_Steps.md`, Oct 13), so credentials cross the wire in clear HTTP. Capture
  a login once; it supports a "cleartext management interface" finding.

## For the report (Deliverables 6 and 10)

For each scenario keep: one screenshot of the Wireshark packet list filtered to
the attack, one expanded Modbus packet showing function code, reference number
and value, and the matching row of the run log. Name screenshots like the pcap.
