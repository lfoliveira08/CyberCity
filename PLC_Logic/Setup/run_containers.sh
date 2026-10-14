#!/usr/bin/env bash
# Run inside the OpenPLC Docker VM, after create_network.sh:  sudo ./run_containers.sh
# One container per line in openplc_nodes.csv, each on its own macvlan IP.
# Updated Oct 13, 2026 after the first container (BSH-FireAlarm) was started by hand:
#  - containers that already exist are skipped, so it is safe to re-run
#  - each container gets its own named volume on /docker_persistent (programs and settings survive a rebuild)
#  - no --privileged (the first container ran without it)
set -euo pipefail
IMAGE="${IMAGE:-openplc:v3}"   # built locally from the OpenPLC_v3 repo, see README.md
CSV="$(dirname "$0")/openplc_nodes.csv"
tail -n +2 "$CSV" | while IFS=, read -r name esp ip; do
  if docker ps -a --format '{{.Names}}' | grep -qx "$name"; then
    echo "Skipping $name (already exists)"
    continue
  fi
  vol="openplc_$(echo "${name#OpenPLC-}" | tr 'A-Z-' 'a-z_')"   # OpenPLC-BSH-FireAlarm -> openplc_bsh_firealarm
  echo "Starting $name at $ip (polls ESP32 $esp), volume $vol"
  docker run -d \
    --name "$name" --hostname "$name" \
    --network cybercity_lan --ip "$ip" \
    --restart unless-stopped \
    -v "$vol":/docker_persistent \
    "$IMAGE"
done
