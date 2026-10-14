# Proxmox build log -- infrastructure

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
- Build the SCADA/HMI VM (VM 2) next.
- Then configure each container in its web UI: add its ESP32 slave device, upload its `Program.st`, start the PLC, verify (start with BSH-FireAlarm, see `PLC_Logic/BSH-FireAlarm/Build_Log.md`).
- Detach the install ISO from VM 100 (cleanup).
