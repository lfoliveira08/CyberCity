# Scenario 13 runbook — ESP32 bricking (recoverable soft-brick demo)

Created Nov 7, 2026. The **extra, final, destructive** scenario. Closed lab only. Detail:
`Security_Capture/Scenarios.md` (#13). Run on a **spare/expendable ESP32 on the bench — NEVER a
production node.**

## Scope and what this demonstrates

Shows a **destructive attack on a node and its recovery**: an attacker with access to a device's
flashing interface can erase its firmware so it will not boot, and how the lab recovers. This is
the "destructive cleanup" final stage (`Real_World_Incidents.md §3`). It is **not** a Modbus/network
attack — our firmware has no OTA path, so bricking uses the local USB/serial flashing interface.

**Two levels (we perform only the first):**
- **Soft-brick (performed, recoverable):** erase/corrupt the application flash → board won't boot →
  **re-flash the known-good firmware** to recover. UART download mode is enabled by default, so
  `esptool` can always re-flash.
- **Permanent brick (documented, NOT performed):** burning the eFuse that disables ROM/UART download
  mode (`esp_efuse_disable_rom_download_mode()`) makes the board unflashable — *"if UART Download mode
  is disabled then esptool.py can not work on the device"* (`References/ESP-IDF/05-security-guides.md`
  lines 70–72). Irreversible; only wastes a board; **do not do it.**

## Safety (read first)

- **Spare board only.** Confirm the board on the bench is an expendable spare, physically separate
  from the eight production nodes. Label it.
- **Have the recovery firmware in hand before you start** — the known-good `.bin` (or the Arduino
  sketch to re-upload) for the node type you are imitating. Without it you cannot recover.
- **Never** run the eFuse/download-mode-disable step. Soft-brick only.
- Closed lab; no production node touched.

## Prerequisites

1. The spare ESP32, connected by USB to the workstation/Kali VM with `esptool` available.
2. The recovery firmware for that node type (from `Firmware/<System>/.../*.ino` re-flash, or a saved
   `.bin`). Verify you can build/flash it **before** erasing.
3. Optional: a baseline capture / the Ignition view open, if the spare is joined to the lab, so the
   offline + recovery is visible on the dashboard (same symptom as #9).

## Procedure (you drive; one step at a time)

**Step 0 — Confirm recovery.** Prove you can re-flash the spare with the known-good firmware **now**,
before breaking anything. Flash it, see it boot/heartbeat. If recovery works, proceed.

**Step 1 — Baseline.** Note the board boots and runs (serial banner / heartbeat). If it is on the
lab subnet, note its SCADA dot is green.

**Step 2 — Soft-brick.** Erase the application flash with the standard flashing tool (`esptool`
erase). This is normal developer tooling used destructively — no custom exploit.

**Step 3 — Observe the effect.** The board no longer boots: no serial banner, no heartbeat; if on
the subnet, its SCADA dot goes red and `NodeOffline` fires (the destructive-looking impact).

**Step 4 — Recover (the point of the demo).** Re-flash the known-good firmware over UART download
mode. Confirm it boots, heartbeat climbs, SCADA dot returns to green. Record the recovery time.

**Step 5 — Log.** Record the board used (spare ID), the erase, the dead state, and the recovery in
that day's `Logs/` entry and (if captured) `Security_Capture/Capture_Log_Template.md`.

## For the report

- **ATT&CK:** T0857 System Firmware / T0839 Module Firmware; impact **T0826 Loss of Availability**
  (+ T0809 Data Destruction). (Our analysis — verify IDs on `attack.mitre.org`.)
- **Mitigation:** physical access control to the USB/flash port; **recoverability** (keep per-node
  firmware images); detection via the heartbeat / `NodeOffline` alarm.
- **The lesson:** a destructive attack needs device/flash access, is detectable (node drops off), and
  is **recoverable if you keep firmware images** — permanent loss requires the irreversible eFuse
  step, which we chose not to perform.

## Not performed / not verified

Nothing has been run. The permanent eFuse path is documented, not executed. ESP32 flash/eFuse
specifics are cited from `References/ESP-IDF/05-security-guides.md`; confirm against the board's
actual behavior on the bench.
