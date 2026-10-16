# Address mapping cheat sheet

Two different address spaces meet in each PLC. Most bugs come from mixing them up.

## A. This PLC as a Modbus master (talking to its ESP32)

OpenPLC puts the slave device's data in the 100-block of its located variables (OpenPLC v3 convention -- **CONFIRMED Oct 15, 2026 on BSH-FireAlarm's container**: the Slave Devices page states "attached to address 100 onward" and listed the device at exactly these addresses. Verify per-system ranges on each system's own container):

| ESP32 data | Located address of item k |
|---|---|
| Discrete input k | `%IX100.k` (k = 0..7 in the first byte, then `%IX101.0` ...) |
| Coil k | `%QX100.k` |
| Input register k | `%IW(100+k)` |
| Holding register k (write) | `%QW(100+k)` |
| Holding register k (read) | `%IW(100+inputRegisterCount+k)` -- not used in this project |

Slave device sizes (counts) are set per device in OpenPLC's Slave Devices page. If a value is missing or stuck at 0, the count is too small.

## B. This PLC as a Modbus server (talking to SCADA), from the OpenPLC docs

| Modbus | Maps to |
|---|---|
| Coil n (FC 1/5/15) | `%QX` bit n, starting at `%QX0.0` (coil 8 = `%QX1.0`) |
| Discrete input n (FC 2) | `%IX` bit n, starting at `%IX0.0` |
| Input register n (FC 4) | `%IW` n, starting at `%IW0` |
| Holding register n (FC 3/6/16) | `%QW` first (one register each), then `%MW`, `%MD`, `%ML` |

Source: OpenPLC Editor documentation, "Modbus addressing" (checked Oct 11, 2026). The Editor's Modbus Server page also shows the full table computed for your current settings.

## C. Gotchas

- Modbus protocol addresses are 0-based. Many SCADA tools show 1-based or 0xxxx / 4xxxx notation, which looks like an off-by-one.
- Only holding registers mapped to `%QW` are writable from SCADA and forwarded to the ESP32.
- OpenPLC keeps answering with the last value when the ESP32 goes away. Only a counter the ESP32 increments (`HEARTBEAT`) proves it is alive.
