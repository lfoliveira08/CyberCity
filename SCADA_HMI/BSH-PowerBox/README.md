# BSH-PowerBox -- SCADA / HMI

ESP32 192.168.50.14 -> OpenPLC container 192.168.50.114 (Ignition device host, port 502). Kind: Power Box.

**Status:** not started.

**Command rule for this node:** Buttons must write a changed value: the ESP32 counts a write only when it differs from the last hub value (send the opposite, then the original).

Sources: `PLC_Logic/BSH-PowerBox/Register_Map.md` (addresses), `PLC_Logic/BSH-PowerBox/README.md` (what the PLC does).
