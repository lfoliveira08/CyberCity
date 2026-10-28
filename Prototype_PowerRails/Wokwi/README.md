# BSH-PowerBox — PROPOSAL: power rails (Wokwi)

**Status: DRAFT proposal, opened Oct 27, 2026. Ran in Wokwi Oct 27, 2026 (boot and lights button seen in Luis's screenshots; rail cut tests still pending). Not approved. Not the as-built node.** The as-built simulation (lights + fan) is `Firmware/BSH-PowerBox/Wokwi/` and is untouched. Nothing here changes firmware, PLC, SCADA or any register.

## The idea

The PowerBox stops driving a fan. Three relay channels each switch the USB +5V of one other BSH node (Elevator, FireAlarm, Door). Lights stay as they are. See `Logs/2026-10-27.md` (evening sections) for the discussion and open decisions.

## How to open it

1. wokwi.com → new ESP32 project.
2. Replace the project's `diagram.json` and `sketch.ino` with the two files in this folder.
3. Start the simulation, open the Serial Monitor.

**Run result (Oct 27, 2026, from two screenshots Luis sent):** the diagram and sketch loaded and ran with no error. The Serial Monitor showed the status line `lights=off Elevator=powered FireAlarm=powered Door=powered` every second, and after clicking the lights button, `Lights button pressed -> ON` and `lights=ON`. **Third screenshot (same day):** the Serial Monitor shows rails 1, 2 and 3 each cut (`CUT`, status `Elevator=CUT FireAlarm=CUT Door=CUT`) and then each restored (`POWERED`), so the sketch logic for `1`, `2`, `3` works. **Fourth screenshot (rail 1 cut, zoomed):** the Serial Monitor says `Elevator=CUT`; the Elevator LED looks dull and the Elevator relay's onboard `LED1` looks lit, which fits "coil energised, NC contact open". **Fifth screenshot (rail 1 restored, same zoom):** the Elevator LED glows bright and the Elevator relay's onboard `LED1` is off. Compared with the fourth screenshot this confirms rail 1: cut = LED dull + relay LED1 lit, powered = LED bright + relay LED1 off (Luis's screenshots, read by me). **Luis reported (no screenshot, not seen by me):** `a` cut all three rails and every rail LED went dark; `r` restored all three and every LED came back. Treat the FireAlarm and Door LEDs and `a` / `r` as reported working, not verified. Not tested: any behaviour outside Wokwi.

**Flipped to NO on Oct 27, 2026 (Luis's decision: PowerBox off means the other systems are off).** Everything reported above was seen with the **NC** version. The `diagram.json` and `sketch.ino` in this folder are now the **NO** version and have **not been re-run**. What changes when you run it: at boot the sketch energises the three rail coils (sim default `RAILS_ON_AT_BOOT`), so each relay board's onboard `LED1` is **lit while its rail is powered** and **off while cut**, the opposite of the NC observations. The node LEDs should still be lit when powered and dark when cut. Re-test `1`, `2`, `3`, `a`, `r` and report. **Luis reported (Oct 27, 2026, no screenshot, not seen by me): the NO version ran fine.** Treat as reported, not verified.

## What to try

| Type + Enter | Expected |
|---|---|
| (nothing, at boot) | lights off; Elevator, FireAlarm, Door LEDs **lit** (rails powered) |
| `1` | Elevator LED goes dark (rail cut); `1` again restores it |
| `2` / `3` | same for FireAlarm / Door |
| `a` | all three LEDs dark |
| `r` | all three lit again |
| `l` or click the yellow button | lights LED toggles |

## What each wire means (the wiring you would build)

Per rail, copying the as-built fan relay (`Firmware/BSH-PowerBox/Schematic/Connections.md` rows 5–9) but with a different load:

| Relay pin | Simulation | Real bench |
|---|---|---|
| IN | GPIO 25 / 26 / 27 | an ESP32 GPIO (pin not chosen for the bench) |
| VCC | ESP32 5V | ESP32 5V (VIN) |
| GND | ESP32 GND | ESP32 GND |
| COM | ESP32 5V (stand-in for the supply) | + of the **USB 5V source** for that node |
| NO | LED → resistor → GND ("node has power") | the node's USB cable +5V going to its board |
| NC | unused | unused (fail-dark: NO contact, decided Oct 27) |

Real build idea (**assumed, not tested**): put a USB breakout with screw terminals between each node's USB source and its cable, and route only the +5V through the relay. Ground and data stay connected.

## Real bench differences (simulation hides these)

- Wokwi has one 5V; the real rail needs its own USB source per node, separate from the PowerBox's own supply (`BSH_Sensors/09_Relay_Module.md` Notes).
- Current draw and relay contact rating are not modelled; neither has been measured or read (open).
- Whether the ESP32 pins glitch during boot, which could flick a relay, is not modelled and not tested.
- Whether the DevKit can back-feed when USB and another 5V are both present: not verified.
- Relay polarity: Wokwi energises on HIGH; test each real board separately.
- No Modbus, WiFi, heartbeat, status inputs or local override buttons for the rails here. Those are later phases and not designed yet.

## Open decisions (Luis)

- Go / no-go, and before or after SCADA.
- ~~NC vs NO~~: decided NO (fail-dark), Oct 27. Still open: how rails come on after power-up (decision 8).
- Lights relay stays (assumed here) or goes.
- Local override buttons for the rails: yes / no.
- Real GPIOs for the three rails.
