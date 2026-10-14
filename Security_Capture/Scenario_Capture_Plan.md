# Scenario capture plan

For each scenario in `Piranga_Workflow_v2.md` Section 11: what to record, what
the pcap should show, and the filter that proves it. Register names and
addresses come from the system docs (`BSH_Systems/`, `FH_Systems/`) and
`PLC_Logic/<System>/Register_Map.md`; check them there before a run rather than
trusting the examples below. Setup, placement and general filters:
`Capture_Setup.md`.

Every run: baseline (30-60 s of normal polling) -> action -> a few seconds
after -> stop. Log with `Capture_Log_Template.md`.

| # | Scenario | Capture on | What the pcap should show | Filter |
|---|---|---|---|---|
| 1 | Normal operation | Both captures if possible | The baseline: steady Modbus reads on port 502 from each container to its ESP32 (and SCADA to each container once it exists); HEARTBEAT incrementing; no writes except those the PLC program makes | `mbtcp` |
| 2 | Authorized door access | PLC side | A badge is a physical event, not a network one. The pcap shows its *effect*: `ACCESS_GRANTED` (DI2) going to 1 for about 2 s in the polled response, then the gate position changing. No Modbus write from a foreign host | `ip.addr == 192.168.50.13 && mbtcp` |
| 3 | RFID UID clone / replay | PLC side | The attack is on the card reader, so nothing attack-specific appears on the network. Capture to show that the cloned card produces exactly the same `ACCESS_GRANTED` pulse as the real one, which is the point of the scenario. Note the time of each badge in the log so the pulses can be matched | `ip.addr == 192.168.50.13 && mbtcp` |
| 4 | False fire alarm injection | Attacker + PLC side | A Modbus **Write Single Coil (FC 5)** to Coil 0 (`ALARM_ACTIVATE`) from the attacker's IP, no matching `MANUAL_PULL_TRIGGERED` input in the surrounding polls, then the alarm outputs active. Compare the write's source IP with the legitimate Modbus clients | `modbus.func_code == 5 && ip.addr == 192.168.50.112` (or `.12` if aimed directly at the ESP32) |
| 5 | Elevator lockout (BSH) | Attacker + PLC side | A write that sets `ELEVATOR_DISABLE` with no accompanying fault bit, from a non-PLC source | `modbus.func_code == 5 && ip.addr == 192.168.50.111` |
| 6 | Backup power ATS | -- | **Dropped Oct 5, 2026. Do not run or capture.** | -- |
| 7 | Power box manipulation | Attacker + PLC side | A write flipping the lights or fan coil from an unexpected IP, with no schedule or PLC logic behind it | `modbus.func_code == 5 && (ip.addr == 192.168.50.114 || ip.addr == 192.168.50.124)` |
| 8 | Cross-building cascade | Mirror or both sides | A Door event in one building, then SCADA writes to both Fire Alarm nodes. Needs the SCADA layer, which does not exist yet (Ignition license pending); plan the capture for when it does. The pcap order and timing of the writes is the evidence | `modbus.func_code == 5 || modbus.func_code == 15` |
| 9 | Node isolation / resilience | Mirror or PLC side | One container stops: the TCP session to it ends (FIN or RST), polls to that IP go unanswered or are reset, while the other 7 keep answering. Time between the stop and SCADA flagging it can be read directly | `tcp.flags.reset == 1 || tcp.analysis.retransmission` and `ip.addr == <stopped container>` |
| 10 | Red / Blue exercise | Mirror (preferred) | Contains scenarios 3-5, 7 and 11 in one session; the Blue Team can use the capture afterwards to check what they missed. Keep a single running timeline in the log | all of the above |
| 11 | HVAC spoofing / setpoint tampering (FH) | Attacker + PLC side | A write to `TARGET_TEMP_C` (HR0) from a foreign IP (FC 6 or 16), or the fabricated `TEMP_C` value, followed by `COMPRESSOR_RUNNING` not matching the temperature trend. Modbus input registers cannot be written with a standard FC; confirm in `FH_Systems/` how the spoof is delivered (holding-register write, or a PLC-side value) and capture that path | `modbus.func_code == 6 || modbus.func_code == 16` with `ip.addr == 192.168.50.121` |

## Also worth capturing

- **Reconnaissance** that precedes any attack: a ping sweep or a port scan of
  `192.168.50.0/24`. Use the broad capture filter from `Capture_Setup.md`. Shows
  the project's decision to keep real port 502 open on every container
  (`Piranga_Workflow_v2.md` Section 6): one default Modbus scan finds all eight.
- **The unauthenticated nature of Modbus TCP**: open any write packet and show
  there is no credential field in the frame (MBAP header: transaction ID,
  protocol ID, length, unit ID, then function code and data). This is a good
  screenshot for the report.
- **OpenPLC web UI traffic (port 8080)**: the default login is kept
  (`Next_Steps.md`, Oct 13), so credentials cross the wire in clear HTTP. Capture
  a login once; it supports a "cleartext management interface" finding.

## For the report (Deliverables 6 and 10)

For each scenario keep: one screenshot of the Wireshark packet list filtered to
the attack, one expanded Modbus packet showing function code, reference number
and value, and the matching row of the run log. Name screenshots like the pcap.
