# ScadaBR

- **Link:** https://github.com/ScadaBR/ScadaBR
- **System:** SCADA/HMI layer (Phase C)
- **Status:** **Not chosen** (Ignition chosen Oct 12, 2026); a weak candidate because of its old release, see below
- **License:** GPL 3+ (per the repository page)
- **Version installed:** none. Latest release on the page: 1.2, Sept 7, 2021
- **Checked:** Oct 9, 2026, repository front page only

## What it is

Free open-source SCADA (Java). Per the page: 20+ protocols including Modbus TCP/IP, real-time data points, graphical HMI screens, a configurable alarm and event engine, a scripting engine, browser access. Runs on Java 8 and Tomcat 9; Derby is the default database (MySQL/MariaDB optional). No Docker image is mentioned for ScadaBR.

## Things to watch

- Last release is from 2021 and the runtime (Java 8) is end-of-life. Fine for an isolated lab, but worth stating in the security write-up.
- Scada-LTS is a separate, maintained fork with a Docker image, according to search results. It has not been opened or evaluated.
