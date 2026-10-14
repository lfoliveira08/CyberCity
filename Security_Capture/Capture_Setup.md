# Capture setup -- Wireshark on the Linux machine

Fill in the blanks marked `[ ]` once; they were not recorded when this file was
written (Oct 14, 2026).

- Linux machine: `[ ]` (which VM or PC), IP `[ ]`
- Capture interface: `[ ]` (find it with `ip -br link`)
- Wireshark version: `[ ]`

## 1. What the machine can see (read this first)

The testbed is a switched LAN (lab router SCT5, subnet 192.168.50.0/24). A
normal NIC only sees traffic addressed to it, plus broadcasts. That decides
what each placement proves:

| Where the capture runs | Sees | Does not see |
|---|---|---|
| The attacking Linux machine itself | Its own traffic to the target: scans, Modbus writes, the target's replies, ARP | The other conversations: SCADA polling an ESP32, an OpenPLC container polling its ESP32 |
| A mirror (SPAN) port on the switch/router, if SCT5 supports one | Everything on the LAN: the attack, the PLC<->ESP32 polling, SCADA<->PLC | n/a (best option) |
| On the OpenPLC Docker VM (`192.168.50.10`, interface `ens18`) with `tcpdump` | All traffic of the 8 containers, i.e. every PLC<->ESP32 and SCADA<->PLC conversation, plus attacks aimed at a container | Traffic that never touches the VM (e.g. an attack straight at an ESP32) |
| Proxmox bridge `vmbr0` on `pve` (`tcpdump -i vmbr0`) | Everything crossing the bridge to and from the VMs | Traffic between two other machines on the physical LAN |

**Recommendation:** capture on the Linux machine for the attack itself, and
*also* run a second capture on the OpenPLC VM or `vmbr0` so the pcap shows how
the attack propagated (for example, a forged Modbus write reaching an ESP32
and the HEARTBEAT/coil values changing in the next poll). Put the two
files in the same run with a `_attacker` / `_plc-side` suffix. If SCT5 has a
port-mirror setting, use that instead and a single file is enough. `[ ]` record
which option is used.

Capturing other hosts' traffic by ARP spoofing is a *scenario* in its own right,
not a capture method. Do not use it just to get a wider view.

## 2. Capture filter (limits what is recorded)

Set in Wireshark under Capture > Options > Capture Filter, or with `-f`:

```
net 192.168.50.0/24 and (tcp port 502 or tcp port 8080 or arp or icmp)
```

For a port scan scenario, drop the port list so the scan itself is recorded:

```
net 192.168.50.0/24
```

## 3. Display filters (what to look at afterwards)

| Goal | Filter |
|---|---|
| All Modbus TCP | `mbtcp` or `tcp.port == 502` |
| Only writes | `modbus.func_code == 5 || modbus.func_code == 6 || modbus.func_code == 15 || modbus.func_code == 16` |
| Write Single Coil only (e.g. `ALARM_ACTIVATE`) | `modbus.func_code == 5` |
| One node | `ip.addr == 192.168.50.12` (ESP32) or `ip.addr == 192.168.50.112` (its container) |
| Writes from anything that is not the PLC or SCADA | `modbus.func_code == 5 && !(ip.src == 192.168.50.10) && !(ip.src == <SCADA IP>)` |
| Modbus exceptions | `modbus.exception_code` |
| OpenPLC web UI | `tcp.port == 8080` |
| Connection resets / loss of a node | `tcp.flags.reset == 1 || tcp.analysis.retransmission` |
| ARP changes | `arp` |

Function codes used in this project: 1/2/3/4 read, 5 write single coil, 15 write
multiple coils, 6/16 write holding register(s). Wireshark shows register
addresses as `Reference Number`; they are 0-based, matching the register maps in
`BSH_Systems/` and `FH_Systems/`.

## 4. Settings that avoid problems

- **Format:** pcapng. **Time display:** View > Time Display Format > Date and
  Time of Day, so the packets line up with the written log.
- **Ring buffer** for long sessions: Capture > Options > Output, "Create a new
  file automatically after 100 MB", keep the last 10 files. Lets the capture
  run unattended through a whole exercise without filling the disk.
- **No capture as root GUI session:** add the user to the `wireshark` group
  (`sudo dpkg-reconfigure wireshark-common`, then log out and in) instead.
- **Command-line equivalent**, handy over SSH and for the OpenPLC VM:
  ```
  sudo tcpdump -i <iface> -s 0 -w Captures/<name>.pcap 'net 192.168.50.0/24 and (tcp port 502 or tcp port 8080 or arp or icmp)'
  ```
  Wireshark opens `.pcap` files directly.
- **Clock:** NTP is blocked on this network. Before each session run `date` on
  the capture machine and on the attacking machine and write both in the run
  log. If they differ by more than a second, say so in the log.
- **Disk:** the Linux machine is a VM or PC with limited space; check `df -h`
  before a long capture.

## 5. Keeping evidence trustworthy

- Do not capture while changing the setup (reflashing a node, restarting a
  container). That belongs to a different log.
- Save the first minute of every run as baseline: it shows the normal polling
  pattern the attack will be compared with.
- Hash each finished capture (`sha256sum`) and write the hash in the run log if
  the file will be handed in as evidence for the report.
