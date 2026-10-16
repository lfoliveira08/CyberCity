# Logs/ -- daily decision and lessons-learned log

*Created Oct 15, 2026. This is where every new decision and lesson goes. It continues
`Piranga_Workflow_v2.md` Section 14, which is frozen as history up to Oct 14, 2026.*

## What lives here

One file per working day, named `YYYY-MM-DD.md` (example: `2026-10-15.md`), plus
`INDEX.md`, which has one line per day, newest first. Nothing else goes in this folder.

## The rules

1. **Write it the day it happens**, not at the end of the project. A session that
   changes anything ends by writing or updating that day's file.
2. **Never delete or rewrite an old entry.** If something changes, add a new entry that
   says which earlier one it supersedes (by date and title). Old entries stay as they were.
3. **Every decision has a rationale.** What was chosen, why, and what was rejected.
4. **Every lesson has a resolution.** The symptom, the real cause, and the fix. A lesson
   that was only applied where it was found is half a lesson: say where else it applies.
5. **Label what is unverified.** If something was not bench-confirmed, say so. Never copy
   a result from one node to another without testing it there.
6. **Open items get their own section**, not a sentence buried in a rationale. Close them
   in a later day's file by naming the item and the date it was opened.
7. **Retired files go to `Archive/`** with a row in `Archive/README.md`, and the retirement
   is logged here with the reason.
8. **A day with no decision and no lesson still gets a short file** ("Done today" only),
   so the record has no gaps.
9. **Update `INDEX.md` in the same sitting** as the daily file.
10. **This folder wins over memory.** If a chat, a Claude project doc and this folder
    disagree, this folder (like the rest of the CyberCity folder) is the source of truth.

## Daily file template

```
# YYYY-MM-DD

## Done today
- What changed, one line each. Name the files or nodes.

## Decisions
| Decision | Rationale |
|---|---|
| ... | why, and what was rejected |

## Lessons learned
| Issue | Resolution | Applies to |
|---|---|---|
| symptom and real cause | the fix | node, file or "all nodes" |

## Open items
- [ ] Something unresolved (opened YYYY-MM-DD)
- [x] Closed item (opened YYYY-MM-DD, closed today)

## Supersedes
- Earlier entry (date, title) that this day changes, if any.
```

Sections with nothing to say can be left out, except "Done today".

## Relationship to other records

| Record | Role |
|---|---|
| `Piranga_Workflow_v2.md` Section 14 | Frozen history, Sep 2 - Oct 14, 2026. Many docs cite it by date; do not move its rows. |
| `Logs/` | Everything from Oct 15, 2026 on. |
| `Proxmox_Build_Log.md` | Step-by-step infrastructure record. Log the decision here, and link to it for detail. |
| Per-component `Build_Log.md` | Bench detail for one component. Log the lesson here, and link to it for detail. |
| `Next_Steps.md` | The current handoff (what to do next). The log is the history (what happened). |
