# PLC_Logic/ — rules for any agent working here

Root rules: `../CLAUDE.md`. Current status: `../STATUS.md`. Ignition (the SCADA side) is `../SCADA_HMI/`; this folder is OpenPLC only.

- One folder per system: `README.md`, `Register_Map.md`, `Ladder_Diagram.md`, `Program.st`, `Build_Log.md`, plus the OpenPLC Editor project `<Bldg>-<System>-ESP32/`.
- **`Program.st` is the deploy artifact** for the v3 container (the v4 Editor does not export a loose `.st`). The Editor project is the visual source and learning record. Keep them in step.
- `<System>-ESP32/build/` is regenerable compiled output. Never read it, never edit it, never commit it.
- Runtime is OpenPLC **v3**, decided Oct 15, 2026 and held through the Dec 11 demo. Do not propose v4.
- Slave ID 0 on every node; the container's Modbus server only answers while the PLC is **running**.
- Address conventions live in `Templates/Address_Mapping.md`. A register map change must be mirrored in `../SCADA_HMI/<System>/Tag_Map.md` and `Tags.json` the same sitting.
- Luis builds the rungs in the Editor himself. Teach each rung and why; do not hand over finished logic. (See the `teach-while-building-ladder` skill.)
