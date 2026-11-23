# Security_Capture/Kali/Scripts — attack scripts (in-repo; closed-lab-only, authorized)

Created Nov 7, 2026. **Revised Nov 8, 2026 — scripts now live here in the repo** (Luis's
decision), reversing the Nov 7 "flash-drive only" rule. See `Logs/2026-11-08.md`.

## Handling policy — read first

The **runnable attack scripts** for the scenarios in `Security_Capture/Scenarios.md` live **in
this folder, tracked by git**, alongside the manifest below — one file per scenario, written in
**Python 3**, each **self-contained** (reimplements the minimal Modbus it needs, standard library
only — see "Script design" below). They reach
GitHub only when Luis commits and pushes (git is unreachable from an agent shell, so Claude never
pushes); the repo is private.

**Because these scripts are pushed, two limits are strict:**
- **Nothing sensitive in any script** — no credentials, keys, Wi-Fi details or license keys
  (`CLAUDE.md` §8). Pass secrets as arguments or read them from the environment at run time; never
  hard-code them.
- **Demonstration-level only** — a Modbus coil/register write, an nmap scan, a basic DoS. Do **not**
  commit weaponized exploit chains for the cited CVEs (`Security_Capture/References.md`).

The `.gitignore` here tracks the scripts but keeps out captures (`*.pcap*`), logs and secret files.

### Rules

1. **Closed lab only** (`192.168.50.0/24`) — same rule as `../README.md` (Kali) and `../../README.md` (Security_Capture). Scripts run from the Kali VM (`.40`).
2. **Nothing sensitive in any script** — no credentials, keys, Wi-Fi details or license keys
   (`CLAUDE.md` §8). They are pushed to GitHub, so never hard-code a secret; pass it as an argument
   or read it from the environment at run time.
3. **Authorized testing.** This is Luis's own isolated capstone lab; his professor is aware of the
   full project. Scripts exist only to demonstrate the scenarios in `Security_Capture/Scenarios.md`.
4. **Demonstration-level.** Keep scripts to what the runbooks need (a Modbus coil/register write, a
   scan, a DoS). Do **not** build weaponized exploit chains for the cited CVEs
   (`Security_Capture/References.md`).
5. **Log** every script created/retired in that day's `Logs/` file (name + purpose).

## Manifest — scripts live in this folder

| Scenario | Script file | Purpose | Base |
|---|---|---|---|
| Recon | `recon_scan.py` | port/Modbus scan of `192.168.50.0/24`, find `:502` servers | nmap (via subprocess) |
| 3 — Unauthorized door-open | `s03_door_open_inject.py` | FC 5 write `DOOR_OPEN_CMD` coil 0 → `.113` (before/after coil read around it); revised Nov 8, 2026 — replaces RFID clone | self-contained (stdlib FC 1/5) |
| 4 — False fire alarm | `s04_false_alarm.py` | FC 5 write `ALARM_ACTIVATE` coil 0 → `.112`; before/after coil read; `restore` arg silences it | self-contained (stdlib FC 1/5) |
| 5 — Elevator lockout | `s05_elevator_lockout.py` | FC 5 `ELEVATOR_DISABLE` coil 3 → `.111`; `persistent [S]` re-asserts every S s (Industroyer-style), `restore` clears; before/after read | self-contained (stdlib FC 1/5) |
| 6 — Building blackout | `s06_blackout.py` | **FC 15** Write Multiple Coils 0–2 = 0 → `.114`/`.124` (cut all rails in one frame); `restore` powers all on; before/after read. Repurposes dropped slot 6 (Nov 15, 2026) | self-contained (stdlib FC 1/15) |
| 7 — Power box | `s07_powerbox.py` | FC 5 rail coil (0 ELEV / 1 FIRE / 2 DOOR) → `.114` (default) or `.124`; `0`=cut / `1`=power; before/after read | self-contained (stdlib FC 1/5) |
| 9 — Node isolation / DoS | `s09_dos.py` | connection-slot exhaustion, incl. `SOURCE_IPS` multi-IP mode — **ruled out empirically Nov 23, 2026** (10 distinct source IPs held 45s, zero rejections; does not isolate this hardware) | self-contained (stdlib sockets) |
| 9 — SYN flood (the vector that actually works) | `s09_synflood.py` | bounded `hping3 -S -p 502 -c N --flood` burst (default N=3000, `--dry-run` to preview); **verified Nov 23, 2026: causes a network-wide outage (all 8 dots + Ignition), not single-node isolation** — see its docstring | thin wrapper around `hping3` |
| 15 — Multi-target DoS (extra) | `s15_multitarget_dos.py` | same connection-slot exhaustion as #9, threaded against **two hosts at once** (default `.14` + `.24`); one attacker, several victims — explicitly **NOT** a DDoS (see script docstring) | self-contained (stdlib sockets + threading) |
| 10 — Red/Blue composite | `s10_redblue.py` | runs S3/S4/S5/S6/S7/S11 in sequence on one timeline for one pcap; `restore` undoes what it can | drives the s0x scripts (no new technique) |
| 11 — HVAC setpoint | `s11_hvac.py` | FC 6 write `TARGET_TEMP_C` HR 0 → `.121` (tenths-degC; default 900 = 90 C); FC 3 before/after read; `restore` → 240 | self-contained (stdlib FC 1/3/6) |
| 12 — PLC logic injection | `s12_web_inject.py` (demo scaffold) | `:8080` web-UI: `creds` (login+upload) / `cookie` (session-cookie replay); endpoints TODO-confirm vs live; **NOT** a weaponized XSS | `requests` — see `../Scenario12_PLC_Logic_Injection_Runbook.md` |
| 13 — ESP32 bricking (spare board) | `s13_brick.py` | wraps `esptool` erase (soft-brick) + reflash recovery; spare-board confirm prompt; **never** the eFuse step | esptool — see `../Scenario13_ESP32_Bricking_Runbook.md` |
| 14 — IRONGATE combined MITM (false view) | `s14_mitm_proxy.py` (scaffold, **gated**) | minimal Modbus value-rewriting relay (`rewrite_response` hook) — demonstration-level (rule 5); ARP spoof via bettercap/ettercap (external) + iptables REDIRECT; **gated** on the Docker/L2 network check; runbook `../Scenario14_IRONGATE_MITM_Runbook.md` | stdlib relay + bettercap/ettercap |

Fill in the real file names as scripts are created here.

## Script design — self-contained per scenario (decided Nov 15, 2026)

Each scenario script is **self-contained**: it implements the little Modbus/TCP it needs (FC 1 read
coils, FC 5 write single coil) inline, standard library only — no external tool, no install. This
**supersedes the Nov 8 "thin wrapper that calls `modbus_coil_tool.py`" idea**: Luis's call, so each
script fully shows what it does (better for the report) and runs anywhere on its own. The ~12-line
Modbus block is copied into each coil scenario (#3/#4/#5/#7); the FC 6/16 scenarios (#11) add a
holding-register write the same way.

`modbus_coil_tool.py` stays in this folder as an **optional standalone interactive tool** (handy for
ad-hoc `read`/`write` at the shell), but the scenario scripts no longer depend on it. A copy also sits
at `Prototype_PowerRails/BenchTest/modbus_coil_tool.py`, and the original is in git history
(`git show <rev>:Kali/modbus_coil_tool.py`). (See `Logs/2026-11-15.md`.)
