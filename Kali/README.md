# Kali -- attack scripts and attack reports

Created Oct 20, 2026. **Status: created but not in use yet.** Nothing goes in this folder
until the attack-scenario session starts (Section 11 of `Piranga_Workflow_v2.md`).

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
   `Security_Capture/README.md`.
2. **No passwords, license keys or Wi-Fi credentials in any file here.** The folder is pushed
   to a private GitHub repo (`Next_Steps.md`, Backups section).
3. **One home per file.** The network evidence (the `.pcapng` and its run log) stays in
   `Security_Capture/`. A report here links to its capture there by file name instead of
   copying it.
4. Log every new file or retired file in that day's file in `Logs/`.
