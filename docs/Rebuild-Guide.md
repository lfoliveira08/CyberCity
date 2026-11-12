# Rebuild guide — one node, end to end

How to reproduce a single node. Source of truth for each layer: the per-system folders under [`../Firmware/`](../Firmware/), [`../PLC_Logic/`](../PLC_Logic/) and [`../SCADA_HMI/`](../SCADA_HMI/). This is an overview; follow the system's own bring-up checklist for the exact wiring and pins.

## 1. Hardware
Read the system's design doc in [`../BSH_Systems/`](../BSH_Systems/) or [`../FH_Systems/`](../FH_Systems/) and wire it per its `../Firmware/<System>/` folder and bring-up checklist. Component sheets are in [`../BSH_Sensors/`](../BSH_Sensors/) and [`../FH_Sensors/`](../FH_Sensors/).

Hardware gotchas (from `../00_START_HERE.md` §8, **verified** in the project's lessons):
- **GPIO 34–39 are input-only with no internal pull resistors** — `INPUT_PULLUP` there silently does nothing; use an external resistor.
- **GPIO 0, 2, 5, 12, 15 are strapping pins** — don't put a pulled-up peripheral on one without checking boot behaviour.
- **ADC2 is unreliable while Wi-Fi is on** — prefer ADC1 pins (32–39) for analog reads.
- Any 5 V digital output into a 3.3 V GPIO needs a divider.

## 2. Firmware
Copy the sketch from `../Firmware/<System>/`, set **your own** Wi-Fi name and password in it, and flash with the Arduino IDE.
- **Plain ASCII only in `.ino` files** — non-ASCII (em dashes) fails to compile on this project's setup.
- Never commit real credentials. The Wi-Fi password lives in the sketch only on the bench.
- The node must answer Modbus TCP on its static IP and expose `HEARTBEAT` + `RECONNECT_COUNT` as its last two Input Registers.

## 3. OpenPLC
Create the node's OpenPLC container, add the ESP32 as a Generic Modbus TCP slave, and load `../PLC_Logic/<System>/Program.st`. See [`../PLC_Logic/README.md`](../PLC_Logic/README.md). Confirm slave ID 0 and a rising heartbeat with a Modbus read from Kali.

## 4. SCADA (Ignition)
Add one Modbus device per container in the Gateway with **Zero-based Addressing ON**, then build the node's tags in the `<Bldg>/<System>/` folder layout. Every Ignition step is looked up in `../References/Ignition/` and cited. See [`SCADA.md`](SCADA.md).

## 5. Verify
A node is "running" when `Program.st` is uploaded, the PLC is started, a Kali read proves slave ID 0, and the heartbeat is rising. Record results in the system's `Build_Log.md`, labelled verified / reported / not checked.

Build order per building: Fire Alarm → Elevator (BSH) / HVAC (FH) → Door → Power Box. **One system active at a time.**
