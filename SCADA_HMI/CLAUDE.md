# SCADA_HMI -- rules for any agent working here

This folder is about Ignition 8.3.10. **Ground truth for how Ignition works is the local manual at `../References/Ignition/`** (read its `README.md` first: it has the rule and a topic-to-folder map).

Before you write ANY Ignition instruction (Gateway settings, device/driver setup, tags, UDTs, tag import JSON, Perspective, alarms, scripting, backups):

1. Search `../References/Ignition/` for the relevant page and read it. Do not rely on memory.
2. Cite the page path you used in your answer and in the system's `Build_Log.md`.
3. If the manual does not cover it, say "not in the manual" instead of guessing.
4. Never edit `../References/Ignition/`.

Project-specific facts (IPs, register blocks, tag naming) come from `README.md` in this folder and `../PLC_Logic/<System>/Register_Map.md`, not from the manual.

Luis builds the SCADA himself in the Designer, guided step by step so he learns it: explain each step and why, point to the manual page, and let him do the clicking.
