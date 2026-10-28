# Firmware/ — rules for any agent working here

Root rules: `../CLAUDE.md`. Current status: `../STATUS.md`. **`WORKFLOW.md` in this folder is authoritative** for build order, the single-focus rule, phase definitions and where every kind of file belongs (Section 8). If any other doc disagrees with it about firmware process, it wins.

- Phases A0 (Wokwi) → A (bench) → B (Modbus backhaul). Start at the system's `Wokwi/` before any real wiring.
- All eight nodes are Phase B bench-confirmed. Do not re-flash or "improve" working firmware unless Luis asks.
- A register or pin change touches four things together: the sketch, the system doc in `../BSH_Systems/` or `../FH_Systems/`, the bring-up checklist, and the OpenPLC Input Register count in `../PLC_Logic/`. All four or none.
- Every node exposes `HEARTBEAT` and `RECONNECT_COUNT` as its last two Input Registers.
- The ESP32 owns safe-state behavior. The PLC forwards commands; it does not decide safety.
- Coil writes count only when the value **differs** from the last hub value (changed Oct 21, 2026 — OpenPLC rewrites coils every poll).
- Luis does the wiring and the flashing. Guide one step at a time; never assume a bench result you were not shown.
