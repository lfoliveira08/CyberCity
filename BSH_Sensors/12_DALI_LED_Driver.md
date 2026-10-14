# DALI LED Driver / Ballast Interface — RETIRED (Sep 28, 2026)

This component was replaced by a plain on/off relay channel for BSH-PowerBox
lighting (the second channel of the same relay module already driving the
fan). See `claude/BSH_Systems/04_BSH-PowerBox.md` Notes,
`claude/BSH_Sensors/09_Relay_Module.md`, and the Sep 28, 2026 entries in
`Piranga_Workflow_v2.md` Section 14 for the decision and rationale.

The retired library reference is kept at
`References/GitHub/PowerBox_silahcen001_DALI-ESP32.md` (status: Rejected)
so nobody re-evaluates it without knowing why it was dropped. The physical
build folder that used to document this component,
`Firmware/BSH-PowerBox/DALI_LED_Driver/`, has been renamed to
`Firmware/BSH-PowerBox/Lights_Relay/`.

This file is intentionally left as a stub, not deleted, so any old
cross-reference to `12_DALI_LED_Driver.md` still resolves to something.
