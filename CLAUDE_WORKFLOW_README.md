# Claude Code Session-Persistence System — README

This project uses a small doc system so you can put the project down for weeks
and resume without re-explaining anything to Claude.

## What's native vs. what's built here

**Native (Claude Code does this automatically):**
- **`CLAUDE.md`** — Claude Code auto-loads this file into context at the start of
  every session run from this directory. You never have to point Claude at it.
- **`claude --continue`** — resumes your most recent conversation in this directory,
  full transcript intact. **`claude --resume`** shows a picker of past sessions.
  Great for same-day continuation; less reliable weeks later when the old
  conversation is long, stale, or you've lost track of which session mattered.
- **Auto-memory** — Claude also keeps its own notes in `~/.claude/projects/.../memory/`,
  but that's Claude-managed and not visible/editable in your repo.

**Built for this project (because native tools don't keep a debugging journal):**
- `docs/JOURNAL.md` — running troubleshooting log + always-current status block
- `docs/HARDWARE.md` — durable board facts (pins, addresses, quirks)
- CLAUDE.md contains a *Session Start Protocol* instructing Claude to read the
  journal before doing anything — that's the glue that makes this automatic.

## How to use it (your part is tiny)

**Resuming after time away:** just start `claude` in this directory and state your
goal ("let's keep debugging the LCD"). CLAUDE.md auto-loads, tells Claude to read
the journal, and it picks up where the status block left off.

**Ending a work session:** say **"wrap up"** or **"update the journal"** before you
stop. Claude refreshes the Current Status block and appends a dated entry.
If you forget, ask at the start of the next session: "did we log last session?
Reconstruct from git diff if not."

## The files

| File | Purpose | Who updates it |
|------|---------|----------------|
| `CLAUDE.md` | Auto-loaded context: what the project is, commands, protocols | Claude, when stable facts change |
| `docs/JOURNAL.md` | Current status + dated log of every debugging session | Claude, end of each session |
| `docs/HARDWARE.md` | Pinout, I2C addresses, partition map, discovered quirks | Claude, when hardware facts are confirmed/corrected |
| `CLAUDE_WORKFLOW_README.md` | This file — how the system works | You/Claude, rarely |

## Rules that keep it working

1. **JOURNAL.md's status block is the single source of "where are we"** — if it
   disagrees with anything else, it wins and should be fixed.
2. **Never delete journal entries** — failed approaches are recorded precisely so
   they aren't retried.
3. **Durable facts migrate up**: a discovery starts as a journal entry, then gets
   written into HARDWARE.md (hardware) or CLAUDE.md (workflow/commands) once confirmed.
4. **Commit these files** along with code changes so the log travels with the repo.
