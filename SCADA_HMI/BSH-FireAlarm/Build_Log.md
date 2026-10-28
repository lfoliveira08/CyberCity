# BSH-FireAlarm -- SCADA build log

| Date | Step | Result |
|---|---|---|
| (none yet) | | |
| 2026-10-26 | Created Modbus TCP device `BSH-FireAlarm` in the Gateway (host 192.168.50.112, port 502, timeout 2000 ms, request-optimization and read/write options left at defaults). Showed Disconnected until the PLC was started in OpenPLC (Start PLC on 192.168.50.112:8080), then Connected. | Device Connected. |
| 2026-10-26 | Designer: created temporary OPC tag `TestHeartbeat` (server Ignition OPC UA Server, item path `[BSH-FireAlarm]HR1`, Integer). | Value read 173 and rose over time (Luis reported it increases). `HR1` = PUB_HEARTBEAT confirmed with default device settings. Unit ID and Zero-Based Addressing fields were not found or inspected, so their values are not recorded. |
