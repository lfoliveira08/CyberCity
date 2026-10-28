# FH-HVAC -- SCADA / HMI

ESP32 192.168.50.21 -> OpenPLC container 192.168.50.121 (Ignition device host, port 502). Kind: HVAC zone unit.

**Status:** not started.

**Command rule for this node:** Write the target (setpoint) first, then enable. The unit stays off after a PLC start until both are written.

Sources: `PLC_Logic/FH-HVAC/Register_Map.md` (addresses), `PLC_Logic/FH-HVAC/README.md` (what the PLC does).
