# Templates -- shared pieces

To be filled while building the pilot node (BSH-FireAlarm), then reused for the other seven.

- [ ] Tag naming rule: `<Bldg>/<System>/<Name>` (proposed).
- [x] Device UDT (built Nov 2, 2026; see the rule below). Blue (recovering) and red (intrusion) are **not** in it.
- [ ] Tag import template (`Tags.json` shape that works).
- [ ] Per-system view template for Perspective.

## Rules that every node's tags must respect (from the PLC build notes, Oct 20-21, 2026)

| Node type | Rule |
|---|---|
| Door (BSH, FH) | Open/close commands are pulses of 300 ms or longer. A held open bit keeps the gate open and blocks the card reader. |
| Elevator (BSH) | Call-floor commands are pulses (the PLC makes a 300 ms pulse itself); lockout is a plain level. |
| PowerBox (BSH, FH) | The ESP32 counts a write only when its value differs from the last hub value. Buttons must write a changed value (send the opposite, then the original). |
| HVAC (FH) | Set the target (setpoint) first, then enable. The PLC rewrites the ESP32 defaults on every start, so the unit stays off until both are written. |
| FireAlarm (BSH, FH) | `SCADA_ALARM_ACTIVATE` is a plain level copy. |

## The `Device` UDT (built Nov 2, 2026)

One definition, one instance per node, in the tag folder `Device` (one instance per node, named like the Modbus device: `BSH-Elevator`, `FH-HVAC`, and so on). The hand-built tags in `BSH/` and `FH/` stay; they carry the sensors and commands. Manual: `platform/tags/user-defined-types-udts.md`.

| Part | What it is |
|---|---|
| Parameters (String) | `DeviceName` (the Modbus device name), `HeartbeatAddr`, `ReconnectAddr` (Modbus item paths such as `HR1`) |
| `Heartbeat` | OPC member, path `[{DeviceName}]{HeartbeatAddr}` |
| `ReconnectCount` | OPC member, path `[{DeviceName}]{ReconnectAddr}` |
| `HeartbeatAge` | Expression, Integer: `secondsBetween(timestampOf({[.]Heartbeat}), now(1000))` = seconds since the heartbeat last changed |
| `Status` | Expression, Integer: `if(isGood({[.]Heartbeat}), if({[.]HeartbeatAge} >= 5, 2, if({[.]HeartbeatAge} >= 3, 1, 0)), 2)`: 0 normal (green), 1 degraded (yellow, about 3 s), 2 offline (red, about 5 s or bad quality) |

Addresses per node (from the exports of the hand-built tags): BSH-Elevator, BSH-FireAlarm, FH-FireAlarm HR1/HR2; both Doors and both PowerBoxes HR0/HR1; FH-HVAC HR3/HR4.

**How it was proven (Luis-reported, BSH-FireAlarm only):** unplugging the ESP32 with the PLC running froze `Heartbeat`, `HeartbeatAge` climbed to about 10 s, `Status` went 0, 1, 2 and back to 0 after replugging. The timestamp of an unchanged polled value did not move, which the manual does not state. The other seven instances have not been through that test.

**Rule:** a new node gets a new `Device` instance (copy one, rename it, set the three parameters). Never put logic in a screen component that the UDT can hold; screens only read `Status`.
