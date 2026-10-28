# Wiring — DRAFT (BSH-PowerBox power rails)

**Draft, Oct 27, 2026. Nothing here was wired or tested.** Source for the as-built wiring this copies: `Firmware/BSH-PowerBox/Schematic/Connections.md`. Simulation of the same wiring: `Wokwi/`.

## Parts (assumed)

| Part | Qty (BSH) | Note |
|---|---|---|
| **Spare** ESP32 DevKit 30-pin (prototype PowerBox; **not** the real BSH-PowerBox at .14) | 1 | the real PowerBox stays exactly as built. Luis has a spare DevKit (reported Oct 27, 2026; model, pin count and USB connector not yet confirmed), so nothing to buy for this row |
| 1-channel 5V relay module | 4 new (3 rails + lights; lights kept, decided Oct 27, 2026) | all new boards; do not take the fan or lights relay off the real PowerBox. ~70 mA coil each (`BSH_Sensors/09_Relay_Module.md`) |
| USB Type-A male + female screw-terminal breakout | 3 pairs | one pair per node. A listing seen on Amazon is a 1 male + 1 female pair, "4 Pin Screw Terminal"; **not opened, not bought** |
| 3 LEDs + 3 series resistors, 1 push button | needed (lights kept): spare or new, none borrowed from the real PowerBox | the sim uses 220 ohm; the real design's resistor value is not recorded, so confirm LED current before choosing |
| Stand-in loads for the "other systems" | 3 | spare ESP32s, or cheap USB-powered stand-ins (USB LED lamp, USB fan). **Never** the real Elevator, FireAlarm or Door nodes |

## Per rail (one relay, one node)

| Relay pin | Goes to |
|---|---|
| IN | an ESP32 GPIO (**not chosen**; sim uses 25/26/27 as placeholders) |
| VCC / GND | ESP32 5V (VIN) / GND |
| COM | +5V terminal of the male breakout (the source side) |
| NO | +5V terminal of the female breakout (the node side) |
| NC | unused (fail-dark: NO contact, decided Oct 27, 2026) |

Breakout: source cable end into the male board, node cable into the female board. GND (and D+/D- if you want serial/flashing through) wired straight between the two boards. **Terminal labels must be read off the real board**; I have not seen one.

## Rules carried over from the real design

- Keep what a relay switches on its own supply, not the ESP32's (`BSH_Sensors/09_Relay_Module.md` Notes).
- Test each new relay board's polarity on its own. Do not copy the Oct 5 active-HIGH result (`Firmware/BSH-PowerBox/Lights_Relay/Wiring.md`).
- Relay GPIOs must avoid strapping pins (0, 2, 5, 12, 15), SPI flash pins (6-11, 16-17) and input-only pins (34-39): `References/ESP-IDF/02-api-reference/peripherals-api/gpio-rtc-gpio.md` lines 56-101. The as-built fan relay is on GPIO 15, a strapping pin.
- Common GND across everything the PowerBox touches, as in `Connections.md`.

## Check before buying or wiring

- [x] Contact rating (Oct 27, 2026): Luis sent a photo of a Songle `SRD-05VDC-SL-C` relay module; I read "10A 250VAC, 10A 125VAC, 10A 30VDC, 10A 28VDC" off the relay body. **Read from a photo; confirm it is the same module as the ones on the bench.** 5V USB loads are far below it.
- [x] USB connector at the board end (Luis reported Oct 27, 2026): BSH-FireAlarm is USB-C, the rest are micro-USB. BSH-FireAlarm's cable therefore ends in USB-C at the board; whether it behaves normally behind the in-line breakout is not tested. This does not change the breakout choice, since the breakout sits at the hub (Type-A) end.
- [x] What the nodes plug into (Luis reported Oct 27, 2026): two USB hubs, both plugged into a power extension (mains strip). Not yet known: whether the hubs are powered with their own adapter, and the per-port or total amps on their labels.
- [ ] Current draw of Elevator (L293D + motor), Door, FireAlarm — not measured
- [ ] Terminal labels on the breakout (5V, D-, D+, GND order)
- [ ] Whether the relay pins glitch at ESP32 reset (strapping/boot state)

## Alternatives considered (Oct 27, 2026)

VIN rail with jumper wires (back-feed unverified) · solid-state load switch (not in project) · software-only simulated blackout (does not power anything) · switching the charger or mains side (**rejected, unsafe**) · cutting the USB cable (works, destroys the cable).

## Power source for the nodes (shopping notes, Oct 27, 2026)

**Assumed unless stated; nothing bought or tested.**

- The nodes need only power from this source: Modbus runs over WiFi and flashing is done by plugging the node into the laptop. So a plain **multi-port USB-A charging station** can do the job as well as a data hub, and does not depend on a host computer being connected. A powered data hub is fine only if it supplies power to its ports without a host (not guaranteed; check the box or reviews).
- Look for: its own wall adapter, **4 or more Type-A ports** per building (the PowerBox plus three nodes), and a total rating comfortably above what the nodes draw. Node currents are **not measured**; as a rough shopping margin aim for about 3 A (15 W) total or more and 1 A or more per port. Avoid USB-C-only docks and bus-powered travel hubs with no adapter.
- The PowerBox's own relay coils add load on its port: each coil is about 70 mA (`BSH_Sensors/09_Relay_Module.md`), so lights plus three cut rails is about 280 mA on top of the ESP32. A port that only gives the old 500 mA USB 2.0 limit (general figure, not from the project docs) may be tight for the PowerBox.
- The PowerBox's own USB feed must not go through its own relays.

## Finding that affects the design: motor and servo supplies

`BSH_Sensors/01_L293D_Motor_Driver.md` (lines 11 and 22) says the Elevator motor needs an **external supply**, not the ESP32's pins. `BSH_Sensors/06_SG90_Servo.md` (lines 10-11) says the Door servo needs an **external 5V supply** (~500 mA stall). So cutting the ESP32's USB 5V would **not** cut the motor or servo supply unless that supply is also switched. **Luis reported (Oct 27, 2026):** the motor and servo are usually powered from power banks (not recorded in any doc; no further detail, and not seen by me). So Elevator and Door would have **two 5V feeds**: the hub or station port for the ESP32, and a power bank for the motor or servo. Open: decide whether a "rail" means the ESP32 only, or the ESP32 plus its motor/servo supply (options below).

### Side note, not part of the goal: what "rail" could mean for Elevator and Door (raised by the agent Oct 27, 2026; none tested)

| Option | What it switches | Cost / risk |
|---|---|---|
| A. ESP32 only | the hub-port feed to the ESP32 | 1 relay per node. The motor or servo keeps its power bank with no controller; the L293D inputs may float with the ESP32 off (not verified). A moving elevator motor could keep running |
| B. ESP32 + motor/servo bank | two relays per node (ESP32 feed, and the power bank's +5V line), both driven by the same command | Elevator and Door need 2 relays each. The Songle relay is single-pole, so one board cannot switch two separate sources. Power banks and hub ports must stay separate sources, never tied together |
| C. Everything from the station | move the motor/servo off the power banks onto charging-station ports, one switched feed | fewer relays, but I have not checked the current limits or whether tying feeds is safe; likely needs a separate port per feed |

FireAlarm has one feed in the docs read (the ESP32 and the MQ sensor on its 5V pin), so it is the simplest first rail. Power banks may shut their output off on a very light load (general knowledge, not tested); keep that in mind before using one as a switched source.

## Materials for the wiring (BSH first; general practice, nothing bought, nothing tested)

| Item | Qty (BSH) | Why |
|---|---|---|
| USB-A male + female screw-terminal breakout pair | 3 (+1 spare) | one pair per node; the relay sits between the two boards' +5V terminals |
| 1-channel 5V relay module, same Songle SRD-05VDC-SL-C type | 4 new; none taken from the real PowerBox | 3 rails + lights (decided Oct 27, 2026). Test each new board's polarity on its own |
| Hookup wire, stranded, about 22 AWG, red and black (about 2-3 m each) | 1 set | 5V at under about 1 A per rail; keep runs short. Check the gauge the relay and breakout screw terminals accept before buying |
| Female-to-male Dupont jumper wires | a pack | ESP32 GPIO / 5V / GND to each relay's IN / VCC / GND, as the as-built wiring does |
| Breadboard rail or small terminal strip | 1 | shares the ESP32's 5V and GND across four relay modules |
| Small screwdriver that fits the terminal screws | 1 | read the screw head type off the boards |
| Wire strippers, heat-shrink or tape, label tape | 1 | label each wire by rail |
| Multimeter | 1 | check 5V at each node side with the rail on and off, check continuity, and later measure current |

Wiring per rail: male breakout +5V to relay **COM**; relay **NO** to female breakout +5V; male breakout GND straight to female breakout GND (not through the relay). The two data terminals are not needed for power; leave them unconnected unless you want to flash through that cable (assumed). Work only on the 5V side; do not open or touch the hub's or power strip's mains side.
