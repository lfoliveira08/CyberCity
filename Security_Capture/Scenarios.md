# Attack scenario catalog — full detail (Deliverables 6 and 10)

Created Nov 7, 2026. The single detailed description of every lab attack scenario: its goal,
the exact target (node, container IP, Modbus object + function code with the register-map
cite), preconditions, the attacker action, the expected physical and SCADA effect, **the
real-world attack it is modelled on**, its MITRE ATT&CK for ICS IDs, any CVE, and the
capture filter. This is the hub; the companions are:

- `Scenario_Capture_Plan.md` — the Wireshark capture procedure/filters per scenario
- `Real_World_Incidents.md` — the real attacks explained + how we use each (Deliverable background)
- `ATTACK_ICS_Mapping.md` — the ATT&CK tactic/technique/mitigation table
- `References.md` — links and CVEs

**Labels.** Addresses are **verified** in each `PLC_Logic/<System>/Register_Map.md` on the
date noted there. Physical/SCADA effects are **expected/assumed** — **nothing has been run
against the lab yet.** Where an attack targets the OpenPLC container's Modbus server, whether
the write reaches the ESP32 depends on the ladder passing the coil through (`Program.st`);
that pass-through is **to verify on the bench**. Closed lab only (`192.168.50.0/24`).

**Two places to aim every Modbus attack.** Each node can be hit at (a) the **ESP32 directly**
(`.1x`/`.2x`) at its native coil/register numbers, bypassing the PLC; or (b) the **OpenPLC
container** (`.11x`/`.12x`), where the SCADA-facing command coils (`%QX0.x`) are written and
the ladder forwards them to the ESP32. The register maps give both columns.

Tool on the Kali VM (`.40`): `modbus_coil_tool.py` (read/write single coil, FC 1/5) — kept on the
**lab-only flash drive**, not in the repo (`Kali/Scripts/README.md`). Holding-register writes
(FC 6/16) need a small extra tool (not yet written).

---

## Scenario 1 — Normal operation (baseline)

- **Goal:** record the steady state the attacks are compared against.
- **Target:** all nodes; no attacker action.
- **Action:** capture 30–60 s of normal polling.
- **Expected:** steady Modbus reads on :502 from each container to its ESP32; HEARTBEAT
  incrementing; no writes except those the PLC program makes.
- **Real-world attack:** none (baseline).
- **ATT&CK:** — · **CVE:** — · **Capture filter:** `mbtcp`

## Scenario 2 — Authorized door access (baseline)

- **Goal:** a legitimate badge event, as the comparison baseline for #3.
- **Target:** BSH-Door — ESP32 `192.168.50.13` / container `192.168.50.113`.
- **Objects (read):** `ACCESS_GRANTED` = DI2, published at container coil 5 (`%QX0.5`, FC 1);
  `DOOR_POSITION` = DI1 → container coil 4 (`BSH-Door/Register_Map.md`, verified Oct 20).
- **Action:** present a valid card (physical).
- **Expected:** `ACCESS_GRANTED` pulses to 1 ~2 s in the polled response, then door position
  changes. **No Modbus write from a foreign host.**
- **Real-world attack:** none (baseline).
- **ATT&CK:** — · **CVE:** — · **Capture filter:** `ip.addr == 192.168.50.13 && mbtcp`

## Scenario 3 — Unauthorized remote door-open (Modbus command injection)

*(Revised Nov 8, 2026 — replaces the physical RFID clone/replay, which needed card-cloning
hardware and put nothing attack-specific on the wire. Luis's decision: demonstrate the
network-level equivalent instead. The reader-level RFID-clone threat is kept as a paper
note for the report — see Background.)*

- **Goal:** open the gate from a rogue host with **no card presented** — the network-level
  equivalent of credential bypass.
- **Target:** BSH-Door — ESP32 `192.168.50.13` / container `192.168.50.113`.
- **Object (write):** `DOOR_OPEN_CMD` = **Coil 0** (FC 5). ESP32-native Coil0; on the container,
  `SCADA_DOOR_OPEN_CMD` at `%QX0.0` = coil 0 (`PLC_Logic/BSH-Door/Register_Map.md`, verified
  Oct 20). No matching `ACCESS_GRANTED` (DI2) pulse in the surrounding polls.
- **Precondition:** PLC running (for the container path); attacker on the subnet.
- **Action:** `python3 modbus_coil_tool.py write 192.168.50.113 0 1` (container) or `...13 0 1`
  (ESP32 direct). **Assumed** the container write passes through — verify the ladder forwards `%QX0.0`.
- **Expected:** gate opens with no card. The **tell** is an FC 5 write to coil 0 from a foreign
  IP with **no `ACCESS_GRANTED` pulse before it** (contrast Scenario 2's authorized baseline).
  Note: `DOOR_POSITION` (DI1) reads true both open and closed (undiagnosed, STATUS.md), so the
  evidence is the write packet + the physical gate, **not** the position bit.
- **Real-world attack:** the unauthenticated-command / credential-bypass class — the insider/
  credential abuse of **Maroochy Shire (2000)**.
- **ATT&CK:** T1692.001 (old T0855) · T0831 · impact **T0827 Loss of Control**
- **CVE:** none — Modbus no-auth design weakness.
- **Capture filter:** `modbus.func_code == 5 && ip.addr == 192.168.50.113` (or `.13` direct)
- **Background (report only, not run):** reader-level RFID UID cloning (MIFARE/Wiegand) is the
  physical-layer cousin — a cloned card yields an identical `ACCESS_GRANTED` pulse to a real one,
  with nothing attack-specific on the wire. Cite the credential-abuse class; no live run.

## Scenario 4 — False fire alarm injection

- **Goal:** sound the fire alarm with no real pull — a forged safety action.
- **Target:** BSH-FireAlarm — ESP32 `192.168.50.12` / container `192.168.50.112`.
- **Object (write):** `ALARM_ACTIVATE` = **Coil 0** (FC 5). ESP32-native Coil0; on the container,
  `SCADA_ALARM_ACTIVATE` at `%QX0.0` = coil 0 (`BSH-FireAlarm/Register_Map.md §3`, verified
  Oct 15). No matching `MANUAL_PULL_TRIGGERED` (DI0) in the surrounding polls.
- **Precondition:** PLC running (for the container path); attacker on the subnet.
- **Action:** `python3 modbus_coil_tool.py write 192.168.50.112 0 1` (container) or `...12 0 1`
  (ESP32). **Assumed** the container write passes through — verify the ladder forwards `%QX0.0`.
- **Expected:** buzzer/alarm outputs active; SCADA FireAlarm view shows `AlarmActivate` true
  while `ManualPull` stays false (the tell).
- **Real-world attack:** **TRITON/TRISIS (2017)** — tampering with a *safety* function; plus
  **Maroochy (2000)** — unauthenticated command.
- **ATT&CK:** T1692.001 (old T0855) · T0831 · impact **T0880 Loss of Safety**
- **CVE:** none — Modbus no-auth design weakness.
- **Capture filter:** `modbus.func_code == 5 && ip.addr == 192.168.50.112` (or `.12` direct)

## Scenario 5 — Elevator lockout (+ persistent variant)

- **Goal:** lock the elevator out of service from a rogue host; the persistent variant keeps
  it locked against the operator.
- **Target:** BSH-Elevator — ESP32 `192.168.50.11` / container `192.168.50.111`.
- **Object (write):** `ELEVATOR_DISABLE` = **Coil 3** (FC 5). ESP32 Coil3; container
  `SCADA_ELEVATOR_DISABLE` at `%QX0.3` = coil 3 (`BSH-Elevator/Register_Map.md §3`, verified
  in use Oct 20). No accompanying `ELEVATOR_FAULT` (DI1).
- **Action (one-shot):** `python3 modbus_coil_tool.py write 192.168.50.111 3 1`.
  **Persistent variant (Industroyer-style):** re-issue the write on a short timer (e.g. every
  1 s) so each SCADA/operator clear of the disable is immediately re-asserted.
- **Expected:** car refuses calls / is held out of service; SCADA Elevator view shows `Lockout`
  true with no fault. Persistent variant: the operator's clear "doesn't stick."
- **Real-world attack:** **Industroyer/CrashOverride (2016)** — looped breaker re-open to beat
  operators; **Ukraine 2015** — operating the control path.
- **ATT&CK:** T1692.001 (old T0855) · T0831 · impact **T0827 Loss of Control**
- **CVE:** none (Modbus no-auth). *(Later phase, real AB PLC: CVE-2020-6111.)*
- **Capture filter:** `modbus.func_code == 5 && ip.addr == 192.168.50.111` (periodic write = the tell)

## Scenario 6 — Building-wide power cut (FC 15 Write Multiple Coils)

*(Repurposed Nov 15, 2026 — the original "Backup power ATS" was dropped Oct 5, 2026; this reuses
slot 6 as a distinct FC-15 scenario. Scenario 7 cuts one rail with FC 5; this cuts all three at once.
**Target switched to FH as the primary example Nov 17, 2026** — see note below.)*

- **Goal:** drop a whole building's nodes in a single coordinated command.
- **Target:** **FH-PowerBox `192.168.50.24` / container `192.168.50.124` (rail coils 0–2) — primary
  example.** BSH-PowerBox `.14`/`.114` mirrors the same coils but is **no longer usable for this
  scenario** (see note below).
- **Object (write):** rail coils **0 `HVAC_RAIL_CMD`, 1 `FIRE_RAIL_CMD`, 2 `DOOR_RAIL_CMD`** (FH
  naming) written together with **FC 15 Write Multiple Coils** (0 = cut, 1 = power).
  `PLC_Logic/FH-PowerBox/Register_Map.md` (confirm names there; BSH's equivalent map uses
  `ELEV_RAIL_CMD` instead of `HVAC_RAIL_CMD` for coil 0, verified Oct 28).
- **Action:** `s06_blackout.py 192.168.50.124` — one FC 15 frame setting coils 0–2 = 0 on the FH
  container; `restore` powers all three on. (The script's default host is still `.114`; pass
  `.124` explicitly, or swap the default — see note below.)
- **Expected:** all three FH nodes lose power together; their heartbeats freeze in SCADA at once
  (contrast #7, which drops one).
- **Real-world attack:** **Ukraine 2015** (operators opened multiple breakers) and **Industroyer**
  (coordinated multi-point commands).
- **ATT&CK:** T1692.001 (old T0855) · T0831 · impact **T0826 Loss of Availability**
- **CVE:** none — Modbus no-auth design weakness.
- **Capture filter:** `modbus.func_code == 15 && ip.addr == 192.168.50.124`

**Note (Nov 17, 2026):** BSH-PowerBox's coil 0 (`ELEV_RAIL_CMD`) no longer actually powers anything — the
BSH-Elevator node brownout-looped when powered through its PowerBox relay rail (heavy DC-motor load on
a weak source) and was moved to permanent USB power instead. Cutting all three BSH rails would now only
take down 2 of 3 nodes (FireAlarm, Door), not a true "whole building" blackout — so FH, where all three
rails still actually power their nodes, is the scenario's primary target going forward. BSH is still a
valid demo of "2 of 3 nodes drop," just not the full-blackout story. See `STATUS.md` and `Logs/2026-11-17.md`.

## Scenario 7 — Power box manipulation

- **Goal:** cut (or force) a building's power rail to a node from an unexpected host — a
  denial-of-power.
- **Target:** BSH-PowerBox `192.168.50.14`/`.114`; FH-PowerBox `192.168.50.24`/`.124`.
- **Objects (write):** BSH rails — **Coil 0** `ELEV_RAIL_CMD`, **Coil 1** `FIRE_RAIL_CMD`,
  **Coil 2** `DOOR_RAIL_CMD` (FC 5; 1 = node powered, NO relay, no inversion). Container
  `SCADA_*_RAIL_CMD` at `%QX0.0`–`%QX0.2` (`BSH-PowerBox/Register_Map.md §3`, verified Oct 28).
  FH-PowerBox mirrors it (HVAC/FireAlarm/Door rails) — **confirm names in `FH-PowerBox/Register_Map.md`.**
- **Action:** write a rail coil 0 or 1 (cut or force), e.g. `... write 192.168.50.114 1 0` to drop
  **the FireAlarm rail** — **the worked example as of Nov 17, 2026**. **Coil 0 (Elevator) is no
  longer a valid example** (see Scenario 6's note — the elevator is on permanent USB power and its
  rail no longer controls anything real); Coil 2 (Door) also still works as an example.
- **Expected:** the targeted node loses/gains power; its relay LED and `*_RAIL_STATUS` (DI) follow;
  the powered node goes offline in SCADA (heartbeat freezes).
- **Real-world attack:** **Ukraine 2015** — opening breakers to cut power.
- **ATT&CK:** T1692.001 (old T0855) · T0831 · impact **T0826 Loss of Availability**
- **CVE:** none (Modbus no-auth).
- **Capture filter:** `modbus.func_code == 5 && (ip.addr == 192.168.50.114 || ip.addr == 192.168.50.124)`

## Scenario 8 — Cross-building cascade

- **Goal:** show an event in one building driving automated writes to both Fire Alarm nodes.
- **Target:** BSH-Door event → SCADA → BSH-FireAlarm + FH-FireAlarm (`.112` + `.122`).
- **Objects:** door `ACCESS_GRANTED`/`IR_DETECTED` (read) then `ALARM_ACTIVATE` Coil 0 writes
  to both fire nodes. *(Note: the IR-without-card cascade was dropped Nov 3; the live cascade is
  the pull-station rule — see `Logs/2026-11-03.md`. As of Nov 17, the BSH side writes
  `BSH/Elevator/ElevatorDisable` = True (not a rail coil — see Scenario 6/7 notes); the FH side
  still cuts `FH/PowerBox/HvacRailCmd`. This scenario captures whichever automated multi-node
  write chain is in place.)*
- **Expected:** the **order and timing** of the two writes is the evidence.
- **Real-world attack:** **Industroyer** — automated multi-point commands; **Stuxnet** — orchestration.
- **ATT&CK:** T1692.001 (old T0855) · T0831 · **CVE:** none
- **Capture filter:** `modbus.func_code == 5 || modbus.func_code == 15`

## Scenario 9 — Node isolation / resilience (DoS)

- **Goal:** take one node/subsystem offline and show the other seven keep answering; measure
  how fast SCADA flags it.
- **Target:** any ESP32 (`.1x`/`.2x`) or container (`.11x`/`.12x`) on :502.
- **Vectors:** (a) **connection-slot exhaustion** — hold the ESP32's small number of Modbus-TCP
  client slots (Emelianov `ModbusIP_ESP8266`; exact `MODBUSIP_MAX_CLIENTS` **to confirm in the
  library header**); (b) SYN/packet flood (`hping3`); (c) malformed-frame fuzzing; (d) stop the
  container. Time-box and watch recovery.
- **Expected:** HEARTBEAT freezes for that node; SCADA `Status` → yellow → red; `NodeOffline`
  alarm fires (the alarm you built Nov 3). The other seven unaffected.
- **Real-world attack:** **Industroyer SIPROTEC DoS** (ESET-reported, Dragos-disputed) via
  **CVE-2015-5374**; general availability attacks (Ukraine recovery sabotage).
- **ATT&CK:** **T0814 Denial of Service** · impact T0826 Loss of Availability, T0829 Loss of View
- **CVE:** CVE-2015-5374 is the *real-world reference* (Siemens relays, not our hardware); our
  DoS exploits stack/library limits, no product CVE.
- **Capture filter:** `tcp.flags.reset == 1 || tcp.analysis.retransmission` with `ip.addr == <target>`

## Scenario 11 — HVAC spoofing / setpoint tampering

- **Goal:** push the HVAC setpoint to a dangerous value from a foreign host (Oldsmar analogue).
- **Target:** FH-HVAC — ESP32 `192.168.50.21` / container `192.168.50.121`.
- **Object (write):** `TARGET_TEMP_C` = **Holding Register 0** (FC 6/16), **tenths of °C**
  (240 = 24.0 °C). Container `SCADA_TARGET_TEMP_C` at `%QW0` = HR 0 (`FH-HVAC/Register_Map.md §3`,
  verified Oct 21). Note `TEMP_C` (the *reading*) is an **Input Register** — not writable with a
  standard FC, so you spoof the **setpoint**, not the measured temperature.
- **Action:** FC 6/16 write of an extreme value to HR 0 (needs the holding-register tool, not yet
  written). Optionally also write `HVAC_ENABLE` Coil 0.
- **Expected:** HVAC drives toward the forged setpoint; `COMPRESSOR_RUNNING` no longer matches the
  temperature trend; SCADA HVAC view shows the tampered target.
- **Real-world attack:** **Oldsmar (2021)** — setpoint to a dangerous value; **Stuxnet** — false process.
- **ATT&CK:** **T0836 Modify Parameter** · T0831 · (if the shown value is forged) T1692.002/T0832
- **CVE:** none (Modbus no-auth).
- **Capture filter:** `(modbus.func_code == 6 || modbus.func_code == 16) && ip.addr == 192.168.50.121`

## Scenario 12 — PLC logic injection (OpenPLC web UI)

- **Goal:** replace the control logic itself — the Stuxnet class.
- **Target:** any OpenPLC container web UI, `http://<container>:8080` (all eight expose 8080,
  `Piranga_Workflow_v2.md:114-131`). Worked example: BSH-FireAlarm `192.168.50.112`.
- **Not Modbus:** :502 only exposes IO. Logic changes go through the **engineering channel** (the
  web UI).
- **Vector A (creds):** reach :8080 → log in with the kept default login → upload a malicious
  program → Compile → runtime reloads. With valid (default) creds this is also **`CVE-2026-14480`**
  — an authenticated arbitrary **file write → code execution** in the program-upload workflow
  (CWE-73, CVSS 9.9 Critical, CISA ICSA-26-190-01): write files into the runtime core and they are
  compiled into the running binary. **Vector B (credential-less): `CVE-2026-88020`** — reflected
  XSS (CWE-79) in the v3 web UI routing (CISA ICSA-26-265-09, CVSS 6.1): a crafted URL hijacks the
  operator's session cookie and issues the same state-changing requests.
- **Expected:** on :502 the normal poll drops for a few seconds during reload, then the node runs
  attacker logic (e.g. force `ALARM_ACTIVATE` on, drop a safety interlock). **Mandatory restore:**
  re-upload the known-good `PLC_Logic/<System>/Program.st`.
- **Real-world attack:** **Stuxnet (2010)** — reprogram the controller; **PIPEDREAM/INCONTROLLER
  (2022)** — ICS toolkit.
- **ATT&CK:** **T0843 Program Download** (.001/.002/.003) · **T0889 Modify Program** · T0842 Network Sniffing
- **CVE:** **CVE-2026-88020** (OpenPLC v3 web-UI XSS, 6.1) and **CVE-2026-14480** (OpenPLC v3
  authenticated file-write → RCE, 9.9 Critical, ICSA-26-190-01). Vendor fix for both = v4; project
  stays on v3 by decision, so the lab carries them on purpose. See `References.md`.
- **Capture filter:** `tcp.port == 8080 && ip.addr == 192.168.50.112` (Follow HTTP Stream) +
  `mbtcp && ip.addr == 192.168.50.112` for the reload gap.

## Scenario 13 — ESP32 bricking (extra, final, destructive)

- **Goal:** demonstrate a **destructive** attack on a node and its **recovery** — the extra, last
  scenario (added Nov 7, 2026, professor-requested).
- **Target:** a **dedicated spare/expendable ESP32 on the bench — never a production node.** There
  are plenty of spare boards.
- **Not a Modbus/network attack:** bricking is done through the device's **flashing interface**
  (local USB/serial). Our firmware has no OTA path (ModbusIP only), so the realistic lab method is
  an attacker with **device/flash access** performing destructive impact — the "destructive cleanup"
  final stage (`Real_World_Incidents.md §3`, Industroyer wiper / bricked converters).
- **Method — recoverable soft-brick (this is the procedure):** erase/corrupt the application flash
  (standard `esptool` erase). The board no longer boots → heartbeat gone → SCADA shows it offline /
  `NodeOffline` fires (same symptom as #9, but it looks permanent). **Recover** by re-flashing the
  node's known-good firmware over UART download mode (enabled by default), and it comes back. The
  attack **and** the recovery are the demo.
- **Permanent brick — documented, NOT performed:** burning the eFuse that disables ROM/UART download
  mode (`esp_efuse_disable_rom_download_mode()`) makes the board unflashable — *"if UART Download mode
  is disabled then esptool.py can not work on the device"* (`References/ESP-IDF/05-security-guides.md`
  lines 70–72). That is irreversible and only wastes a board; we **do not** perform it.
- **Expected:** soft-bricked board dead (no boot, no heartbeat, SCADA red/alarm); after reflash,
  normal. Nothing else on the subnet affected.
- **Real-world attack:** Industroyer / Ukraine 2015 destructive cleanup (recovery denial).
- **ATT&CK:** T0857 System Firmware / T0839 Module Firmware · impact **T0826 Loss of Availability**
  (+ T0809 Data Destruction for the erase) — our analysis, verify IDs.
- **CVE:** none — destructive use of normal flashing tooling, not a software vulnerability.
- **Mitigation:** physical access control to the USB/flash port; **recoverability** (keep per-node
  firmware images — the backup theme); detection via the heartbeat / `NodeOffline` alarm you built.
- **Runbook:** `Security_Capture/Kali/Scenario13_ESP32_Bricking_Runbook.md`.

## Scenario 14 — IRONGATE-style combined MITM (false view / manipulation of view)

*(Added Nov 8, 2026, Luis's decision. This is the "HMI lies" half of Stuxnet that
`Real_World_Incidents.md` previously marked out of scope — now **in scope** as a deliberate
man-in-the-middle scenario. **GATED:** the SCADA-side position (B) depends on the Docker/L2
networking; feasibility to be checked **Nov 9, 2026** before any runbook is written. Scenario 13
remains the final **destructive** scenario; #14 is a non-destructive **deception** scenario.
Nothing run.)*

- **Goal:** fake the monitored view at **both** tiers so a real manipulation (or a real field
  event) is hidden from the PLC logic **and** from the operator — the IRONGATE technique of
  recording normal values and replaying them upward while tampering underneath.
- **Target:** the BSH-FireAlarm path end to end — ESP32 `192.168.50.12` ⟷ container
  `192.168.50.112` ⟷ Ignition `192.168.50.30` (worked example, consistent with the learning slice).
- **Two MITM positions, combined:**
  - **A — field↔PLC** (ESP32 `.12` ⟷ container `.112`): rewrite the ESP32's input reads so the
    PLC sees a false field (e.g. `MANUAL_PULL_TRIGGERED` false, gas normal, `HEARTBEAT` still
    incrementing). One lie here propagates up the whole chain because OpenPLC republishes field state.
  - **B — PLC↔SCADA** (container `.112` ⟷ Ignition `.30`): forge what Ignition reads **and swallow
    the operator's write commands** (e.g. a manual `Alarm OFF`) while echoing success. Adds the
    ability to make the PLC and the operator believe different, self-consistent stories.
- **Method:** ARP spoofing (ettercap/bettercap class) to gain each position, plus a **minimal
  Modbus value-rewriting proxy** at each — demonstration-level, not a weaponized framework
  (`Kali/Scripts/README.md` rule 5). Record a baseline of normal values first, then replay/rewrite.
- **Preconditions:** attacker (Kali `.40`) on the subnet; **the L2 topology must actually allow
  insertion at each position.** A is plausible (ESP32s are real devices with their own MACs); **B
  depends on how the containers sit on the network (own MAC vs NAT behind VM 100 `.10`) — the
  Nov 9 check.** If B is infeasible, the scenario degrades to A-only (still a valid false-view demo).
- **Expected / the tell:** on the wire — the **ARP takeover** (gratuitous ARP / a changed MAC for
  the target IPs; `arp` filter) and **two Modbus streams that disagree** (what the ESP32 really
  sends vs what the container/Ignition receives). On the system — a field manipulation or a **real
  pull suppressed** while Ignition shows calm and `NodeOffline`/alarms stay quiet. *(Whether
  suppressing `MANUAL_PULL_TRIGGERED` to the PLC also kills the buzzer depends on where the alarm
  latch lives in the BSH-FireAlarm ladder — **assumed**, confirm against `PLC_Logic/BSH-FireAlarm/`.)*
- **Real-world attack:** **IRONGATE (FireEye, 2016)** — MITM faking process I/O in a *simulated*
  Siemens environment (technique reproduced, not the malware); the false-view half of **Stuxnet
  (2010)**; the safety-**suppress** direction of **TRITON/TRISIS (2017)**.
- **ATT&CK:** **T0830 Adversary-in-the-Middle** · **T0832 Manipulation of View** · **T0856 Spoof
  Reporting Message** · (suppress direction) impact **T0880 Loss of Safety**.
- **CVE:** none — ARP spoofing + unauthenticated Modbus, both design weaknesses.
- **Mitigation:** **M0814 Static Network Configuration** (anti-ARP-spoof) · **M0802 Communication
  Authenticity** · **M0931 Network Intrusion Prevention** (Dynamic ARP Inspection / spoof
  detection) · **M0937 Filter Network Traffic** · **M0930 Network Segmentation**.
- **Capture filter:** `arp` (the takeover) **plus** `mbtcp && ip.addr == 192.168.50.112` captured
  at **two points** (ESP32-side and SCADA-side) so the pcap shows the streams disagree — see
  `Capture_Setup.md` placement table.
- **Restore:** stop the ARP spoofing; verify the ARP tables re-converge and real values flow again.
  No persistent change to any node.
- **Runbook:** _to be written_ (`Security_Capture/Kali/Scenario14_IRONGATE_MITM_Runbook.md`) —
  **after** the Nov 9 network check confirms whether position B is reachable.

## Scenario 10 — Red / Blue exercise (composite)

- Runs #3, #4, #5, #7 and #11 in one session on a single timeline for the Blue Team to review
  afterward. No new technique; the union of the above. Capture filter: all of the above.

---

## Scenario → real-world attack → ATT&CK → CVE (quick index)

| # | Scenario | Real-world attack | ATT&CK (primary) | CVE |
|---|---|---|---|---|
| 3 | Unauthorized door-open (Modbus) | Maroochy 2000 | T1692.001 · T0827 | — |
| 4 | False fire alarm | TRITON / Maroochy | T1692.001 · T0880 | — |
| 5 | Elevator lockout | Industroyer / Ukraine 2015 | T1692.001 · T0827 | (later: CVE-2020-6111) |
| 6 | Building blackout (FC 15 multi-coil) | Ukraine 2015 / Industroyer | T1692.001 · T0826 | — |
| 7 | Power box | Ukraine 2015 | T1692.001 · T0826 | — |
| 8 | Cross-building cascade | Industroyer / Stuxnet | T1692.001 · T0831 | — |
| 9 | Node isolation / DoS | Industroyer SIPROTEC DoS | T0814 | CVE-2015-5374 (reference only) |
| 11 | HVAC setpoint tamper | Oldsmar / Stuxnet | T0836 · T0831 | — |
| 12 | PLC logic injection | Stuxnet / PIPEDREAM | T0843 · T0889 | **CVE-2026-88020**, **CVE-2026-14480** |
| 13 | ESP32 bricking (spare board, destructive, last) | Industroyer destructive cleanup | T0857/T0839 · T0826 | — |
| 14 | IRONGATE combined MITM (false view) *(added Nov 8; gated on Nov 9 net check)* | IRONGATE / Stuxnet / TRITON | T0830 · T0832 · T0856 | — |
