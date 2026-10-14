# Captures

One `.pcapng` and one filled-in log per run. Keep the pair together.

## Naming

`YYYYMMDD_HHMM_S<scenario>_<target>_<short-description>.pcapng`

Examples:
- `20261104_1430_S4_BSH-FireAlarm_coil0-injection.pcapng`
- `20261104_1430_S4_BSH-FireAlarm_coil0-injection.md` (the log, from `../Capture_Log_Template.md`)
- `20261104_1500_S1_baseline_normal-operation.pcapng`

Target = the system name used everywhere else (`BSH-FireAlarm`, `FH-HVAC`, ...);
use `campus` when the run touches several.

## Housekeeping

- Never rename or edit a pcap after the run. Filtered or trimmed copies get a
  `_filtered` suffix and sit beside the original.
- Pcaps can be large; delete throwaway test captures the same day.
- Do not add `_v2` or `_final`: if a run is repeated, it is a new run with a
  new timestamp.
