#!/usr/bin/env python3
"""recon_scan.py - Reconnaissance (precedes every attack scenario).

CyberCity closed-lab ONLY (192.168.50.0/24). Authorized capstone testing.
Run on the Kali VM (192.168.50.40). NO secrets in this file (CLAUDE.md section 8).
Capture with the BROAD filter (net 192.168.50.0/24, no port list) so the scan
itself shows on the wire - see Security_Capture/Capture_Setup.md section 2.

Steps: 1 host discovery -> 2 port scan (502/8080) -> 3 Modbus ID on the :502 hosts.
"""
import subprocess
import sys
from datetime import datetime

SUBNET = "192.168.50.0/24"
OUT = f"recon_{datetime.now():%Y%m%d_%H%M%S}"  # evidence basename, for the report


def run(cmd):
    """Echo a command, then run it and stream its output."""
    print(f"\n$ {' '.join(cmd)}\n")
    subprocess.run(cmd, check=True)


def step1_host_discovery():
    # -sn = ping sweep, no ports. The quietest first look; cross-check the live
    # hosts against which nodes you actually have powered.
    run(["nmap", "-sn", SUBNET, "-oN", f"{OUT}_hosts.txt"])


def step2_port_scan():
    # Only the two ports that matter in this lab:
    #   502  = Modbus/TCP     -> every OpenPLC container (.11x/.12x) AND every ESP32 (.1x/.2x)
    #   8080 = OpenPLC web UI -> containers only (.11x/.12x)
    # -Pn skips ping; --open shows only hosts that answer. We ALSO write greppable
    # output (-oG) so step 3 can pull out just the hosts that have :502 open.
    run(["nmap", "-Pn", "-p", "502,8080", "--open", SUBNET,
         "-oN", f"{OUT}_ports.txt", "-oG", f"{OUT}_ports.gnmap"])


def modbus_hosts():
    # Read step 2's greppable file and return the IPs with 502 open. A host line
    # looks like:  Host: 192.168.50.112 ()  Ports: 502/open/tcp//mbap///, ...
    hosts = []
    with open(f"{OUT}_ports.gnmap") as f:
        for line in f:
            if line.startswith("Host:") and "502/open" in line:
                hosts.append(line.split()[1])
    return hosts


def step3_modbus_id():
    # modbus-discover SPEAKS Modbus (queries unit ID + function-43 device ID),
    # confirming live endpoints. Run it ONLY on the hosts that have :502 open
    # (from step 2) - scanning the whole /24 with the NSE is slow and pointless.
    targets = modbus_hosts()
    if not targets:
        print("No hosts with :502 open - nothing to identify.")
        return
    target_file = f"{OUT}_modbus_targets.txt"
    with open(target_file, "w") as f:
        f.write("\n".join(targets) + "\n")
    print(f"Modbus hosts ({len(targets)}): {', '.join(targets)}")
    # -oA saves all three formats (normal/greppable/XML) under OUT -> report evidence.
    # --host-timeout/--max-retries cap each host: these simple servers often don't
    # answer the device-ID query, so without a cap the NSE waits on timeouts.
    run(["nmap", "-n", "-Pn", "-p", "502", "--script", "modbus-discover",
         "--host-timeout", "15s", "--max-retries", "1",
         "-iL", target_file, "-oA", OUT])
    # Optional, noisier - probe every unit ID 0-255 instead of the default:
    #   --script-args modbus-discover.aggressive=true


if __name__ == "__main__":
    try:
        step1_host_discovery()
        step2_port_scan()
        step3_modbus_id()
    except KeyboardInterrupt:
        print(f"\nInterrupted - partial output saved under {OUT}")
        sys.exit(130)
