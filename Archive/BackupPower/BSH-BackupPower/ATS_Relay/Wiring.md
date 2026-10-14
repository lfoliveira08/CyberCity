# ATS Transfer Relay — BSH-BackupPower

Reference doc: `BSH_Sensors/09_Relay_Module.md`

## What it does here
Drives the demo load/indicator that models an Automatic Transfer Switch
committing to backup power. Commanded by `ATS_TRANSFER_CMD` (Coil0),
open-loop — no feedback discrete wired yet, same limitation as
BSH-PowerBox's relay commands.

## Key specs
- 1-channel 5V relay module (this node only needs one channel)
- **Typically active-LOW trigger** — check your specific module
- Relay VCC from 5V, GND common with ESP32; demo load on its own power
  rail, separate from the ESP32's own supply

## Wiring
| Relay module pin | ESP32 pin |
|---|---|
| IN (signal) | GPIO 27 |
| VCC | 5V |
| GND | Common ground |

## Firmware tie-in
`BSH_BackupPower_Node.ino` (not written yet): `PIN_ATS_RELAY = 27`,
driven by `setAtsRelay(bool on)`. Commanded by Modbus Coil `ATS_TRANSFER_CMD`
(Coil0). **Fails toward not-commanded on comm loss** — the opposite fail
direction from the retired Perimeter node's `ZONE_DISABLE`.

## Bench-test checklist (Phase A, this component only)
- [ ] Relay wired per the table above, demo load on its own supply
- [ ] Toggle GPIO 27 from a throwaway test sketch, confirm the relay
      clicks and the demo load actually responds, not just the relay's
      own LED
- [ ] If it behaves backwards, flip the active-LOW/HIGH assumption in
      firmware rather than rewiring

## Lessons Learned
*(fill in as you go)*

## Troubleshooting Tips
*(fill in as you go)*
