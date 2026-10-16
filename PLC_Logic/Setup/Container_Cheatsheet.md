# Container cheat sheet -- the 8 OpenPLC containers

Created Oct 14, 2026. One place for every container's address and the commands to start, check and stop it. The containers are all built from the image `openplc:v3` and attached to the macvlan network `cybercity_lan`. Setup steps are in `README.md` in this folder; the per-system programs are in `PLC_Logic/<System>/`.

## Quick start -- initiate and check all 8 containers

Run on the Docker VM (`ssh plc@192.168.50.10`):

```bash
# initiate (start) all 8 -- containers that are already running are left alone
sudo docker start $(sudo docker ps -aq --filter "name=OpenPLC-")

# check -- expect 8 containers, all Up
sudo docker ps -a --filter "name=OpenPLC-" --format 'table {{.Names}}\t{{.Status}}'

# stop all 8
sudo docker stop $(sudo docker ps -q --filter "name=OpenPLC-")
```

If a container does not exist at all, create it with `cd ~/Setup && sudo ./run_containers.sh` (it skips the ones that exist). Everything else is below: section 3 for all 8, section 4 for one at a time.

## 1. Systems, addresses and containers

| System | What it is | ESP32 IP | Container name | Container IP (OpenPLC) | Web UI (open from the laptop) | Data volume |
|---|---|---|---|---|---|---|
| BSH-Elevator | Elevator | 192.168.50.11 | `OpenPLC-BSH-Elevator` | 192.168.50.111 | http://192.168.50.111:8080 | `openplc_bsh_elevator` |
| BSH-FireAlarm | Fire Alarm | 192.168.50.12 | `OpenPLC-BSH-FireAlarm` | 192.168.50.112 | http://192.168.50.112:8080 | `openplc_bsh_firealarm` |
| BSH-Door | Door/Gate | 192.168.50.13 | `OpenPLC-BSH-Door` | 192.168.50.113 | http://192.168.50.113:8080 | `openplc_bsh_door` |
| BSH-PowerBox | Power Box | 192.168.50.14 | `OpenPLC-BSH-PowerBox` | 192.168.50.114 | http://192.168.50.114:8080 | `openplc_bsh_powerbox` |
| FH-HVAC | HVAC zone unit | 192.168.50.21 | `OpenPLC-FH-HVAC` | 192.168.50.121 | http://192.168.50.121:8080 | `openplc_fh_hvac` |
| FH-FireAlarm | Fire Alarm | 192.168.50.22 | `OpenPLC-FH-FireAlarm` | 192.168.50.122 | http://192.168.50.122:8080 | `openplc_fh_firealarm` |
| FH-Door | Door/Gate | 192.168.50.23 | `OpenPLC-FH-Door` | 192.168.50.123 | http://192.168.50.123:8080 | `openplc_fh_door` |
| FH-PowerBox | Power Box | 192.168.50.24 | `OpenPLC-FH-PowerBox` | 192.168.50.124 | http://192.168.50.124:8080 | `openplc_fh_powerbox` |

- Each container uses Web UI port 8080 and Modbus port 502 on its own IP. Systems are told apart by IP, not by port.
- Default OpenPLC login on every container: `openplc` / `openplc` (kept on purpose, decided Oct 13, 2026).
- The Docker host is the VM `openplc-docker` (192.168.50.10, user `plc`). The SCADA/HMI VM is `scada-hmi` (VM 101, built Oct 14, static IP 192.168.50.30; it hosts no container, so it is not in this table).

## 2. Where to run what

- **Docker commands** run on the Docker VM: from the laptop, `ssh plc@192.168.50.10`, then use the commands below. `sudo` is needed.
- **Browser and network checks** run on the **laptop**. A macvlan container cannot be reached from its own Docker host, so `ping`, `curl` or Firefox on the VM itself will not reach .111-.124.
- **Stopping the container is not the same as stopping the PLC.** The OpenPLC web UI has its own Start PLC / Stop PLC button, which starts or stops the program while the container keeps running. `docker stop` stops the whole container, web UI included.
- Docker may print `Error response from daemon: No such container` if a name is mistyped. Names are case-sensitive.

## 3. All 8 containers at once (on the Docker VM)

```bash
# status of all 8 (name, status, how long)
sudo docker ps -a --filter "name=OpenPLC-" --format 'table {{.Names}}\t{{.Status}}'

# start all 8 (containers that are already running are left alone)
sudo docker start $(sudo docker ps -aq --filter "name=OpenPLC-")

# stop all 8
sudo docker stop $(sudo docker ps -q --filter "name=OpenPLC-")

# restart all 8
sudo docker restart $(sudo docker ps -q --filter "name=OpenPLC-")

# restart counts and start times (restarts above 0 means a crash and auto-restart)
sudo docker inspect -f '{{.Name}} restarts={{.RestartCount}} started={{.State.StartedAt}}' $(sudo docker ps -q --filter "name=OpenPLC-")

# last 20 log lines of every container
for c in $(sudo docker ps --filter "name=OpenPLC-" --format '{{.Names}}'); do echo "== $c"; sudo docker logs --tail 20 $c; done

# create any container that is missing (skips the ones that exist; needs the network cybercity_lan)
cd ~/Setup && sudo ./run_containers.sh
```

Check all 8 web UIs from the **laptop** (PowerShell). Expect `HTTP 200` for each one that is running:

```powershell
111..114 + 121..124 | ForEach-Object { $ip="192.168.50.$_"; try { $r = Invoke-WebRequest "http://${ip}:8080" -UseBasicParsing -TimeoutSec 5; "$ip  HTTP $($r.StatusCode)" } catch { "$ip  FAILED: $($_.Exception.Message)" } }
```

Containers have `--restart unless-stopped`: they start again by themselves when the VM or Docker restarts, unless you stopped them yourself with `docker stop`.

## 4. One container at a time

Run the Docker commands on the Docker VM and the laptop check in PowerShell on the laptop.

### BSH-Elevator (Elevator) -- OpenPLC-BSH-Elevator, 192.168.50.111, polls ESP32 192.168.50.11

```bash
sudo docker start OpenPLC-BSH-Elevator       # initiate
sudo docker ps --filter "name=OpenPLC-BSH-Elevator" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-BSH-Elevator    # check what it is doing
sudo docker restart OpenPLC-BSH-Elevator     # restart
sudo docker stop OpenPLC-BSH-Elevator        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.111:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### BSH-FireAlarm (Fire Alarm) -- OpenPLC-BSH-FireAlarm, 192.168.50.112, polls ESP32 192.168.50.12

```bash
sudo docker start OpenPLC-BSH-FireAlarm       # initiate
sudo docker ps --filter "name=OpenPLC-BSH-FireAlarm" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-BSH-FireAlarm    # check what it is doing
sudo docker restart OpenPLC-BSH-FireAlarm     # restart
sudo docker stop OpenPLC-BSH-FireAlarm        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.112:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### BSH-Door (Door/Gate) -- OpenPLC-BSH-Door, 192.168.50.113, polls ESP32 192.168.50.13

```bash
sudo docker start OpenPLC-BSH-Door       # initiate
sudo docker ps --filter "name=OpenPLC-BSH-Door" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-BSH-Door    # check what it is doing
sudo docker restart OpenPLC-BSH-Door     # restart
sudo docker stop OpenPLC-BSH-Door        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.113:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### BSH-PowerBox (Power Box) -- OpenPLC-BSH-PowerBox, 192.168.50.114, polls ESP32 192.168.50.14

```bash
sudo docker start OpenPLC-BSH-PowerBox       # initiate
sudo docker ps --filter "name=OpenPLC-BSH-PowerBox" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-BSH-PowerBox    # check what it is doing
sudo docker restart OpenPLC-BSH-PowerBox     # restart
sudo docker stop OpenPLC-BSH-PowerBox        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.114:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### FH-HVAC (HVAC zone unit) -- OpenPLC-FH-HVAC, 192.168.50.121, polls ESP32 192.168.50.21

```bash
sudo docker start OpenPLC-FH-HVAC       # initiate
sudo docker ps --filter "name=OpenPLC-FH-HVAC" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-FH-HVAC    # check what it is doing
sudo docker restart OpenPLC-FH-HVAC     # restart
sudo docker stop OpenPLC-FH-HVAC        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.121:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### FH-FireAlarm (Fire Alarm) -- OpenPLC-FH-FireAlarm, 192.168.50.122, polls ESP32 192.168.50.22

```bash
sudo docker start OpenPLC-FH-FireAlarm       # initiate
sudo docker ps --filter "name=OpenPLC-FH-FireAlarm" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-FH-FireAlarm    # check what it is doing
sudo docker restart OpenPLC-FH-FireAlarm     # restart
sudo docker stop OpenPLC-FH-FireAlarm        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.122:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### FH-Door (Door/Gate) -- OpenPLC-FH-Door, 192.168.50.123, polls ESP32 192.168.50.23

```bash
sudo docker start OpenPLC-FH-Door       # initiate
sudo docker ps --filter "name=OpenPLC-FH-Door" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-FH-Door    # check what it is doing
sudo docker restart OpenPLC-FH-Door     # restart
sudo docker stop OpenPLC-FH-Door        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.123:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

### FH-PowerBox (Power Box) -- OpenPLC-FH-PowerBox, 192.168.50.124, polls ESP32 192.168.50.24

```bash
sudo docker start OpenPLC-FH-PowerBox       # initiate
sudo docker ps --filter "name=OpenPLC-FH-PowerBox" --format 'table {{.Names}}\t{{.Status}}'    # check it is Up
sudo docker logs --tail 20 OpenPLC-FH-PowerBox    # check what it is doing
sudo docker restart OpenPLC-FH-PowerBox     # restart
sudo docker stop OpenPLC-FH-PowerBox        # stop
```

```powershell
# check from the laptop (expect HTTP 200)
(Invoke-WebRequest http://192.168.50.124:8080 -UseBasicParsing -TimeoutSec 5).StatusCode
```

## 5. Check the Modbus side (from the laptop)

Port 502 should answer only while the PLC is running inside the container (after Start PLC in the web UI). It will be closed on a container that still has the blank program stopped.

```powershell
# one container (example: BSH-FireAlarm); TcpTestSucceeded : True means the Modbus port answers
Test-NetConnection 192.168.50.112 -Port 502

# all 8
111..114 + 121..124 | ForEach-Object { $ip="192.168.50.$_"; $t = Test-NetConnection $ip -Port 502 -WarningAction SilentlyContinue; "$ip  502 open: $($t.TcpTestSucceeded)" }
```

Check an ESP32 itself the same way (its own Modbus port, for example `Test-NetConnection 192.168.50.12 -Port 502`) to tell a dead ESP32 apart from a dead container.

## 6. Dangerous commands -- they cannot be undone

Only run these on purpose, and ask first.

```bash
# remove ONE container (its data volume stays, so a new container with the same volume gets its programs and settings back)
sudo docker rm -f OpenPLC-BSH-FireAlarm

# delete a data volume = permanently deletes that container's uploaded programs and settings
sudo docker volume rm openplc_bsh_firealarm
```

After removing a container, `cd ~/Setup && sudo ./run_containers.sh` creates it again, reusing the same volume. Keep every program as `Program.st` in its `PLC_Logic/<System>/` folder anyway.
