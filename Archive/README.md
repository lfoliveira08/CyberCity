# Archive — retired files, kept for reference only

Nothing in this folder is part of the current design. Files land here when a
decision supersedes them, instead of being deleted outright, so the reasoning
behind an old approach stays recoverable. Every entry is also recorded in
`Piranga_Workflow_v2.md` Section 14 (Lessons Learned & Decisions Log).

**If a file is in here, do not build from it.** The authoritative references
are `Piranga_Workflow_v2.md` (architecture, networking, scenarios) and
`Firmware/WORKFLOW.md` (process, build order, status).

| File | Retired | Why | Replaced by |
|---|---|---|---|
| `01_FH-Elevator.md` | Sep 26, 2026 | Founder's Hall slot 1 changed from Elevator to an HVAC zone unit — FH no longer mirrors BSH in slot 1 | `FH_Systems/01_FH-HVAC.md` |
| `FH_Elevator_Bringup_Checklist.md` | Sep 26, 2026 | Bring-up checklist for the retired FH-Elevator node. Its unfixed-IP warning (`.11` hardcoded where `.21` belonged) is preserved here because the same class of copy-paste bug is worth re-checking in every other FH sketch | `Firmware/FH_HVAC_Bringup_Checklist.md` |
| `BSH_Elevator_Node-1.ino` | Sep 26, 2026 | Abandoned draft using the alternate L293D pin mapping (ENABLE 13 / DIRA 14 / DIRB 26) that was prepared but never needed. The bench-confirmed mapping is 18/19/5 | `Firmware/BSH-Elevator/BSH_Elevator_Node/BSH_Elevator_Node.ino` |
| `BSH_Elevator_Node.ino` | Sep 26, 2026 | Byte-identical to the canonical sketch except for a missing STATUS header — a stale copy left in the download folder | same as above |
| `wiring_diagram_preview.png` | Sep 26, 2026 | Unreferenced raster preview; the canonical, editable diagram is the SVG | `Firmware/BSH-Elevator/Wiring_Diagram.svg` |
| `05_BSH-Perimeter.md` / `05_FH-Perimeter.md` | Sep 28, 2026 | BSH/FH slot 5 changed from the Perimeter/Motion Intrusion Zone to a Backup Power / ATS Monitor system -- Perimeter/Motion was a pure sensor-only node with no actuator, and the new system gives the project a genuine OT-security-relevant target (ATS/generator failover manipulation) plus an actuator | `BSH_Systems/05_BSH-BackupPower.md` / `FH_Systems/05_FH-BackupPower.md` |
| `BSH_13_PIR_Motion_Sensor.md` / `FH_13_PIR_Motion_Sensor.md` | Sep 28, 2026 | PIR motion sensor was Perimeter/Motion-specific and has no other user in the current design after that system's retirement | none -- no PIR sensor in the active build |
| `BSH_14_HC-SR04_Ultrasonic_Sensor.md` / `FH_14_HC-SR04_Ultrasonic_Sensor.md` | Sep 28, 2026 | HC-SR04 ultrasonic sensor was Perimeter/Motion-specific and has no other user in the current design after that system's retirement | none -- no ultrasonic sensor in the active build |
| `BSH_Perimeter_Bringup_Checklist.md` / `FH_Perimeter_Bringup_Checklist.md` | Sep 28, 2026 | Bring-up checklists for the retired Perimeter/Motion node | `Firmware/BSH_BackupPower_Bringup_Checklist.md` / `Firmware/FH_BackupPower_Bringup_Checklist.md` |
| `BSH-Perimeter/` (whole folder -- `README.md`, `ESP32_Core_Modbus/`, `PIR_Motion_Sensor/`, `HC-SR04_Ultrasonic_Sensor/`) | Sep 28, 2026 | Preliminary per-component bench files (two `BenchTest.ino` sketches, `Wiring.md`, `Build_Log.md` entries) written ahead of this system's turn in the build order, before the swap decision. Kept here rather than deleted in case the PIR/HC-SR04 bench notes are useful if a future system (e.g. the parked pedestrian-crosswalk idea, see `Piranga_Workflow_v2.md` Section 14) reuses either sensor | `Firmware/BSH-BackupPower/` |
| `BackupPower/` (whole folder -- `05_BSH-BackupPower.md` / `05_FH-BackupPower.md`, sensor sheets `BSH_17`-`19` / `FH_18`-`20` (Toggle Switch, Voltage Divider Battery Sense, ACS712 Current Sensor), `BSH_`/`FH_BackupPower_Bringup_Checklist.md`, and `BSH-BackupPower/` with its `Wokwi/` simulation and four component Build_Logs) | Oct 5, 2026 | Backup Power / ATS Monitor (slot 5 in both buildings) dropped without a replacement by Luis together with the course professor. Each building now has four systems; the project has 8 nodes. Its Phase A0 Wokwi simulation had been built and verified the same day (all scenarios passed, relay + demo-load LED confirmed). No FH firmware folder had been created | none -- slot 5 no longer exists. `Firmware/WORKFLOW.md` and `Piranga_Workflow_v2.md` Section 14 (Oct 5, 2026 entry) record the change |
| `PowerBox_PhaseA_inbox/` (`phaseA_03_buttons_only_test.ino`, `phaseA_04_combined_test.ino`, `BSH-PowerBox_Wiring_Reference.html`) | Oct 9, 2026 | Files that were left in the `Claude outputs/` download inbox: the two standalone PowerBox Phase A test sketches and a BSH-PowerBox wiring reference page. Phase A is long done and both PowerBoxes are now bench-confirmed through Phase B, so these are superseded history | `Firmware/BSH-PowerBox/` (`SYSTEM_REPORT.md` holds the Phase A sketch and wiring) and `BSH_PowerBox_Node.ino` / `FH_PowerBox_Node.ino` |
| `Door_PhaseA_sketches/` (`BSH_Door_Node_PhaseA.ino`, `FH_Door_Node_PhaseA.ino`) | Oct 9, 2026 | The standalone Door Phase A sketches (no WiFi, no Modbus), replaced in place by the Phase B sketches the same day | `Firmware/BSH-Door/BSH_Door_Node/BSH_Door_Node.ino` / `Firmware/FH-Door/FH_Door_Node/FH_Door_Node.ino` |

Safe to delete this whole folder once the project is finished and the final
report is written.
