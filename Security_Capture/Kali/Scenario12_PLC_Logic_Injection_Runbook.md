# Scenario 12 runbook — PLC logic injection (controlled demonstration)

Created Nov 7, 2026. Step-by-step procedure for demonstrating Scenario 12 on the **closed,
authorized lab only** (`192.168.50.0/24`). Detail: `Security_Capture/Scenarios.md` (#12);
capture setup/filters: `Security_Capture/Capture_Setup.md`, `Scenario_Capture_Plan.md`;
run log template: `Security_Capture/Capture_Log_Template.md`; CVEs: `Security_Capture/References.md`.

## What this demonstrates (and what it does not)

**Demonstrates:** that OpenPLC v3's management interface, kept on the **default login** with no
network restriction, lets a host that is not the operator **change the control logic** of a PLC —
the Stuxnet-class risk, at bench scale. The "exploit" here is simply **using OpenPLC's own
Programs upload feature from an unauthorized seat**; no custom exploit code is needed, which is
exactly the point for the report.

**Does NOT weaponize the CVEs.** OpenPLC v3 also carries `CVE-2026-88020` (web-UI XSS) and
`CVE-2026-14480` (authenticated file-write → RCE, CVSS 9.9) — see `References.md`. This runbook
**cites them from the CISA advisories as the documented weaknesses** but does not build a
working XSS/RCE exploit; the default-credential logic change is enough to make the point and is
safe to reverse. Keep it that way.

## Scope and safety (read before anything)

- **Closed lab only.** Target only `192.168.50.0/24`. One container at a time.
- **Pick one target:** BSH-FireAlarm container `192.168.50.112` (matches the learning slice).
- **Reversible:** you will re-upload the known-good program at the end. **Back it up first.**
- **Heads-up effects:** reloading the runtime briefly drops the Modbus server (:502), so SCADA
  will show that node offline for a few seconds — expected.
- **No credentials in any file** (`CLAUDE.md` §8). Do not write the login anywhere in the repo.

## Prerequisites

1. Kali VM `192.168.50.40` on the lab subnet; browser + `curl` available.
2. Wireshark/tcpdump capture ready per `Capture_Setup.md` (capture on Kali for the `:8080`
   session, and on the OpenPLC VM / a mirror for the `:502` effect if possible).
3. The node's **known-good program saved**: `PLC_Logic/BSH-FireAlarm/Program.st` (the deploy
   artifact) is the restore source — confirm it matches what the container is running before you
   start.
4. The scenario's register facts: `ALARM_ACTIVATE` = Coil 0 (`PLC_Logic/BSH-FireAlarm/Register_Map.md`).

> **Accuracy note.** There is no OpenPLC manual in `References/`, so the exact menu names/paths
> below are **well-known OpenPLC v3 behavior, not cited from a project doc** — confirm each against
> the live `:8080` as you go (Step 1) and record what you actually see in the run log. Label every
> result verified / reported / assumed.

## Procedure (you drive; one step at a time)

**Step 0 — Baseline.** Start the capture. Record 30–60 s of normal `:502` polling (HEARTBEAT
incrementing) and note the wall-clock time and both machines' clocks (NTP is off — `Capture_Setup.md` §4).

**Step 1 — Confirm the management interface.** From Kali, open `http://192.168.50.112:8080` in the
browser. Confirm the OpenPLC login page loads (proves the engineering channel is reachable from a
non-operator host). In the capture you should see plain **HTTP on :8080** — note that it is
**cleartext** (this is the finding behind `CVE-2026-88020`). Write down the exact page/menu names
you see so the later steps match your build.

**Step 2 — Authenticate.** Log in with the lab's OpenPLC credentials (do **not** record them in any
file). The point being demonstrated: the kept **default login** means any host on the subnet can
reach this. If you had already changed the password, note that the cleartext HTTP still exposes it.

**Step 3 — Prepare the modified program.** Take a **copy** of the node's own `Program.st` and make
**one minimal, obvious change**: a rung that forces `ALARM_ACTIVATE` (Coil 0) ON regardless of the
pull station. Keep it minimal and clearly labeled — this is a demonstration artifact, not a payload.
Save it as a clearly-named demo file **outside** `PLC_Logic/` (e.g. on the Kali VM), so the
known-good `Program.st` in the repo stays untouched.

**Step 4 — Upload and load the modified logic.** In the OpenPLC web UI, use **Programs → upload**
the modified program, then **Compile**, then **Start PLC / reload** (confirm the exact labels from
Step 1). This is OpenPLC's own intended workflow — that an unauthorized seat can do it is the
vulnerability.

**Step 5 — Observe the effect.** Expected (assumed until seen on the bench): the buzzer sounds; in
Ignition the FireAlarm view shows `AlarmActivate` true while `ManualPull` stays **false** (the
tell — an alarm with no physical trigger). In the `:502` capture, note the brief poll gap during
the reload, then the changed behavior.

**Step 6 — Capture the evidence.** Stop/segment the capture. Save: the `:8080` session (Follow HTTP
Stream shows the cleartext management traffic), the `:502` before/after, and screenshots of the UI
and of the Ignition effect. Name them per `Scenario_Capture_Plan.md` ("For the report").

**Step 7 — RESTORE (mandatory).** In the web UI, **re-upload the known-good
`PLC_Logic/BSH-FireAlarm/Program.st`**, Compile, Start. Confirm: buzzer off, `AlarmActivate` false,
HEARTBEAT climbing again, SCADA dot back to green. Do not end the session until the node is back to
normal. Record the restore time in the run log.

**Step 8 — Log.** Fill in `Security_Capture/Capture_Log_Template.md` (actions + wall-clock times),
hash the pcap (`sha256sum`) if it will be handed in, and write the day's `Logs/` entry.

## Capture filters (from `Scenario_Capture_Plan.md`)

- Management session: `tcp.port == 8080 && ip.addr == 192.168.50.112` (use Follow HTTP Stream).
- Reload gap + changed behavior: `mbtcp && ip.addr == 192.168.50.112`.

## Expected results (fill in verified/reported after the run)

| Check | Expected | Result |
|---|---|---|
| `:8080` reachable from Kali `.40` | login page loads | |
| `:8080` traffic is cleartext | credentials/upload visible in HTTP stream | |
| Modified program loads | Compile OK, PLC runs it | |
| Effect | buzzer on; SCADA `AlarmActivate` true, `ManualPull` false | |
| `:502` reload gap | brief poll outage during reload | |
| Restore | known-good `Program.st` reloaded; node normal, SCADA green | |

## For the report

Pair this with: the ATT&CK IDs (T0843 Program Download, T0889 Modify Program — `ATTACK_ICS_Mapping.md`),
the CVEs (`References.md`), and the mitigations (M0804 Human User Authentication, M0927 Password
Policies, M0932 MFA, M0808 Encrypt Network Traffic, M0930 Network Segmentation, M0937 Filter Network
Traffic, M0951 Update Software → v4). The one-line lesson: an unauthenticated, cleartext engineering
interface on the OT network means logic integrity depends entirely on network reach.
