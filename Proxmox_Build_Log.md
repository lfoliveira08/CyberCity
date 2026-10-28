# Proxmox build log -- infrastructure

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Durable record of the Proxmox / VM infrastructure build. Newest work at the bottom. The folder is the source of truth; this file logs what was actually done on `pve`. Status and "do next" live in `Next_Steps.md`.

## Host: `pve`
- Proxmox VE 9.2.2, 192.168.50.143 (web UI :8006), wired to lab router SCT5, subnet 192.168.50.0/24, gateway 192.168.50.1.
- Hardware: 30 GB RAM; single NIC (a second NIC is needed later for pfSense). Disks: 120 GB SSD (`sdb`, holds Proxmox) + 500 GB HDD (`sda`, storage `Big_Storage`).

## Oct 13, 2026 -- storage, clock, VM 1, Docker

### Storage (kept existing layout; nothing erased)
- Considered and **rejected** the "erase the 500 GB drive and make it local-lvm" plan. The SSD is faster for VMs, so VMs stay on the SSD.
- Enlarged the SSD VM pool only:
  - Gave 13.6 GB unallocated to `local-lvm`: 48.87 -> 62.5 GB (`lvextend -l +100%FREE pve/data`).
  - Shrank swap 8 GB -> 2 GB and gave the freed 6 GB to `local-lvm`: 62.5 -> **68.5 GB** (`swapoff /dev/pve/swap`; `lvreduce --fs ignore -L 2G pve/swap`; `mkswap /dev/pve/swap`; `swapon /dev/pve/swap`; `lvextend -l +100%FREE pve/data`). Swap persists via `/dev/pve/swap` in `/etc/fstab` (by path, not UUID).
- Final SSD layout: `local` (Proxmox root) 37.5 GB, `local-lvm` (VM disks) 68.5 GB, swap 2 GB.
- `Big_Storage` (500 GB HDD) unchanged: holds the 4 ISOs (Kali 2026.2, Ubuntu 26.04 desktop, Win10, Win11); already accepts Disk image + Container content. Use for backups and idle VMs.
- Precaution: the 4 ISOs were also copied to a 58 GB NTFS flash drive, sizes verified byte-for-byte. Since no wipe happened, that flash drive is just a spare backup.

### Clock
- `pve` clock was wrong: system read Apr 17 2026; hardware clock (RTC) read 2010. Time zone correct (America/Chicago). NTP service active but could not sync (lab blocks outbound NTP, UDP 123).
- Fixed: `timedatectl set-ntp false`; `timedatectl set-time "<current time>"`; `hwclock --systohc`. If it drifts again, set by hand (NTP will not sync on this network).

### VM 100 `openplc-docker` (OpenPLC Docker host)
- Created via the Create VM wizard. Specs: VMID 100, name `openplc-docker`; Ubuntu 26.04.1 desktop ISO (from `Big_Storage`); QEMU agent on; SeaBIOS / i440fx default; SCSI disk **40 GB on `local-lvm`** (discard on, iothread on); CPU 1 socket / 2 cores / type `host`; **6144 MB** RAM; net VirtIO on `vmbr0` with **firewall off** (needed for macvlan); start-at-boot not set yet.
- Ubuntu 26.04 LTS **desktop** installed (desktop over server, so web UIs can be shown to a non-technical audience). No disk encryption. User **`plc`**, hostname **`OpenPLC-Docker`** (password known to Luis, not recorded here).
- Inside the VM: `qemu-guest-agent` installed + enabled; **static IP 192.168.50.10** set via GNOME Settings (netmask 255.255.255.0, gw 192.168.50.1, DNS 192.168.50.1 + 8.8.8.8); interface name **`ens18`**. Internet + DNS verified (ping 8.8.8.8 and google.com OK).
- **Docker installed** (`apt install docker.io`), enabled at boot, user `plc` added to the `docker` group, verified with `sudo docker run hello-world` (OK). Docker Hub reachable.

## Oct 13, 2026 (evening) -- OpenPLC image, macvlan network, first container

Run on VM 100 (`OpenPLC-Docker`, 192.168.50.10) over SSH from the laptop. `sudo` is needed for `docker` until `plc` logs out and back in.

- **SSH:** the laptop reaches the VM with `ssh plc@192.168.50.10`. `curl` was installed (`sudo apt install -y curl`). `PLC_Logic/Setup/` was copied to `~/Setup` (scp from the laptop).
- **Image:** `git clone https://github.com/thiagoralves/OpenPLC_v3.git && cd OpenPLC_v3 && sudo docker build -t openplc:v3 .` (about 10 minutes; Debian trixie base; the Dockerfile runs OpenPLC's `install.sh docker`). Result: `openplc:v3`. The repo's README says v3 is end-of-life, replaced by OpenPLC Runtime v4; staying on v3 to match the drafted programs.
- **Smoke test:** `sudo docker run -d --name openplc-test -p 8080:8080 openplc:v3` -> `curl -sI http://localhost:8080` returned HTTP 302 (login redirect). Test container removed.
- **macvlan network:** `cd ~/Setup && sudo env PARENT_IF=ens18 ./create_network.sh` created `cybercity_lan` (192.168.50.0/24, gw 192.168.50.1, parent `ens18`). `sudo -E` does not work on Ubuntu 26.04 (its sudo ignores `-E`), hence `sudo env`.
- **First container:** .112 was checked free (no reply to ping from the laptop or the VM), then `sudo docker run -d --name OpenPLC-BSH-FireAlarm --hostname OpenPLC-BSH-FireAlarm --network cybercity_lan --ip 192.168.50.112 --restart unless-stopped -v openplc_bsh_firealarm:/docker_persistent openplc:v3`. Status Up; the dashboard loaded from the laptop at `http://192.168.50.112:8080` (Blank Program, Stopped). No `--privileged`, no `-p` (macvlan gives the container its own IP).
- **Persistence:** the OpenPLC_v3 Dockerfile keeps its database, uploaded programs and settings in `/docker_persistent` (declared as a volume), so each container gets a named volume.
- Two `hello-world` containers from the Docker check are still listed in `docker ps -a` (harmless; `sudo docker container prune` removes them).

## Oct 14, 2026 -- the other 7 OpenPLC containers

- Re-copied `PLC_Logic/Setup/` to `~/Setup` on VM 100 (`scp -r` from the laptop); the Oct 13 copy still had the old `run_containers.sh`.
- Pinged .111, .113, .114, .121, .122, .123, .124 from the laptop: all timed out (free).
- `cd ~/Setup && chmod +x *.sh && sudo ./run_containers.sh`: skipped `OpenPLC-BSH-FireAlarm` (exists), started `OpenPLC-FH-FireAlarm` (.122), `-BSH-Elevator` (.111), `-BSH-Door` (.113), `-FH-Door` (.123), `-BSH-PowerBox` (.114), `-FH-PowerBox` (.124), `-FH-HVAC` (.121), each with its own named volume (`openplc_<name>`) on `/docker_persistent`, no `--privileged`.
- Verified: `docker ps` shows 8 containers Up (restarts=0 on all). From the laptop, `Invoke-WebRequest http://192.168.50.<ip>:8080` returned HTTP 200 for all 8.
- Detail: `docker ps` listed `OpenPLC-BSH-FireAlarm` as "Up Less than a second" with a `StartedAt` a few minutes in the future, while its log shows a single continuous run since 19:59:38 UTC and no crash. Likely a VM clock step after a reboot. Worth a `date -u` / `uptime -s` check on the VM; the Wireshark evidence in `Security_Capture/` depends on accurate clocks.

### Not done yet (next session)
- Build the SCADA/HMI VM (VM 2) next. (Done later on Oct 14: VM 101, see the next section.)
- Then configure each container in its web UI: add its ESP32 slave device, upload its `Program.st`, start the PLC, verify (start with BSH-FireAlarm, see `PLC_Logic/BSH-FireAlarm/Build_Log.md`). (Done on all 8 containers Oct 19-21, 2026; per-node results in `PLC_Logic/<System>/Build_Log.md`.)
- Detach the install ISO from VM 100 (cleanup).

## Oct 14, 2026 (evening) -- pve DNS fix, Ubuntu 24.04 ISO, VM 101 `scada-hmi`

### pve DNS fix
- `pve` DNS (Node > System > DNS) pointed at `192.168.100.1`, which is not on this lab network (192.168.50.0/24), so "Download from URL" failed with "Temporary failure in name resolution". Changed DNS server 1 to `192.168.50.1` and added `8.8.8.8` (search domain `CyberCampus` unchanged). Only `pve` itself is affected by this setting; the VMs and the Modbus traffic use their own settings and IP addresses.

### Ubuntu 24.04 ISO
- Downloaded to `Big_Storage` with Download from URL after the DNS fix: `ubuntu-24.04.5.1-desktop-amd64.iso` (5.82 GiB, from `https://releases.ubuntu.com/24.04/`). `Big_Storage` now holds 5 ISOs (the 4 listed on Oct 13 plus this one).
- Why 24.04 and not the 26.04.1 ISO already there: Ignition's docs name Ubuntu but no release, and 24.04 LTS is the Ubuntu release most Ignition users run, so it removes one unknown. Windows Server was considered and not chosen (no license, 50-60 GB disk would not fit on `local-lvm`, extra VirtIO driver setup, nothing in the design needs OPC-DA/DCOM). The Win10/Win11 ISOs were not used (Win10 22H2 is out of support, Win11 needs about 64 GB and a TPM).

### VM 101 `scada-hmi` (SCADA/HMI host, second architecture VM)
- Created with the Create VM wizard: VMID 101, name `scada-hmi`; OS type Linux (`7.x - 2.6 Kernel` label on this Proxmox); ISO `ubuntu-24.04.5.1-desktop-amd64.iso`; QEMU agent on (agent=1); SCSI disk **25 GB on `local-lvm`** (iothread on, `virtio-scsi-single`); CPU 1 socket / 2 cores / type `host`; **8192 MB** RAM; net VirtIO on `vmbr0`, **firewall off**.
- Ubuntu 24.04.5.1 LTS **desktop** installed with Erase disk (no LVM, no encryption), no third-party software. Hostname **`SCADA-HMI`**, user **`scada-hmi`** (password known to Luis, not recorded here). Install finished and the VM rebooted from its disk.
- Install ISO **detached** (CD/DVD drive set to "Do not use any media"), so it will not boot the installer again.
- **Oct 15:** installed `qemu-guest-agent` and `openssh-server` with apt, enabled `ssh`, started the agent. The VM first came up on **DHCP** (`ens18` = 192.168.50.237/24). Later the same day Luis set the **static IP 192.168.50.30** (outside .10-.24 and .111-.124; gw 192.168.50.1, DNS 192.168.50.1 + 8.8.8.8); `ip -4 addr` now shows 192.168.50.30/24 (as reported by Luis).
- Internet and DNS from the VM verified Oct 15 (ping 8.8.8.8 and google.com both OK).
- SSH from the laptop works (`ssh scada-hmi@192.168.50.237`, confirmed Oct 15); the password was reset to a letters-and-numbers one to avoid keyboard-layout problems between the Proxmox console and the laptop (not recorded here).
- Ignition OS-support check (Oct 15): Inductive's Ignition 8.3 docs name no Linux distribution or Ubuntu release. They say Ignition "works with any major operating system, including Windows, MacOS, Linux, iOS, and Android" and give a minimum of a dual-core processor, 4 GB RAM and 10 GB free disk (the Windows install guide's figures). VM 101 (2 vCPU, 8 GB, 25 GB) meets that. Ubuntu 24.04 is therefore not on a published support list for the standalone Linux installer. Official evidence: the Ignition 8.3.0 release notes (Sep 16, 2025) say Cloud Edition 8.3.0 "is now based on Ubuntu Server 24.04 LTS" and the Docker image moved to Ubuntu 24.04 LTS; the bundled Java is 17.0.14. That shows 8.3 is built on 24.04 but is not a written support statement for desktop installs. Re-check at install time.
- Guest agent confirmed Oct 15: VM 101 Summary in Proxmox shows IPs 192.168.50.30 (status running, 2 vCPU, 8 GB, 25 GB boot disk; about 2.5 GiB memory in use at idle).
- **Not done yet on VM 101:** the SCADA software itself (Ignition license pending; fallback Rapid SCADA / FUXA), start-at-boot, and a Proxmox backup.
- Disk budget: `local-lvm` is 68.5 GB; VM 100 uses 40 GB and VM 101 25 GB, so about 3 GB is left unallocated on the SSD pool (thin provisioning, but do not plan more VM disks there).

## Oct 19, 2026 (night) -- VM 102 `Kali` (attacker / Modbus-client VM)

### Why and what
- Luis decided on Oct 19 that the Modbus-client tests against the OpenPLC containers, and later the Section 11 attack scenarios, run from a Kali VM instead of the laptop. Named in the plan as "Further VMs (optional): attacker/analysis VM".
- Built as the third VM on `pve`. It is a lab tool for the closed testbed subnet only (`Security_Capture/README.md` rule 1).

### VM 102 specs (from the Create VM Confirm tab)
- VMID 102, name **`Kali`** in Proxmox; OS type Linux (`7.x - 2.6 Kernel`); ISO `kali-linux-2026.2-installer-everything-amd64.iso` (already on `Big_Storage`); QEMU agent on (agent=1); CPU 1 socket / 2 cores / type `host`; **4096 MB** RAM; SCSI disk **40 GB on `Big_Storage`** (qcow2, iothread on; chosen over `local-lvm` because `local-lvm` has about 3 GB unallocated); net VirtIO on `vmbr0`, no `firewall=1` (firewall off); started after creation.
- Installed from the graphical installer: English, Central time, Guided - use entire disk, one partition (about 40.7 GB ext4 on `/` plus 2.2 GB swap), default software selection (Xfce desktop, top10 and default tool sets), GRUB on `/dev/sda`. No disk encryption, no LVM. Hostname `kali`, user `kali`. The password is a simple one chosen by Luis and is not recorded here.
- Network: set to a static IP **192.168.50.40/24** with `nmcli` on connection "Wired connection 1" (gw 192.168.50.1, DNS 192.168.50.1 + 8.8.8.8). Verified on screen: `ip -4 addr` shows `192.168.50.40/24` on `eth0`, and `ping google.com` works (2 of 2, 0% loss).

### Confirmed later the same night
- **Guest agent works:** Proxmox's Network Information panel for VM 102 shows `eth0` = 192.168.50.40/24 (MAC `bc:24:11:f6:36:6a`); that panel is filled in by the guest agent.
- **Install ISO detached:** Hardware > CD/DVD Drive (ide2) shows `none` and the Edit dialog has "Do not use any media" selected. (The Kali desktop still shows an empty `cdrom0` icon, which is only the empty virtual drive.)

- **SSH from the laptop works** (confirmed Oct 19, 23:23 local): `ssh kali@192.168.50.40` accepted the host key and the password and showed Kali GNU/Linux, kernel 6.19.14+kali-amd64, hostname `kali`.

### Not verified / not done yet
- Change the weak password before SSH is relied on, since the lab router is shared.
- Where Wireshark runs: a VM on `vmbr0` sees only its own traffic, not traffic between other hosts; see `Security_Capture/Capture_Setup.md`.
- Backup of VMs 100-102 to `Big_Storage` (still open from `Next_Steps.md`).

## Oct 22, 2026 -- Ignition on VM 101 `scada-hmi`

### Done
- **Snapshot first:** `before-ignition` taken on VM 101 (16:24, task OK). Proxmox warned that the thin volumes total 90 GiB, more than the `pve/data` pool. `local-lvm` Usage read at 17:17: 44.30% (32.58 of 73.55 GB), fine.
- **Installer:** `ignition-8.3.10-linux-64-installer.run` (2.0 GB), downloaded in Firefox on the VM. Run with `sudo ./ignition-8.3.10-linux-64-installer.run --accept -- unattended=text`; installer options go after `--` (`--mode text` is rejected). It asked which user to install for; answered `scada-hmi`. Log at `/home/scada-hmi/.ignition/install/install.log`. The Gateway started by itself on port 8088.
- **Commissioning wizard** (from the laptop at `http://192.168.50.30:8088`): edition Ignition (Standard 8.3.10), admin account `AdminLuis` (password not recorded here), ports 8088 / 8043 / 8060 (defaults), Quick Start declined. Gateway name `Ignition-SCADA-HMI`.
- **License:** the Capstone license was activated from the Licensing page with the license key plus the activation token from the license document (the token is a secret; keep it out of the repo and the Claude project). Status ACTIVE, Platform expiry 2027-01-31, "will automatically renew" about every 72 hours (a rolling lease; VM 101 needs internet). One activation only.
- **Modules:** 29 running, 24 licensed. Enterprise Administration and Voice Notification were on Trial and kept the "License Incomplete" bar up; both were disabled and the Gateway restarted, and the bar went away. Modbus Driver v2, Perspective, Vision, Historian Core, SQL Historian, Alarm Notification, SMS Notification, Reporting and SQL Bridge show Activated.
- **Memory check:** `free -h` on VM 101: 7.8 GiB total, 3.1 GiB used, 4.7 GiB available, swap unused (Proxmox's 93% counts cache).
- **Designer:** Designer Launcher installed on the laptop (for me only), Gateway added by URL, Designer opened, empty project `CyberCity` created.
- **Snapshot after:** `ignition-licensed` taken by Luis (17:05, task OK).

### Not done yet
- First Modbus device (Connections > Devices > Modbus TCP, 192.168.50.112:502 for BSH-FireAlarm), tags, Perspective screens, cascade rules.
- Unused drivers (Siemens, Omron, DNP3, BACnet, Allen-Bradley and others) still enabled; disable them later as hardening.
- Ask Inductive whether the lease renewal needs internet and whether an offline option exists.
- Export a project backup and a Gateway backup before the license ends 2027-01-31.

## Oct 26, 2026 -- VM 100 `openplc-docker` paused with io-error (thin pool full)

### What broke
- VM 100 showed a yellow warning triangle in the tree, a black Console, and on the Summary tab Status `running (io-error)` with IPs `Guest Agent not running`. VMs 101 and 102 were normal.

### Cause (from `dmesg -T` on pve)
- `local-lvm` thin pool `pve/data` is 68.5 GiB and the volume group is fully allocated (`pvs`: `/dev/sdb3`, PFree 0). It was extended on Oct 13 (about 49 GiB to 68.5 GiB) and cannot grow again.
- The disks on it are over-provisioned: vm-100-disk-0 (40 GiB), vm-101-disk-0 (25 GiB) and two 25 GiB snapshots of VM 101 (`before_ignition`, `ignition_licensed`), about 115 GiB of virtual size on a 68.5 GiB pool. Proxmox already warned about this on Oct 22.
- A teammate logged into Proxmox, started a clone of a VM (reported by Luis; the source VM and the target storage were not checked, presumably a full clone of VM 100 to the default `local-lvm`) and then deleted the clone. VM 100's disk was about 77% used (about 30 GiB of data), more than the free space in the pool.
- Kernel log: Oct 26 14:07:58 pool reached low water mark and went to out-of-data-space (queue IO); 14:08:58 out-of-data-space (error IO); 14:09:00 back to write mode (the clone had been deleted and the space freed). On the first failed write QEMU paused VM 100 (`io-error`); it stays paused until resumed, even after space is free.
- Not the cause: a physical disk fault. No SATA or I/O error lines in `dmesg`. (Unrelated and minor: `tg3 nic0 transmit timed out, resetting` on Oct 20-22.)

### Checks run (after recovery)
- `pvesm status`: Big_Storage 15.39% (71 of 458 GiB used), local 11.24%, local-lvm 68.47%.
- `lvs`: pool `data` Data% 68.47, Meta% 3.32; vm-100-disk-0 76.72%; vm-101-disk-0 56.48%; no leftover clone volume.

### Fix
- `qm resume 100`; `qm status 100 --verbose` then showed `qmpstatus: running`.
- `docker ps` in VM 100: all 8 OpenPLC containers `Up 11 days` (HVAC, FH-PowerBox, BSH-PowerBox, FH-Door, BSH-Door, BSH-Elevator, FH-FireAlarm, BSH-FireAlarm). The VM was not rebooted and no container restarted.

### Not verified / not done yet
- Ignition on VM 101 has not been checked for healthy Modbus connections after the pause.
- VM 100's disk is still on `local-lvm`. Planned: `qm config 100 | grep -E "scsi|virtio|sata|ide"` to get the disk name, then `qm move-disk 100 scsi0 Big_Storage --format qcow2 --delete 1`.
- Decide whether to keep the two VM 101 snapshots (they grow as VM 101 changes).
- Enable Discard on the VM disks and run `fstrim -av` in the guests.
- Rule for the team: when cloning, set Target Storage to `Big_Storage`, never `local-lvm`. To keep a copy, use `vzdump <vmid> --storage Big_Storage --mode snapshot`.
- Watch `Data%` of `data` in `lvs`; keep it under about 80%.
