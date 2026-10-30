# Lights Relay — Build Log (BSH-PowerBox)

> Log it: anything decided, learned or changed here is also recorded in that day's file in `Logs/` (rules: `Logs/README.md`).

Formerly `DALI_LED_Driver/Build_Log.md` — this channel now drives a plain
relay instead of a DALI ballast (design change, Sep 28, 2026; see
`claude/BSH_Systems/04_BSH-PowerBox.md` Notes). As of Sep 30, 2026, this
relay is its own dedicated 1-channel module, no longer the second channel
of a module shared with the fan -- see the Sep 30, 2026 entry in
`Piranga_Workflow_v2.md` Section 14.

## Status
- [x] Simulated in Wokwi first (Phase A0 — see `../Wokwi/README.md`) — date: Sep 28–29, 2026
- [x] Component in hand (own 1-channel relay module as of Sep 30, 2026)
- [x] Wired per `Wiring.md`
- [x] Bench-tested standalone (see checklist in `Wiring.md`) -- confirmed Oct 5, 2026
- [ ] Confirmed working with the real `BSH_PowerBox_Node.ino` firmware
      (Modbus `LIGHTS_ON_CMD` coil write turns the light on/off) -- pending Phase B

## Date log
| Date | What happened |
|---|---|
| Sep 28, 2026 | Wokwi Phase A0 built from scratch with lights on GPIO 12 (carried over from the old DALI TX pin). Lights LED showed inverted behavior — lit when the command log said OFF, dark when it said ON. Fan (GPIO 15, identical logic) worked correctly |
| Sep 28, 2026 | Suspected GPIO 12's strapping-pin role (MTDI, samples flash voltage at reset) — a relay module's IN pull-up could drag it HIGH during the boot window. Moved lights to GPIO 13 (not a strapping pin) across firmware, diagram, and docs |
| Sep 29, 2026 | GPIO 13 did **not** fix it — same inversion, confirmed on the relay module's own onboard LED1 indicator, not just the external test LED. This ruled out the strapping-pin theory and a diagram wiring mistake (fan's circuit was checked line-for-line against lights' and found structurally identical) |
| Sep 29, 2026 | Checking the fan channel just as carefully as lights (not just glancing at it) showed fan was **also** inverted the whole time — never actually correct, just not scrutinized as closely until lights was fixed and compared against it. Root cause: Wokwi's simulated `wokwi-relay-module` energizes on HIGH, not LOW, on both channels. Set `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` to `false` (active-HIGH) in the Wokwi sketch only |
| Sep 29, 2026 | Added a local override push button (GPIO 4) alongside the relay, modeling a real light switch. Real firmware given its own authority rule: a button press wins immediately, the hub only regains control on a *fresh* coil write (not by holding its last value), and actual state reports back honestly via a new `LIGHTS_STATUS` Discrete Input |
| Sep 29, 2026 | First version of the button code used a shared `ButtonState` struct passed by reference. Wokwi build failed: `'ButtonState' was not declared in this scope` — the Arduino/Wokwi build auto-generates function prototypes and inserts them ahead of any struct the sketch defines, which breaks for a function taking a custom struct by reference. Fixed by replacing the struct with plain `int`/`unsigned long` variables per button, passed by reference instead. Same fix applied to `BSH_PowerBox_Node.ino`/`FH_PowerBox_Node.ino` before it could bite on real hardware |
| Sep 29, 2026 | With the polarity fix and the struct fix both in, Wokwi build passed and lights/fan/both buttons all behaved correctly (LED state matches the Serial log; button press toggles the right channel; Phase A0 checklist boxes checked off) |
| Sep 30, 2026 | Switched from a single 2-channel relay module to two separate 1-channel relay modules, one per channel -- see the Sep 30, 2026 decision-log entry in `Piranga_Workflow_v2.md` Section 14. This channel now has its own dedicated relay board with its own VCC/GND. Real firmware's `RELAY_ACTIVE_LOW` split into independent `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` flags. This channel's own Phase A bench test (Step 2) has not happened yet -- Step 1 (fan alone) went first and found the old fan relay module active-HIGH, but that result is for a different, now-retired board and says nothing about this one |
| Oct 5, 2026 | **Bench-confirmed on the new 1-channel lights relay board.** Verified as part of the Phase A Step 4 combined standalone integration sketch (both relays + both override buttons together, no WiFi/Modbus), rather than as a separate lights-alone run -- see `../Fan_Relay/Build_Log.md` for the matching fan-channel entry and the "wrong sketch flashed" episode that preceded this result (an earlier flash attempt used an old lights-only sketch, which looked like dead fan/button hardware; re-flashing the correct combined sketch resolved it). With the right sketch running, `RELAY_LIGHTS_ACTIVE_LOW = false` (active-HIGH) made the 3-LED lights load match the Serial log on both transitions, and the lights override button correctly toggled only the lights channel |

## Lessons Learned
- **A bench polarity result belongs to the physical board it was tested on, not to "the lights channel" in the abstract.** Swapping relay hardware means re-testing, even though the GPIO pin and firmware logic didn't change.
- **Don't trust a partial fix.** Moving GPIO 12→13 looked like it explained the symptom (it's a real, documented hazard worth keeping anyway), but the actual bug was still there afterward. The tell was checking the relay's own onboard indicator LED, not just the external test LED — that's what proved it wasn't a wiring-loop problem specific to the external LED.
- **Check the thing you assume is fine, too.** Fan was assumed correct because nobody complained about it — it was just as broken as lights, just unnoticed. Once lights got real scrutiny, fan needed the same scrutiny before trusting it.
- **Wokwi's relay polarity is not the same as most real relay modules.** `wokwi-relay-module` energizes on HIGH in this simulator; don't assume that carries over to the bench build (see `Wiring.md`'s active-LOW note, unchanged).
- **A custom struct passed by reference to a function can break Arduino/Wokwi's auto-prototype generation.** If a compile error says a type "was not declared in this scope" for a struct that's clearly defined earlier in the file, this is the likely cause — replace the struct with plain-typed reference parameters.
- **A "no Serial output at all" symptom for a whole channel (fan and buttons, in this case) is more likely the wrong sketch flashed than a hardware fault** -- a `Serial.print()` right after a relay-set or button-edge call always runs if that code path is reached at all.

## Troubleshooting Tips
- LED or relay's onboard LED1 lit when it should be off (or vice versa): check `RELAY_LIGHTS_ACTIVE_LOW` (real firmware, its own flag as of Sep 30, 2026, confirmed `false` on Oct 5, 2026) or `RELAY_LIGHTS_ACTIVE_LOW`/`RELAY_FAN_ACTIVE_LOW` (Wokwi sketch) match your actual module/simulator — don't assume one matches the other.
- A build error naming a struct as "not declared in this scope" even though it's defined in the file: likely the Arduino auto-prototype issue above, not a real missing declaration.
- Zero Serial output at all for a button or relay command: confirm the flashed sketch is the one you think it is before suspecting wiring or GPIO strapping pins.
