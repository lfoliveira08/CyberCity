# Rapid SCADA

- **Link:** https://rapidscada.org/ (community modules repo: https://github.com/RapidScada/scada-community, which is add-ons, not the core)
- **System:** SCADA/HMI layer (Phase C)
- **Status:** **Under evaluation**, fallback #1 (Ignition chosen Oct 12, 2026, license confirmed Oct 19, 2026; only needed if that license is lost)
- **License:** per the site, the Community edition is free with no data-point limit and no expiry; paid editions add modules. The community repo is Apache-2.0
- **Version installed:** none. The site lists 6.5.0 released Sept 7, 2026
- **Checked:** Oct 9, 2026, the product site and one third-party integration guide

## What it is

An open-source industrial automation platform. A third-party guide ("Configuring openPLC and Rapid SCADA 6 Modbus Integration", industrialmonitordirect.com) describes connecting Rapid SCADA 6 to OpenPLC: a Modbus line using the DrvModbus plugin, a Modbus TCP device per PLC (IP, port 502, unit ID 1, timeout ~3000 ms), and an acquisition table mapped to input channels with alarm limits. It runs on Windows, or on Linux with the .NET 6 runtime.

## Things to watch

- That guide is a third-party source, not the vendor's documentation. Confirm against Rapid SCADA's own docs.
- The site did not describe the web HMI, alarms or user roles in detail, and the new mimic diagram engine is labelled experimental.
- Gotchas the guide lists: Modbus vs OpenPLC off-by-one addressing, and only holding registers mapped to writable `%QW` variables accept writes. Both apply to `PLC_Logic/`.
