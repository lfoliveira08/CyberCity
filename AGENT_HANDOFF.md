# AGENT HANDOFF — workspace repair tasks

**Scope: this file is only about repairing the workspace itself** — the files and connections that let an agent work here correctly without being corrected. It is not a project plan.

| Question | File that owns it |
|---|---|
| What are the rules for working here? | `CLAUDE.md` |
| Where does the project stand? What project items are open? | `STATUS.md` |
| What is the next *build* step (tags, screens, PLC work)? | `Next_Steps.md` |
| What has to be fixed about the **workspace**? | **this file** |

Opened Oct 27, 2026. Luis works in **Claude Code**, in this folder, from Oct 27, 2026 on.

**How to use this file:** do the tasks in priority order. Each has a verification step — run it and paste the result before claiming the task is done. When a task is finished, tick it here, and log it in `Logs/YYYY-MM-DD.md` per `Logs/README.md`. Do not delete a finished task; tick it so the next agent knows it was handled.

---

## T1 — Make git reachable from the agent shell · HIGH · Luis, then agent

**Why:** `.git` here is a gitlink to `C:/Users/lfoli/git-data/cybercity.git`, which is outside the connected folders. Every `git` command fails from the agent shell, so an agent cannot see history, diff a change, or recover a file it overwrote. `Logs/` is the only safety net right now.

**Luis:** in the Claude desktop app, connect `C:\Users\lfoli\git-data` as a folder for the session, the same way `CyberCity` and `References\Ignition` are connected.

**Agent, once connected — verify, do not assume:**
```bash
git --git-dir=$HOME/mnt/git-data/cybercity.git --work-tree=$HOME/mnt/CyberCity status --short | head
git --git-dir=$HOME/mnt/git-data/cybercity.git --work-tree=$HOME/mnt/CyberCity log --oneline -5
```
**Done when:** both commands return real output. Then add the working form of the command to `CLAUDE.md` §6 and delete the "not reachable" warning there. This command has **never been run successfully** — it is derived, not tested. If it fails, report the exact error rather than improvising a second repo.

- [ ] T1 done

---

## T2 — Kill the ten-node contradiction in live docs · HIGH · agent

**Why:** the project has **eight** nodes. Two live reference notes still say ten, and an agent that reads one states the wrong count with confidence.

**Files and lines (verified Oct 27, 2026):**
- `References/GitHub/AllNodes_emelianov_modbus-esp8266.md` line 4 (`all 10 nodes`) and line 19 (`across all 10 nodes`)
- `References/GitHub/FireAlarm_JordyMail.md` line 17 (`this project's ten nodes`)

Change ten/10 to eight/8 in those three places only. Do not reword anything else in the notes.

**Do NOT touch:**
- `Piranga_Workflow_v2.md` line 432 — it is inside the Section 14 decisions log, frozen history through Oct 14, 2026, and was correct on Sep 10. Leave it.
- Anything in `Archive/` — retired by decision, kept as history.

**Verify:**
```bash
grep -rn "10 nodes\|ten nodes" --include=*.md . --exclude-dir=Ignition --exclude-dir=Archive
```
**Done when:** that returns only the `Piranga_Workflow_v2.md` line 432 hit.

- [x] T2 done (Oct 27, 2026 — three edits made; the "done when" condition as written cannot be met, see Logs/2026-10-27.md)

---

## T3 — Fix the stale header date on 00_START_HERE.md · MEDIUM · agent

**Why:** line 1 reads "as of Oct 5, 2026" while the body carries content through Oct 27. A reader cannot tell whether the file is current, so they discount all of it.

Change the line-1 date to the date of its newest content, or drop the date from the title and let `STATUS.md` carry dates. Do not restate status in the title.

**Done when:** `head -1 00_START_HERE.md` no longer claims a date older than the file's newest content.

- [x] T3 done (Oct 27, 2026 — date dropped from the title)

---

## T4 — Collapse the duplicated per-system status lines · MEDIUM · agent, after Luis says go

**Why:** 9 per-system READMEs carry their own `**Status (date):**` paragraph. They were accurate when written and will drift from `STATUS.md`, which is now the single home for status. Two sources of status is the problem we just removed at the top level.

**Files:** `PLC_Logic/*/README.md` (8 of 9 carry one) and `SCADA_HMI/*/README.md`.

**What to do:** replace each `**Status (date):**` paragraph with one line — `Status: see ../../STATUS.md. Build detail: Build_Log.md.` — and move any fact that exists **only** in that paragraph into the system's `Build_Log.md` first, so nothing is lost. Several of those paragraphs hold real detail (open items, what was not shown, the compile-log gaps); read before deleting.

**Ask Luis before starting.** It edits 17 files and is the one task here that destroys text.

**Done when:** `grep -rn "Status (" PLC_Logic/*/README.md SCADA_HMI/*/README.md` returns nothing, and every fact removed is findable in a `Build_Log.md`.

- [ ] T4 approved by Luis   - [ ] T4 done

---

## T5 — Decide the Claude-project mirror policy · MEDIUM · Luis decides, agent executes

**Why:** the claude.ai project "Cyber CIty" holds 138 docs that mirror this folder and are now behind it — `00_START_HERE.md`, `References/INDEX.md`, both workflow docs and all the system docs were changed here on Oct 27. The precedence rule says this folder wins, so nothing breaks, but a stale mirror is the largest remaining source of wrong answers. It matters less now that work happens in Claude Code, which reads this folder, and it still matters whenever the project is opened in claude.ai.

**Options for Luis:**
- **Stop mirroring.** Keep in the project only what exists nowhere else, and let the project instructions point at this folder. Fewest moving parts.
- **Re-sync on a schedule.** Push the changed docs up after each working session. More work, keeps claude.ai useful on its own.
- **Leave it.** Accept the drift and rely on the precedence rule.

**Also Luis-only, whichever is chosen:** the project **description** field still says 10 ESP32 nodes and names an old SCADA choice. It can only be edited by hand in claude.ai. Low priority while work is in Claude Code; wrong either way.

- [ ] T5 decided by Luis: ____________   - [ ] executed

---

## T6 — Session-start self-check · LOW · any agent, any session

Run this before starting work. It proves the workspace is intact and nothing silently moved.

```bash
cd ~/mnt/CyberCity
ls CLAUDE.md STATUS.md AGENT_HANDOFF.md Next_Steps.md 00_START_HERE.md >/dev/null && echo "entry points OK"
ls Firmware/CLAUDE.md PLC_Logic/CLAUDE.md SCADA_HMI/CLAUDE.md >/dev/null && echo "layer rules OK"
find References/Ignition -name '*.md' | wc -l    # expect 1572 (1571 pages + README)
head -1 STATUS.md                                # how stale is status?
ls Logs/ | tail -3                                # is the last working day logged?
grep -rn "10 nodes\|ten nodes" --include=*.md . --exclude-dir=Ignition --exclude-dir=Archive | wc -l   # expect 1 after T2
```

If `STATUS.md` is older than the newest file in `Logs/`, status is stale: read the newer logs and update `STATUS.md` before answering any "where are we" question.

---

## Rules for whoever works these tasks

- **Verify, do not assume.** Every task above has a verification command. Run it.
- **Never edit `References/Ignition/`.** It is an unmodified vendor copy. If a page is wrong or missing, re-pull it (method is in that folder's `README.md`).
- **Never edit `Archive/` or build from it.** Retired by decision.
- **`Piranga_Workflow_v2.md` Section 14 is frozen** through Oct 14, 2026. Many docs cite its rows by date. Do not move, renumber or correct them; supersede in `Logs/` instead.
- **Log the same day.** A session that changes anything writes `Logs/YYYY-MM-DD.md` and a row in `Logs/INDEX.md`.
- **One task at a time**, and say what you did not check.
