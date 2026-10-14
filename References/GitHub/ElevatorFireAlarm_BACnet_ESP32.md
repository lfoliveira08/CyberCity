# BACnet on ESP32 — `bacnet-stack-esp32` and Chipkin `ESP32-BACnetServerExample`

- **Links:** the two options evaluated were the `bacnet-stack-esp32` port and Chipkin's `ESP32-BACnetServerExample`. Exact repo URLs are not recorded in this project's docs — add them here if this is ever revisited
- **System:** Elevator and Fire Alarm (evaluated for, not used)
- **Status:** **Rejected for now** — the BACnet backhaul is not part of the current design
- **License:** not recorded; nothing was taken
- **Checked:** not independently verified; written from `References/INDEX.md` and `Alternate_Design_BACnet_MQTT_Option.md`

## What they are

Implementations that let an ESP32 act as a BACnet/IP device. BACnet is genuinely
common in real building integration — fire alarm and elevator status reporting are
two of its most typical uses — so it was the first choice for those two systems'
backhaul.

## Why BACnet is not in the project

Not these libraries' fault. **OpenPLC's master/polling engine only speaks Modbus
TCP or Modbus RTU** — it cannot poll a BACnet device or subscribe to MQTT. Since
the project's requirement is that each ESP32 connects *directly* to its own
dedicated OpenPLC instance with its own ladder logic, the backhaul can only be
Modbus. Using BACnet would require a translation bridge in front of every
OpenPLC, which is a realistic pattern (real BACnet-to-BMS gateways work exactly
that way) but is extra software to build, test and debug — against the project's
"Stability Over Complexity" principle.

Protocol diversity was preserved at the **field-bus** layer instead: SPI/RFID at
the Door, DALI at the Power Box. Neither is affected by this decision.

## If it is revisited

- The full alternate design is preserved in
  `Alternate_Design_BACnet_MQTT_Option.md`, including the BACnet object maps for
  both systems and the bridge pattern. Nothing in the current field-layer choices
  would need to change.
- Budget real debugging time. Prior research found working implementations but
  also community reports of rough edges in some ports — that risk was the main
  driver behind the simpler direct-Modbus design.
- A lower-effort middle ground exists that needs no bridge and no firmware
  change: OpenPLC can act as a **DNP3 outstation**, so the SCADA-to-OpenPLC link
  could use DNP3 for some instances instead of Modbus. Note it is not available
  on Windows.
