# Campus_Map -- top-level HMI

Both buildings side by side, all four systems each, with per-system status and an event log panel.

Color coding (from `Piranga_Workflow_v2.md` Section 10):

| Color | Meaning |
|---|---|
| Green | Normal operation |
| Yellow | Warning / degraded communication |
| Red | Intrusion / compromised |
| Blue | Active communication |

Heartbeat rule: frozen about 3 s = yellow, about 5 s = red, resumed = blue briefly, then green. Log every lost/reconnected event with timestamp and downtime; use `RECONNECT_COUNT` to tell a WiFi drop from a reboot.

Status: not started.
