# BSH-PowerBox — Wokwi Simulation (Phase A0)

A working simulation of the whole BSH-PowerBox system: same GPIO pins, same
relay and button logic as the real bench sketch will use. Use it to learn
how each relay channel and its local override button behave, and how
everything is wired, all without touching real hardware.

Rule: `Firmware/WORKFLOW.md` Section 7. Modeled on
`Firmware/BSH-FireAlarm/Wokwi/`, the project's first simulation and
template for the other nine.

**Design change (Sep 28, 2026):** this node originally used DALI for
dimmable lighting, which Wokwi can't simulate natively (see the retired
`BSH_Sensors/12_DALI_LED_Driver.md`). DALI was dropped in favor of a plain
relay for lights — the same pattern as the fan — so unlike every other
system's Phase A0 folder, this one has **no stand-ins**: both relay
channels are native Wokwi parts (`wokwi-relay-module`), so this simulation
proves the whole node's logic, not just teaches it.

**Local override buttons (Sep 29, 2026):** added one `wokwi-pushbutton`
per channel (GPIO 4 for lights, GPIO 16 for fan), modeling a real physical
light switch alongside the Serial "hub command" stand-in. See
`BSH_Systems/04_BSH-PowerBox.md` Notes and the Sep 29, 2026 entry in
`Piranga_Workflow_v2.md` Section 14 for why.

## Files

| File | What it is |
|---|---|
| `diagram.json` | The parts and every wire. This is the wiring diagram, and Wokwi draws it for you |
| `sketch.ino` | The firmware. Same pin map, relay logic, and button-debounce handling as `BSH_PowerBox_Node.ino`, minus WiFi/Modbus — the "hub command" comes from the Serial Monitor instead of a Modbus coil write |
| `README.md` | This guide |

No `libraries.txt` needed, because the sketch uses only the ESP32 core.

## How to open it

1. Go to **wokwi.com** → *Start from scratch* (or *New Project*) → **ESP32**.
2. In the `sketch.ino` tab, delete everything and paste this folder's `sketch.ino`.
3. In the `diagram.json` tab, delete everything and paste this folder's `diagram.json`.
4. Press the green **▶ Play** button. The Serial Monitor opens underneath.
5. Optional: sign in and **Save** so the project gets its own link the team can reuse.

## What's in the circuit

| GPIO | Part in Wokwi | Real part | Notes |
|---|---|---|---|
| 13 | Relay module + LED "light" | 2-channel relay module (lights channel) + light fixture | Native — no stand-in needed |
| 15 | Relay module + LED "fan" | 2-channel relay module (fan channel) + fan | Native — no stand-in needed |
| 4 | Push button "light" | Local lights override switch | Native — `wokwi-pushbutton`, `INPUT_PULLUP` |
| 16 | Push button "fan" | Local fan override switch | Native — `wokwi-pushbutton`, `INPUT_PULLUP` |

Both relay modules' coil (VCC/GND/IN) and switched-contact (COM/NO) sides
share the ESP32's 5V/GND rails in the diagram, purely so the simulated
LEDs light up when each relay closes — on the real bench, whatever the
relay switches (the actual light fixture, the actual fan) lives on its own
power rail, physically separate from the ESP32's own supply (see
`BSH_Sensors/09_Relay_Module.md`). Each button wires straight to GND on the
other leg — no external resistor, since the firmware uses `INPUT_PULLUP`
(see `BSH_Sensors/02_Push_Button.md`).

## Scenarios to try

Start with nothing triggered: both LEDs off, and the Serial Monitor should
print a line like this once a second:

```
lights=off  fan=off
```

| # | Do this | You should see |
|---|---|---|
| 1 | Nothing (normal operation) | Both LEDs off, `lights=off  fan=off` |
| 2 | Type `l` + Enter in the Serial Monitor's input box | "light" LED turns on, prints `Lights command: ON`, then `lights=ON` |
| 3 | Type `l` + Enter again | Light turns back off |
| 4 | Type `f` + Enter | "fan" LED turns on independently of the light |
| 5 | Toggle both on at once | Both LEDs on together — the two channels are fully independent, same as the real Modbus coils `LIGHTS_ON_CMD`/`FAN_CMD` |
| 6 | Click the lights push button instead of typing `l` | Same effect as the Serial command — prints `Lights button pressed -> ON/OFF` |
| 7 | Click the fan push button | Same as `f`, but from the button instead of Serial |
| 8 | Turn lights on with `l`, then turn it off again with the button (or vice versa) | Either source can flip it at any time — in this Phase A0 sketch both write straight to the same variable, so there's no conflict to referee (that logic lives in the real firmware's Modbus coil edge-detection, see below) |

Things to notice while you play:
- **Relays energize on HIGH here, not LOW:** `RELAY_LIGHTS_ACTIVE_LOW` and
  `RELAY_FAN_ACTIVE_LOW` are both `false` in this sketch. That was NOT the
  original assumption — see "Relay polarity" below.
- **Independent channels:** lights and fan never affect each other, exactly
  like the real register map (`Coil0`/`Coil1`, no shared state).
- **No dimming:** there's no dim level to simulate any more — on/off is
  the whole story for lights now, same as the fan.
- **Buttons keep working with no "hub" involved at all:** unlike the real
  firmware, this sketch has no network to lose, so this scenario is best
  understood by reading `BSH_PowerBox_Node.ino`'s header comment on comm
  loss — the point stands the same either way: a physical switch doesn't
  care whether the network is up.

## Relay polarity — what we actually found (Sep 28–29, 2026)

The original assumption, based on how most cheap real relay modules work,
was `RELAY_ACTIVE_LOW = true` (LOW energizes the relay) for both channels.
Phase A0 testing disproved that for Wokwi's simulated part:

1. First, the lights channel (then on GPIO 12) showed inverted behavior —
   LED lit when the log said OFF, dark when it said ON. The initial theory
   was GPIO 12 being an ESP32 strapping pin (see the section below).
2. Moving lights to GPIO 13 (not a strapping pin) did **not** fix it — same
   inversion, and it showed up on the relay module's own onboard LED1
   indicator too, not just the external test LED. That ruled out both the
   strapping-pin theory and a wiring mistake in this diagram.
3. Testing the fan channel just as carefully (not just glancing at it)
   showed it was **also** inverted the whole time — it just hadn't been
   checked as closely as lights until then.

Conclusion: Wokwi's `wokwi-relay-module` energizes on **HIGH**, not LOW.
Both `RELAY_LIGHTS_ACTIVE_LOW` and `RELAY_FAN_ACTIVE_LOW` are `false` in
this sketch to match. **This is a simulator-only finding** — don't carry
`false` over to the real bench firmware (`BSH_PowerBox_Node.ino` keeps
`RELAY_ACTIVE_LOW = true`); verify the real relay module's polarity fresh
when you get there.

## GPIO 12 vs GPIO 13 -- why the lights channel isn't on GPIO 12

This system originally put the lights relay on GPIO 12 (carried over from
the old DALI TX pin). GPIO 12 is one of the ESP32's strapping pins (MTDI)
-- sampled at reset to pick the flash chip's operating voltage. A relay
module's IN line normally has its own pull-up resistor, and that pull-up
can drag a strapping pin like GPIO 12 to an unintended level during the
boot-time sampling window, before any sketch code runs. That's a real
hazard worth avoiding regardless -- moved lights to GPIO 13, a plain GPIO
with no boot-strapping role, and it's worth remembering for any future
system: never put a pulled-up peripheral (a relay IN, a switch, anything
with its own pull resistor) on GPIO 0, 2, 5, 12, or 15 without checking
that strap first.

**However:** the strapping-pin theory turned out not to be the (sole, or
even main) explanation for the inverted-lights symptom actually observed
here -- see "Relay polarity" above. The GPIO 12→13 move is still worth
keeping (it's a genuine, separate hazard), but it didn't fix the bug that
prompted it. The real cause was Wokwi's relay part being active-HIGH.

## Real bench differences

The simulation can't show these, so they still have to be checked on the
real bench:

- **Relay polarity:** don't assume the "Relay polarity" finding above
  carries over — real cheap relay modules vary, and most are actually
  active-LOW (the opposite of what Wokwi's part turned out to need).
  Verify against your specific module (see `BSH_Sensors/09_Relay_Module.md`).
- **Separate power rails:** the sim draws both the relay coils and the
  switched LEDs from the same 5V pin for simplicity. On the real bench,
  the light fixture and the fan each need their own supply, physically
  separated from the ESP32 — sharing rails on the bench risks browning out
  the ESP32 when the fan motor draws current.
- **Coil current:** each relay module channel draws ~70mA when energized.
  Two channels active at once is fine off USB power, but worth knowing if
  you add more relay-driven nodes to the same supply later.
- **Button debounce:** Wokwi's `wokwi-pushbutton` bounces cleanly enough
  that the ~50ms debounce rarely gets stressed. A real cheap tactile
  button can bounce worse — watch for a single physical press registering
  as more than one toggle on the bench, and increase `BUTTON_DEBOUNCE_MS`
  if so (see `BSH_Sensors/02_Push_Button.md`).

## Troubleshooting

- **Nothing responds to `l`/`f`:** make sure you're typing into the Serial
  Monitor's input box (not just watching the output) and pressing Enter —
  a bare keypress without Enter won't send anything.
- **Nothing responds to a button click:** make sure you're clicking the
  button graphic itself (not the LED or relay), and check the Serial
  Monitor for a `... button pressed ->` line to confirm the press
  registered at all.
- **LED or relay's onboard LED1 seems inverted (on when it should be off):**
  check both `RELAY_LIGHTS_ACTIVE_LOW` and `RELAY_FAN_ACTIVE_LOW` in
  `sketch.ino` are `false`. If they already are and it's still inverted,
  something else is off in the wiring — compare this folder's
  `diagram.json` against what's actually in your Wokwi tab; a stray manual
  edit there is easy to lose track of.
