# Kali -- attack scripts and attack reports

Created Oct 20, 2026. **Moved under `Security_Capture/` on Nov 7, 2026 — this folder is now
`Security_Capture/Kali/`; paths below are relative to it.** **Status: created but not in use
yet.** Nothing goes in this folder until the attack-scenario session starts (Section 11 of
`Piranga_Workflow_v2.md`).

## What will live here

- **Scripts** run from the Kali VM during the attack scenarios (for example the read-only
  Modbus read script that is on the VM today as `~/modbus_read.py`, and the attack scripts).
- **Reports** of each attack run: what was done, from where, when, and what the system did.

Subfolders are created when the first file arrives, not before.

## The Kali VM

VM 102 `Kali` on `pve`, Kali 2026.2, static IP **192.168.50.40**, user `kali`
(see `Proxmox_Build_Log.md`, "Oct 19, 2026 (night)").

## Rules

1. **Closed lab only.** Attack only the testbed subnet `192.168.50.0/24`, the same rule as
   `../README.md` (the parent `Security_Capture/`).
2. **No passwords, license keys or Wi-Fi credentials in any file here.** The folder is pushed
   to a private GitHub repo (`Next_Steps.md`, Backups section).
3. **One home per file.** The network evidence (the `.pcapng` and its run log) stays in the
   parent `Security_Capture/` (`../`). A report here links to its capture there by file name
   instead of copying it.
4. Log every new file or retired file in that day's file in `Logs/`.
5. **Runnable attack scripts live in `Scripts/`** (in the repo, tracked; decision Nov 8, 2026,
   supersedes the Nov 7 flash-drive-only rule — see `Logs/2026-11-08.md`). They reach GitHub only
   when Luis commits/pushes; keep **no secrets** in them (`CLAUDE.md` §8) and keep them
   demonstration-level. Runbooks (e.g. `Scenario12_PLC_Logic_Injection_Runbook.md`) and reports stay
   here in this `Kali/` folder; pcaps and run logs stay in the parent `Security_Capture/` (`../`).
