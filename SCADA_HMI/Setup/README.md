# Setup -- Gateway facts and procedures

No passwords, license keys or activation tokens in this folder or anywhere in the repo.

## Gateway (recorded Oct 22, 2026; see `Logs/2026-10-22.md` and `Proxmox_Build_Log.md`)

| Item | Value |
|---|---|
| Host | VM 101 `scada-hmi`, Ubuntu 24.04.5.1 LTS desktop, 192.168.50.30 |
| Ignition | 8.3.10 (Standard), Gateway name `Ignition-SCADA-HMI` |
| Ports | 8088 (HTTP), 8043 (HTTPS), 8060 |
| Installed as | user `scada-hmi` (not root) |
| License | Capstone, one activation (stays on this Gateway), expires 2027-01-31, rolling lease about every 72 h, so VM 101 needs internet |
| Admin account | `AdminLuis` (password kept outside every project file) |
| Disabled modules | Enterprise Administration, Voice Notification (not licensed) |
| Snapshots | `before-ignition`, `ignition-licensed` (Proxmox) |
| Designer | Designer Launcher on the laptop; project `CyberCity` |

## To write here as it is done

- Device settings per OpenPLC container (name, host, port, unit ID, addressing option).
- Backup procedure: Gateway backup and project export before the license ends 2027-01-31; save them in `Exports/`.
- Hardening list (disable unused drivers) once the build works.
