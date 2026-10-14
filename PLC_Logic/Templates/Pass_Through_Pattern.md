# The pass-through pattern

Every system uses the same three kinds of rungs. Learn them once and you can write any of the 8 programs.

1. **Forward a command.** A SCADA-written bit goes to the ESP32's coil.

```
   %QX0.0                                                  %QX100.0
 +----[ ]---------------------------------------------------( )---+
    SCADA_ALARM_ACTIVATE                                   ALARM_ACTIVATE
```

2. **Publish a status bit.** An ESP32 discrete input is copied to a bit SCADA can read.

```
   %IX100.0                                                %QX0.2
 +----[ ]---------------------------------------------------( )---+
    MANUAL_PULL_TRIGGERED                                  PUB_MANUAL_PULL_TRIGGERED
```

3. **Publish a register value.** An ESP32 input register is copied to a holding register SCADA can read. In OpenPLC Editor use a MOVE block; leave EN unconnected or tie it to TRUE.

Optional fourth kind, used where a system has a signature attack demo:

4. **Compute a flag** from several bits (series contacts = AND, parallel contacts = OR, an inverted contact = NOT). See `BSH-FireAlarm/Ladder_Diagram.md` Rung 2 and `BSH-PowerBox/Ladder_Diagram.md`.

Why publish copies at all: the docs define exactly where the PLC's own Modbus server puts its coils (`%QX`) and holding registers (`%QW`). Copying everything into those makes SCADA's addresses certain, without depending on how slave-device data is exposed.
