# Alternate Design: One Real PLC for One System (FH-PowerBox)

**Status: exploratory only -- discussed with and agreed in direction by the
professor on Sep 28, 2026, but not yet folded into the real project plan.**

This folder holds a parallel version of the project workflow that explores
giving **one** system -- **FH-PowerBox** -- a real, physical PLC (an
Allen-Bradley MicroLogix 1400, CAT 1766-L32BXB, Series B) in place of its
OpenPLC Docker container, while the other nine systems stay exactly as
designed in `Piranga_Workflow_v2.md`.

**This is a revision of the original idea, not the original idea itself.**
The first draft of this folder explored replacing *all 10* OpenPLC instances
with one PLC. After discussing trade-offs, Luis and the professor agreed to
scope it down to **one real system as a live demo piece, alongside nine
software-emulated ones** -- so the project can show off a genuine piece of
industrial hardware without giving up the resilience/realism properties of
the per-system design. `Single_PLC_Workflow.md` has been rewritten to reflect
this narrower, hybrid scope.

**Nothing here changes or overrides `Piranga_Workflow_v2.md` or
`Firmware/WORKFLOW.md`, which remain fully authoritative for the actual
project.** This folder exists so Luis and the professor can review the idea
side by side with the real plan and decide whether/how to adopt it.

- `Single_PLC_Workflow.md` -- the parallel workflow document, describing only
  what changes for FH-PowerBox and confirming everything else stays as-is,
  plus a trade-off summary and open questions for the professor.

If this design is adopted, the changes should be folded into the real
`Piranga_Workflow_v2.md` deliberately (through Section 14's decision log, the
same way every other architecture change in this project has gone) -- not by
promoting this folder wholesale.
