# Setup -- OpenPLC containers on the Proxmox Docker VM

**Status (Oct 14, 2026):** the Docker host, the image, the macvlan network and all 8 containers are done and verified (all 8 web UIs answered HTTP 200 from the laptop). Steps 1-4 were run on Oct 13; the first container was started by hand that day, and `run_containers.sh` started the other 7 on Oct 14 (it skipped the existing BSH-FireAlarm). Infrastructure record: `Proxmox_Build_Log.md`.

Addresses and the start / check / stop commands for all 8 containers: `Container_Cheatsheet.md` (this folder).

Plan (from `Piranga_Workflow_v2.md` Sections 6 and 10): 8 OpenPLC containers, each on its own macvlan IP (`openplc_nodes.csv`), inside the OpenPLC Docker VM (`openplc-docker`, 192.168.50.10, interface `ens18`) on the Proxmox server. Each container uses the default Web UI port 8080 and Modbus port 502.

## Before you start

- The VM's NIC must be on a Proxmox bridge (`vmbr0`) with the Proxmox firewall off for that NIC (done for VM 100).
- Ping each container IP from the laptop first; the router's DHCP can hand the same address to another device. "Destination host unreachable" from your own address means nobody answered, so the IP is free.
- A macvlan container cannot be reached from its own Docker host. Open the Web UI and test Modbus from the laptop or another VM.
- Use `sudo` for `docker` commands until `plc` logs out and back in (group membership). Ubuntu 26.04's `sudo` ignores `-E`; pass variables with `sudo env VAR=value ...`.

## Steps

1. Docker installed in the VM (done Oct 13).
2. **Build the image** (done Oct 13): `git clone https://github.com/thiagoralves/OpenPLC_v3.git && cd OpenPLC_v3 && sudo docker build -t openplc:v3 .` (about 10 minutes). The repo README does not name an official published image, so the image is built locally. The repo says v3 is end-of-life (replaced by v4); this project stays on v3 to match the drafted programs.
3. **Smoke test** (done Oct 13): `sudo docker run -d --name openplc-test -p 8080:8080 openplc:v3`, check `http://localhost:8080` answers (HTTP 302 to the login page), then `sudo docker rm -f openplc-test`.
4. **Create the network** (done Oct 13): copy this folder to `~/Setup` on the VM (scp from the laptop), then `cd ~/Setup && sudo env PARENT_IF=ens18 ./create_network.sh`. Check with `sudo docker network ls` (`cybercity_lan`, driver macvlan).
5. **Start the containers.** The first one (BSH-FireAlarm) was started by hand on Oct 13:
   `sudo docker run -d --name OpenPLC-BSH-FireAlarm --hostname OpenPLC-BSH-FireAlarm --network cybercity_lan --ip 192.168.50.112 --restart unless-stopped -v openplc_bsh_firealarm:/docker_persistent openplc:v3`
   For the rest (done Oct 14): `cd ~/Setup && sudo ./run_containers.sh` (reads `openplc_nodes.csv`, skips containers that already exist, one named volume each).
6. Open `http://<container_ip>:8080` from the laptop and log in. The container starts with the OpenPLC default login. Decision Oct 13, 2026: the default login is kept, not changed.
7. Per system: follow its `Build_Log.md` and `Register_Map.md`.

## Settled (Oct 13, 2026)

- **Persistence:** the OpenPLC_v3 Dockerfile keeps its database, uploaded programs and settings in `/docker_persistent`, declared as a volume. Each container gets its own named volume there (`openplc_<name>`), so a container rebuild does not lose them. Keep every program as `Program.st` in its system folder anyway.
- **`--privileged`:** not needed. The first container ran and served the web UI without it. Add it back only if a container fails because of it.

## Notes from the Oct 14 run

- Re-copy `Setup/` to the VM (`scp -r` from the laptop) whenever a script here changes; the VM keeps its own copy in `~/Setup`.
- Check from the laptop with `Invoke-WebRequest http://<ip>:8080` for each container IP (expect HTTP 200).
- `docker ps` showed `OpenPLC-BSH-FireAlarm` as "Up Less than a second" while the others said "Up about a minute". `docker inspect` showed `restarts=0` and a `StartedAt` a few minutes in the future, and its log shows one continuous run since 19:59:38 UTC with no crash. The likely cause is a VM clock step after a reboot, not a crash loop. Check `date -u` and `uptime -s` on the VM if it shows up again.

## Still open

- ~~Slave-device address convention (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) and the Modbus slave ID.~~ SETTLED Oct 15 on BSH-FireAlarm's container: convention confirmed ("attached to address 100 onward"); slave ID 0 entered (confirm by live polling). See `PLC_Logic/BSH-FireAlarm/Build_Log.md`.
- FH-HVAC comms-loss direction for `HVAC_ENABLE` (see `PLC_Logic/README.md`).
