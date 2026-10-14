# Claude outputs — download inbox, not a project folder

**Nothing in this folder is part of the project.** It is where files generated
in a chat land when they are saved to disk. Treat it as an inbox: a file sitting
here is a copy in transit, not a deliverable, and it has no authority over the
copy in the project tree.

## The rule

Move a file to its real home in the same sitting, then let this folder go back
to being empty. `Firmware/WORKFLOW.md` Section 8 lists where every kind of file
belongs — firmware, simulations, wiring notes, bench tests, reports.

If a file here duplicates something in the project tree, the project tree wins.
If it is an older or abandoned variant worth keeping, it goes to `Archive/` with
a row in `Archive/README.md` explaining why — never left here, and never saved
beside the live file under a name like `..._Node-1.ino`.

## Why this file exists

An audit on Sep 26, 2026 found six files parked here, referenced by no document
in the project:

- three component bench-test sketches (elevator push buttons, perimeter PIR,
  perimeter ultrasonic) — real, useful work that the project structure had no
  home for. They now live in their component folders as `BenchTest.ino`, and
  `Firmware/WORKFLOW.md` Section 3 was amended to allow that file.
- two near-identical copies of the elevator node firmware, one of which used an
  abandoned L293D pin mapping (ENABLE 13 / DIRA 14 / DIRB 26 instead of the
  bench-confirmed 18/19/5). That is the dangerous case: a file that looks
  current, sits beside a correct copy, and would have been wired wrong. Both are
  in `Archive/` now.
- an unreferenced PNG preview of a wiring diagram whose editable SVG is in the
  Elevator's folder. Also archived.

The folder is kept rather than deleted because the desktop app may still be
configured to save here. Keeping it empty is the point.
