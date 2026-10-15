# Next steps -- handoff note

Updated Oct 14, 2026. Read `00_START_HERE.md` first, then this file. If this file and an older doc disagree, this file and `Piranga_Workflow_v2.md` Section 10 are newer.

## Where the project stands

- All 8 ESP32 nodes: Phase B complete, bench-confirmed (Oct 6-9).
- **Proxmox** running on `pve` (192.168.50.143, Proxmox VE 9.2.2), wired to lab router SCT5.
- **Phase C started Oct 13, 2026.** The first architecture VM is built and running:
  - **VM 100 `openplc-docker`** -- the OpenPLC Docker host. Ubuntu 26.04 LTS **desktop**, 2 vCPU (type host), 6 GB RAM, 40 GB disk on the SSD (`local-lvm`). Static IP **192.168.50.10** (gw .1, DNS .1 + 8.8.8.8), hostname `OpenPLC-Docker`, user `plc`. QEMU guest agent + **Docker installed and verified** (`hello-world` OK) on Oct 13. The VM interface is **`ens18`** (this is `PARENT_IF` for the macvlan script).
  - Desktop (not server) edition chosen on purpose, so the OpenPLC/SCADA web UIs can be shown on-screen to a non-technical audience.
- **All 8 OpenPLC containers are running (Oct 14, 2026), each answering HTTP 200 from the laptop, all with a blank program.** The 7 new ones were started with `run_containers.sh`. **Done on the VM, evening of Oct 13:** the OpenPLC image `openplc:v3` is built, the macvlan network `cybercity_lan` exists, and the first container `OpenPLC-BSH-FireAlarm` is running at 192.168.50.112 (web UI reached from the laptop; Blank Program, stopped). **Not yet done:** the ESP32 slave device and `Program.st` in each of the 8 containers. See "Done" and "Do next".
- **SCADA/HMI VM built Oct 14, 2026: VM 101 `scada-hmi`** (the second VM). Ubuntu 24.04.5.1 LTS **desktop** (a different release from VM 100's 26.04 on purpose: 24.04 is the release most Ignition users run), 2 vCPU type `host`, 8 GB RAM, 25 GB disk on `local-lvm`, VirtIO NIC on `vmbr0` with the firewall off, hostname `SCADA-HMI`, user `scada`, install ISO detached. **Still to do on it:** static IP (still DHCP), `qemu-guest-agent`, SSH, internet check, then the SCADA software. SCADA = Ignition (Maker Edition), company license pending.
- **OpenPLC programs drafted, not tested:** `PLC_Logic/` (folder per system).
- pfSense = stretch; VPN = Tailscale first. Target: core system by Oct 31; demo Dec 11.

Full infra record: `Proxmox_Build_Log.md`.

## Storage note (Oct 13)

The Proxmox storage layout was **kept as-is** -- the "erase the 500 GB drive and make it local-lvm" option was considered and rejected (the SSD is faster for VMs). We only enlarged the SSD's VM pool: `local-lvm` went 48.87 GB -> **68.5 GB** (claimed 13.6 GB unallocated + shrank swap 8 GB -> 2 GB). Nothing was erased. The 500 GB `Big_Storage` is unchanged and still holds the 4 ISOs; it is for backups and the idle VMs (Kali, pfSense, twin). The core VMs (OpenPLC, Ignition) live on the fast SSD.

## Waiting on

- Ignition license from Inductive Automation. **If nothing by Oct 21** (checkpoint moved from Oct 14 on Oct 14, 2026), start a fallback (Rapid SCADA, then FUXA) in the SCADA VM and switch later once Ignition is licensed. Confirm Ignition trial terms first.

## Done today (Oct 14, 2026)

- **SCADA/HMI VM built (evening):** VM 101 `scada-hmi`, Ubuntu 24.04.5.1 desktop, specs and what is still open are in the Where-the-project-stands list above and in `Proxmox_Build_Log.md`. To get the ISO onto `pve` the DNS on `pve` was fixed first (it pointed at `192.168.100.1`, now `192.168.50.1` + `8.8.8.8`).

- **Other 7 containers started:** re-copied `Setup/` to the VM (the Oct 13 copy had the old script), checked .111, .113, .114 and .121-.124 were free (all pings timed out), then `cd ~/Setup && sudo ./run_containers.sh`. It skipped BSH-FireAlarm and started the rest, each with its own named volume.
- **Verified:** `docker ps` shows 8 containers Up, and from the laptop all 8 web UIs (`http://192.168.50.<111-114,121-124>:8080`) return HTTP 200.
- **Odd detail:** BSH-FireAlarm showed "Up Less than a second" in `docker ps`, with `restarts=0` and a `StartedAt` a few minutes in the future; its log shows one continuous run since 19:59:38 UTC. Likely a VM clock step after a reboot, not a crash. See `PLC_Logic/Setup/README.md`.

## Done on Oct 13 (evening)

- **Image built:** `openplc:v3`, from a clone of `github.com/thiagoralves/OpenPLC_v3` (`sudo docker build -t openplc:v3 .`, about 10 minutes, Debian trixie base).
- **Smoke test passed:** a test container with `-p 8080:8080` answered on port 8080 (HTTP 302 to the login page); the test container was removed.
- **Access:** SSH from the laptop to the VM works (`ssh plc@192.168.50.10`). `curl` was installed on the VM. `PLC_Logic/Setup/` was copied to `~/Setup` on the VM.
- **Network:** macvlan `cybercity_lan` created with `sudo env PARENT_IF=ens18 ./create_network.sh`. Ubuntu 26.04's `sudo` ignores `-E`, so pass the variable with `sudo env VAR=value ...`.
- **First container:** `OpenPLC-BSH-FireAlarm` at 192.168.50.112, started with a named volume (`openplc_bsh_firealarm:/docker_persistent`), no `--privileged`, no `-p`. .112 was checked free first (ping from the laptop and the VM got no reply). The dashboard loaded from the laptop at `http://192.168.50.112:8080`.
- **Settled:** persistence = a named volume on `/docker_persistent` (the Dockerfile declares it). `--privileged` is not needed: the container runs the web UI without it. Docker commands on the VM need `sudo` until `plc` logs out and back in.

## Do next (order changed Oct 14: containers done, SCADA VM next)

1. **Finish VM 101 `scada-hmi`** (built Oct 14, see above and `Proxmox_Build_Log.md`). In order: (a) ping 192.168.50.30 from the laptop; no reply means it is free (it is outside .10-.24 and .111-.124); (b) set it static in GNOME Settings > Network (netmask 255.255.255.0, gw 192.168.50.1, DNS 192.168.50.1 + 8.8.8.8), then record it in `Piranga_Workflow_v2.md` Section 6 and `New_Architecture_Overview.md` (both still say TBD); (c) `sudo apt update && sudo apt install -y qemu-guest-agent openssh-server`, then enable the agent and check from `pve` that VM 101 shows its IP in Summary; (d) check internet and DNS (ping 8.8.8.8 and a hostname) and that `ssh scada@<ip>` works from the laptop; (e) confirm on Ignition's Downloads / user manual page that it supports Ubuntu 24.04 before installing it; (f) install Ignition when licensed, or Rapid SCADA (then FUXA) as the fallback if nothing arrives by Oct 21; (g) log the final state in `Proxmox_Build_Log.md`.
2. **Configure the containers, starting with BSH-FireAlarm.** Then repeat for the other 7 (each system's `Register_Map.md` has its own sizes). Keep the default OpenPLC login (decided Oct 13: the default password is not being changed). Slave Devices: add `BSH-FireAlarm-ESP32` at 192.168.50.12:502 with Discrete Inputs 0/3, Coils 0/1, Input Registers 0/3, Holding Registers read 0/0 and write 0/0 (see `PLC_Logic/BSH-FireAlarm/Register_Map.md`). Slave ID: try 0, then 1, and record the one that works. Write down the memory addresses OpenPLC prints beside each block; that settles the `%IX100.0` convention. Then upload `PLC_Logic/BSH-FireAlarm/Program.st`, compile, Start PLC, compare Monitoring with the ESP32, and fill in that folder's `Build_Log.md`. (Luis writes some PLC programs himself; for those, leave the rungs for him to build.)
3. **Settle the remaining open questions** in `PLC_Logic/README.md` and `PLC_Logic/Setup/README.md`: slave-device address convention, Modbus slave ID, FH-HVAC comms-loss direction. (Persistence and `--privileged` are settled, see above.)
4. Tailscale subnet router later. Install Ignition when licensed.

## Backups (added Oct 14, 2026)

- **GitHub backup:** this whole folder is pushed to a private repo, `https://github.com/lfoliveira08/CyberCity` (first push Oct 14, 2026, 318 files, history starts there; the PDFs in `References/` are included). The git data lives outside OneDrive at `C:\Users\lfoli\git-data\cybercity.git` (the folder holds only a small `.git` pointer file), so OneDrive sync cannot corrupt it.
- **It is a snapshot, not automatic.** After a work session, in the folder's terminal: `git add .`, `git commit -m "what changed"`, `git push`. To restore on another computer: `git clone https://github.com/lfoliveira08/CyberCity.git`.
- **Keep the repo private.** The firmware `.ino` files and two `SYSTEM_REPORT.md` files contain the lab Wi-Fi name and password, and the commit history carries a personal email address (the GitHub setting that blocks pushing a private email was turned off to allow the push). Clean both before ever making it public. Also remove the book PDFs in `References/` first.
- **Other protection:** OneDrive version history on this folder.
- **Not covered yet:** the Proxmox VMs (OpenPLC Docker VM and the containers' data volumes). Back up VM 100 with Proxmox's built-in backup to `Big_Storage` once the containers are configured, and do the same for the SCADA VM.

## Cleanup pending

- **Detach the install ISO from VM 100** (Proxmox: VM 100 -> Hardware -> CD/DVD Drive -> Edit -> Do not use any media) so a future reboot never boots the installer. It booted from disk fine this time, so low priority.
- The `pve` clock was wrong (system read Apr 2026; hardware clock read 2010). Fixed Oct 13 by manual set + `hwclock --systohc`. NTP auto-sync is blocked by the lab network (UDP 123), so if the clock drifts again, set it by hand.

## Standing rules

- The folder wins over the Claude project copies. Never copy project content back into the folder.
- One home per file (`Firmware/WORKFLOW.md` Section 8). Do not add `_v2` or `_final` to file names.
- Ask before anything that cannot be undone.
- Luis writes some of the PLC programs himself; for those, leave the rungs for him to build.
