# Gas/Smoke Sensor ("MQ-2" slot) — Build Log (BSH-FireAlarm)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Use this to track your actual build, separate from the general wiring
reference in `Wiring.md`. Update as you go, not just at the end.

## Status
- [ ] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: ______
- [ ] AO voltage verified safe for GPIO 34 (divider added if needed) —
      **pending — wiring is already done, so verify this now rather than
      before wiring; retrofit the divider if the check fails**
- [x] Component in hand — actually an MQ-9 (MH Sensor series module), not
      the documented MQ-2 — see `Wiring.md` note. Decision: proceed with
      MQ-9 as-is, reframe as general combustible-gas detection.
- [x] Pin labels confirmed from board silkscreen: AO / DO / GND / VCC
- [x] Wired per `Wiring.md` — finalized pinout: GND→GND, VCC→VIN,
      DO→GPIO 19 (unused by firmware), AO→GPIO 34
- [ ] Bench-tested standalone
- [ ] Confirmed working with the real `BSH_FireAlarm_Node.ino` firmware
- [ ] `GAS_ALERT_MARGIN` tuned against real bench readings (starting value
      in firmware is a placeholder: 400)

## Date log
| Date | What happened |
|---|---|
| 2026-09-17 | Identified the on-hand gas sensor module as MQ-9, not MQ-2 (checked the model number printed on the sensor can). Confirmed pin labels: AO/DO/GND/VCC. Decided to proceed with MQ-9 as a documented substitution rather than source an MQ-2, since the project's threshold logic and demo approach are both gas-agnostic. Found an MQ-9-specific reference (electropeak calibration guide) — noted a much longer real preheat time (24-48 hrs) than originally assumed, and a more rigorous Rs/R0 calibration method not yet adopted. Initial wiring plan had DO on GPIO 21, which was caught and corrected — GPIO 21 has no ADC hardware, wouldn't have worked for AO anyway and wasn't the pin AO was going to use, but confirmed GPIO 21 isn't usable for analog reads at all. Finalized pinout: GND→GND, VCC→VIN, DO→GPIO 19 (wired but unused by firmware), AO→GPIO 34. Physically wired. Voltage check on AO (safety step, verifies it stays under 3.3V before trusting/relying on GPIO 34) still needs to be done — was not completed before wiring. |

## Lessons Learned
- The generic "MH Sensor"/breakout-style PCB used for MQ gas sensors (and separately for the flame sensor) is shared across many different specific sensor variants — always check the printed model number on the actual sensing element, not just the PCB, before assuming which part you have.
- Caught a wiring plan (AO → GPIO 21) that wouldn't have worked before any hardware was connected to it — GPIO 21 has no ADC capability on the ESP32. Worth checking a board's ADC-capable pin list before assuming any GPIO number can do `analogRead()`.
- The AO voltage safety check (multimeter, confirm under 3.3V before trusting the GPIO) ended up happening after wiring instead of before, since wiring moved faster than the check. Worth doing this check before powering on / trusting readings, and treating "already wired" as a reason to check now, not a reason to skip it.

## Troubleshooting Tips
