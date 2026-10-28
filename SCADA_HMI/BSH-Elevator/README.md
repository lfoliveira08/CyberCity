# BSH-Elevator -- SCADA / HMI

ESP32 192.168.50.11 -> OpenPLC container 192.168.50.111 (Ignition device host, port 502). Kind: Elevator.

**Status:** not started.

**Command rule for this node:** Call-floor commands are pulses (the PLC makes a 300 ms pulse); lockout is a plain level.

Sources: `PLC_Logic/BSH-Elevator/Register_Map.md` (addresses), `PLC_Logic/BSH-Elevator/README.md` (what the PLC does).
