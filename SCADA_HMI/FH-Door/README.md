# FH-Door -- SCADA / HMI

ESP32 192.168.50.23 -> OpenPLC container 192.168.50.123 (Ignition device host, port 502). Kind: Door/Gate.

**Status:** not started.

**Command rule for this node:** Open/close commands must be pulses of 300 ms or longer; a held open bit keeps the gate open and blocks the card reader.

Sources: `PLC_Logic/FH-Door/Register_Map.md` (addresses), `PLC_Logic/FH-Door/README.md` (what the PLC does).
