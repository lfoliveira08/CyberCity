#!/usr/bin/env bash
# Run inside the OpenPLC Docker VM (verified Oct 13, 2026):  sudo env PARENT_IF=ens18 ./create_network.sh
# (Ubuntu 26.04's sudo ignores -E, so pass the variable with env.)
# Creates the macvlan network the 8 OpenPLC containers attach to.
set -euo pipefail
PARENT_IF="${PARENT_IF:?set PARENT_IF to the VM network interface, e.g. PARENT_IF=ens18 (see: ip -br link)}"
docker network create -d macvlan \
  --subnet=192.168.50.0/24 \
  --gateway=192.168.50.1 \
  -o parent="$PARENT_IF" \
  cybercity_lan
