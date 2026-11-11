# Security_Capture/Kali/Scripts — attack scripts (in-repo; closed-lab-only, authorized)

Created Nov 7, 2026. **Revised Nov 8, 2026 — scripts now live here in the repo** (Luis's
decision), reversing the Nov 7 "flash-drive only" rule. See `Logs/2026-11-08.md`.

## Handling policy — read first

The **runnable attack scripts** for the scenarios in `Security_Capture/Scenarios.md` live **in
this folder, tracked by git**, alongside the manifest below — one file per scenario, written in
**Python 3** (standard library where possible). They reach
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
| 3 — Unauthorized door-open | _tbd_ | FC 5 write `DOOR_OPEN_CMD` coil 0 → `.113` (revised Nov 8, 2026 — replaces RFID clone) | `modbus_coil_tool.py` |
| 4 — False fire alarm | _tbd_ | FC 5 write `ALARM_ACTIVATE` coil 0 → `.112` | `modbus_coil_tool.py` |
| 5 — Elevator lockout | _tbd_ | FC 5 `ELEVATOR_DISABLE` coil 3 → `.111` (+ timed loop for the persistent variant) | `modbus_coil_tool.py` |
| 7 — Power box | _tbd_ | FC 5 rail coil → `.114`/`.124` | `modbus_coil_tool.py` |
| 9 — Node isolation / DoS | _tbd_ | connection/flood DoS of one node | hping3 / custom |
| 11 — HVAC setpoint | _tbd_ | FC 6/16 `TARGET_TEMP_C` HR 0 → `.121` | (holding-register tool, to write) |
| 12 — PLC logic injection | n/a (web-UI procedure) | upload modified `Program.st` via `:8080` | browser / curl — see `../Scenario12_PLC_Logic_Injection_Runbook.md` |
| 13 — ESP32 bricking (spare board) | n/a (local flash) | `esptool` flash erase (soft-brick) + reflash recovery; eFuse permanent brick documented-not-performed | esptool — see `../Scenario13_ESP32_Bricking_Runbook.md` |
| 14 — IRONGATE combined MITM (false view) | _tbd_ (**gated** on the Nov 9, 2026 Docker/L2 net check) | ARP-spoof both links + a minimal Modbus value-rewriting proxy per position (A: ESP32 `.12`↔container `.112`; B: container `.112`↔Ignition `.30`) — demonstration-level, keep the rewrite rules minimal (rule 5) | ettercap/bettercap + custom proxy — runbook TBD after Nov 9 |

Fill in the real file names as scripts are created here.

## The base tool

`modbus_coil_tool.py` (read/write a single coil, standard library) is the shared base tool for the
coil-write scenarios (#3/#4/#5/#7). It was briefly moved to a flash drive on Nov 7, 2026 and removed
from the working tree; under the Nov 8 decision it belongs **here in this folder** again. A copy
still sits at `Prototype_PowerRails/BenchTest/modbus_coil_tool.py`, and the original is in git
history (`git show <rev>:Kali/modbus_coil_tool.py`) — **Luis to confirm which copy is canonical**,
then place it here. (See `Logs/2026-11-08.md`.)
