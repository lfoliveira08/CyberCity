# thiagoralves / OpenPLC_v3 (OpenPLC Runtime)

- **Link:** https://github.com/thiagoralves/OpenPLC_v3
- **System:** all 8 nodes (Phase C: one OpenPLC instance per system)
- **Status:** **Adopted** (planned for Phase C; nothing installed yet)
- **License:** not yet recorded -- check it before copying any of its code
- **Version installed:** none yet. Record the version/commit here when the first container is built
- **Checked:** Oct 11, 2026, against the repository README only. The documentation site (autonomylogic.com) redirects to edge.autonomylogic.com/docs

## What it is

The OpenPLC soft-PLC runtime: a web UI (port 8080) for uploading programs, a scan-cycle runtime, a Modbus server (port 502) and a Modbus master that polls slave devices. Each system's ESP32 is a Modbus TCP slave of its own OpenPLC instance.

## What we use it for

One instance per system, each in its own Docker container on its own macvlan IP (`Piranga_Workflow_v2.md` Section 6). The programs for each system are in `PLC_Logic/<System>/Program.st`.

## Things to watch

- The README describes building a local image (`docker build -t openplc:v3 .`) and an example `docker run -it --rm --privileged -p 8080:8080 openplc:v3`. It does **not** name an official published image, and it does not mention port 502, data persistence or default login. See `PLC_Logic/Setup/README.md` for the open items.
- Where slave-device data appears (`%IX100.0`, `%QX100.0`, `%IW100`, `%QW100`) is the v3 convention but is not restated in the current docs. Verify on the first container (`PLC_Logic/Templates/Address_Mapping.md`).
- Modbus on OpenPLC has no authentication, which is the teaching point behind Scenarios 4, 6, 7 and 11.
