# Templates -- shared pieces

To be filled while building the pilot node (BSH-FireAlarm), then reused for the other seven.

- [ ] Tag naming rule: `<Bldg>/<System>/<Name>` (proposed).
- [ ] Device UDT: heartbeat, reconnect count, derived status (green / yellow / red / blue).
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
