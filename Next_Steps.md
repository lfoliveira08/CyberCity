# Next steps -- handoff note

Updated Oct 14, 2026; Ignition license status updated Oct 19, 2026. Read `00_START_HERE.md` first, then this file. What happened day by day (decisions, lessons, open items) is in `Logs/` from Oct 15, 2026 on; start with `Logs/INDEX.md`. If this file and an older doc disagree, this file and `Piranga_Workflow_v2.md` Section 10 are newer.

## Where the project stands

- All 8 ESP32 nodes: Phase B complete, bench-confirmed (Oct 6-9).
- **Proxmox** running on `pve` (192.168.50.143, Proxmox VE 9.2.2), wired to lab router SCT5.
- **Phase C started Oct 13, 2026.** The first architecture VM is built and running:
  - **VM 100 `openplc-docker`** -- the OpenPLC Docker host. Ubuntu 26.04 LTS **desktop**, 2 vCPU (type host), 6 GB RAM, 40 GB disk on the SSD (`local-lvm`). Static IP **192.168.50.10** (gw .1, DNS .1 + 8.8.8.8), hostname `OpenPLC-Docker`, user `plc`. QEMU guest agent + **Docker installed and verified** (`hello-world` OK) on Oct 13. The VM interface is **`ens18`** (this is `PARENT_IF` for the macvlan script).
  - Desktop (not server) edition chosen on purpose, so the OpenPLC/SCADA web UIs can be shown on-screen to a non-technical audience.
- **All 8 OpenPLC containers are running (Oct 14, 2026), each answering HTTP 200 from the laptop, all with a blank program.** The 7 new ones were started with `run_containers.sh`. **Done on the VM, evening of Oct 13:** the OpenPLC image `openplc:v3` is built, the macvlan network `cybercity_lan` exists, and the first container `OpenPLC-BSH-FireAlarm` is running at 192.168.50.112 (web UI reached from the laptop; Blank Program, stopped). **Not yet done:** the ESP32 slave device and `Program.st` in each of the 8 containers. See "Done" and "Do next".
- **SCADA/HMI VM built Oct 14, 2026: VM 101 `scada-hmi`** (the second VM). Ubuntu 24.04.5.1 LTS **desktop** (a different release from VM 100's 26.04 on purpose: 24.04 is the release most Ignition users run), 2 vCPU type `host`, 8 GB RAM, 25 GB disk on `local-lvm`, VirtIO NIC on `vmbr0` with the firewall off, hostname `SCADA-HMI`, user `scada-hmi`, install ISO detached, `qemu-guest-agent` and `openssh-server` installed Oct 15. Static IP **192.168.50.30** set Oct 15. **Still to do on it:** the SCADA software (guest agent confirmed in Proxmox Oct 15) (internet and DNS from the VM and SSH from the laptop were verified Oct 15). SCADA = Ignition (Maker Edition), company license pending.
- **Kali attacker/Modbus-client VM built Oct 19, 2026: VM 102 `Kali`.** Kali 2026.2 (Xfce, default tools), 2 vCPU type `host`, 4 GB RAM, 40 GB disk on `Big_Storage` (not `local-lvm`, which has about 3 GB left), VirtIO NIC on `vmbr0` with the firewall off, hostname `kali`, user `kali`, static IP **192.168.50.40** (seen on screen). Guest agent, ISO detach and SSH from the laptop (`ssh kali@192.168.50.40`) confirmed Oct 19. The folder `Kali/` was created Oct 20 for its attack scripts and reports (not in use yet). **Still to do:** change the simple password. Details in `Proxmox_Build_Log.md` and `Logs/2026-10-19.md`.
- **OpenPLC programs drafted, not tested:** `PLC_Logic/` (folder per system).
- pfSense = stretch; VPN = Tailscale first. Target: core system by Oct 31; demo Dec 11.

Full infra record: `Proxmox_Build_Log.md`.

## Storage note (Oct 13)

The Proxmox storage layout was **kept as-is** -- the "erase the 500 GB drive and make it local-lvm" option was considered and rejected (the SSD is faster for VMs). We only enlarged the SSD's VM pool: `local-lvm` went 48.87 GB -> **68.5 GB** (claimed 13.6 GB unallocated + shrank swap 8 GB -> 2 GB). Nothing was erased. The 500 GB `Big_Storage` is unchanged and still holds the 4 ISOs; it is for backups and the idle VMs (Kali, pfSense, twin). The core VMs (OpenPLC, Ignition) live on the fast SSD.

## Waiting on

- Nothing on Ignition any more: **the license was confirmed Oct 19, 2026.** The license type, terms and expiry still need to be recorded (see `Logs/2026-10-19.md`). The fallbacks (Rapid SCADA, then FUXA) are not needed unless the license is lost. Next: install the Ignition gateway in VM 101 `scada-hmi`.

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

1. **Finish VM 101 `scada-hmi`** (built Oct 14, static IP 192.168.50.30 set Oct 15, SSH and internet verified; see above, `Proxmox_Build_Log.md` and the Section 14 entry in `Piranga_Workflow_v2.md`). In order: (a) done Oct 15: Proxmox VM 101 Summary shows IP 192.168.50.30, so the guest agent works; (b) Ignition OS-support check done Oct 15: Inductive publishes no per-distro or per-version Ubuntu list; its 8.3 docs say only "works with any major operating system, including Windows, MacOS, Linux" and a hardware minimum of dual-core, 4 GB RAM, 10 GB free disk (VM 101 has 2 vCPU, 8 GB, 25 GB), so Ubuntu 24.04 is not on any published support list for the standalone Linux installer. Official evidence found Oct 15: the Ignition 8.3.0 release notes (Sep 16, 2025) say Ignition Cloud Edition 8.3.0 "is now based on Ubuntu Server 24.04 LTS", the Docker image moved to Ubuntu 24.04 LTS, and the bundled Java is 17.0.14. That shows Inductive builds and tests 8.3 on 24.04 but is not a support statement for desktop installs; ask Inductive support if a written answer is needed, re-check on install day and keep the Rapid SCADA/FUXA fallback; (c) install Ignition when licensed, or Rapid SCADA (then FUXA) as the fallback if nothing arrives by Oct 21; (d) log the final state in `Proxmox_Build_Log.md`.
2. **BSH-FireAlarm container: DONE and verified on real hardware (Oct 19).** Program uploaded and running, slave ID 0 proven by HEARTBEAT, and all three functional tests passed (remote command, pull station, clear). Remaining for this node: the Modbus-client read of the `PUB_*` addresses was done from Kali on Oct 20 (heartbeat rose 101 -> 131 across reads), and the gas baseline check. Details in `PLC_Logic/BSH-FireAlarm/Build_Log.md`. The paragraph below is the Oct 15 history for this item. On `OpenPLC-BSH-FireAlarm` (192.168.50.112): Modbus server on (502), Hardware Layer = Blank, S7 + EtherNet/IP servers turned OFF (both on by default -- do this on all 8). Slave device `BSH-FireAlarm-ESP32` added (192.168.50.12:502, Discrete Inputs 0/3, Coils 0/1, Input Registers 0/3, Holding 0/0), **slave ID 0** (accepted, not yet proven by live polling). OpenPLC confirmed the address block live: DI `%IX100.0`-`.2`, coil `%QX100.0`, IR `%IW100`-`102` -- the `%IX100.x` convention is now VERIFIED. All 8 rungs were built in the OpenPLC Editor (v4 app) and verified identical to `PLC_Logic/BSH-FireAlarm/Program.st`. **Remaining for BSH-FireAlarm:** v3 web UI -> Programs -> upload `PLC_Logic/BSH-FireAlarm/Program.st` -> compile -> Start PLC -> Monitoring vs the live ESP32 (watch `%IW101` HEARTBEAT tick once/sec to prove slave ID 0). Note: the v4 Editor does not export a loose `.st` for the v3 web UI, so the hand-written `Program.st` is the deploy artifact. Then repeat for the other 7 (each system's `Register_Map.md` has its own sizes). Keep the default OpenPLC login. (Luis writes the PLC programs himself; leave the rungs for him.)
2b. **BSH-Door container: DONE and verified on real hardware (Oct 20).** Program uploaded and running, slave ID 0 proven, address block verified, Kali read test passed (heartbeat 599 -> 604), and the functional tests passed (remote open with no card, valid card, close, unknown card denied, short pulse, ESP32 unplugged and replugged). **Decided Oct 20:** option A (R1/R2 stay as built; SCADA sends a pulse, because a `SCADA_DOOR_OPEN_CMD` held at 1 keeps the gate open and disables the card reader). **Open:** (a) the RFID reader stopped for a while after that test and recovered with no known cause (left as is; watch for a repeat); (b) Polling Period and Timeout read off the Settings page: 100 ms and 1000 ms (closed Oct 20); (c) the WiFi-drop case (watchdog and `RECONNECT_COUNT`) was not tested, only a power unplug. Details in `PLC_Logic/BSH-Door/Build_Log.md`. Next PLC in build order: BSH-PowerBox (see `Firmware/WORKFLOW.md`); the FH nodes after that.
2c. **BSH-Elevator container: DONE and verified on real hardware (Oct 20).** Luis built the 9 rungs in the Editor (task `T#20ms`, his setting); `Program.st` regenerated, uploaded and compiled (the first compile failed on my `Program.st`: located and unlocated variables need separate `VAR` blocks, now fixed), slave ID 0 proven, Kali read test passed (heartbeat 84 -> 90), and the three functional tests were reported as worked (short press of floor 2; held floor 3 bit did not stop the local floor 1 button; lockout on, nothing moved). **Decided Oct 20:** the call-floor rungs use a 300 ms `TP` pulse (a held SCADA bit does not pin the car), the lockout stays a plain level copy. **Open:** (a) ESP32 timeouts right after the first start cleared by an ESP32 reboot, cause unknown; (b) a motor lead had come off and stopped the car (repaired; add strain relief); (c) Polling Period, Timeout and Hardware not read back for this container; (d) direct-write, WiFi-drop and serial-comparison checks not done. Details in `PLC_Logic/BSH-Elevator/Build_Log.md`.
3. **Settle the remaining open questions** in `PLC_Logic/README.md` and `PLC_Logic/Setup/README.md`: ~~slave-device address convention~~ (SETTLED Oct 15: `%IX100.x`/`%QX100.x`/`%IW100`/`%QW100`, confirmed on BSH-FireAlarm's container), ~~Modbus slave ID~~ (Oct 15: ID 0 entered on BSH-FireAlarm, confirm by live polling), FH-HVAC comms-loss direction. (Persistence and `--privileged` are settled, see above.)
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

- **After every PLC logic build, run the Modbus read test from the Kali VM** (decided Oct 20, 2026): port 502 open, read the node's published addresses twice a few seconds apart, and check that the heartbeat moves. Record the result in that system's `Build_Log.md`.
- `Kali/` (attack scripts and reports) exists but is not used until the attack-scenario session.
- The folder wins over the Claude project copies. Never copy project content back into the folder.
- One home per file (`Firmware/WORKFLOW.md` Section 8). Do not add `_v2` or `_final` to file names.
- Ask before anything that cannot be undone.
- Luis writes some of the PLC programs himself; for those, leave the rungs for him to build.
