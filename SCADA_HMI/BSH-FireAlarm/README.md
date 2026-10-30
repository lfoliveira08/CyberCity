# BSH-FireAlarm -- SCADA / HMI

ESP32 192.168.50.12 -> OpenPLC container 192.168.50.112 (Ignition device host, port 502). Kind: Fire Alarm.

**Status:** see `../../STATUS.md`. Build detail: `Build_Log.md` (Oct 29: device rebuilt, 8 tags built by hand, tested).

**Command rule for this node:** Alarm command is a plain level copy.

Sources: `PLC_Logic/BSH-FireAlarm/Register_Map.md` (addresses), `PLC_Logic/BSH-FireAlarm/README.md` (what the PLC does).
