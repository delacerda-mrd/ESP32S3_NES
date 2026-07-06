# Firmware Porting Agent Pipeline — Setup

Five specialist agents that run a firmware port as a pipeline:

```
                    ┌─────────────────────────────────────────┐
                    │   ORCHESTRATOR (your main Claude session) │
                    └─────────────────────────────────────────┘
                         │ calls each agent, passes outputs
   ┌──────────────┬──────────────┬──────────────┬──────────────┐
   ▼              ▼              ▼              ▼              ▼
fw-architect  fw-implementer  fw-code-      fw-tester     fw-debugger
(PORT_PLAN)   (one WI at      reviewer      (build/flash/  (root cause
              a time)         (verdict)      procedure)     on FAIL)
```

Flow per work item: **implement → review → (fix → re-review) → test →
(debug → fix → re-review → re-test) → human hardware check → commit → journal.**

## Why the orchestrator is your main session, not a sixth agent

Claude Code subagents cannot spawn other subagents — only the main conversation
can call the Agent tool. So the main session IS the orchestrator: it holds the
pipeline state, invokes each specialist, carries each agent's report into the
next agent's prompt, and stops at the human gates (plan approval, on-hardware
verification). `ORCHESTRATOR_PROMPT.md` is the prompt that puts it in that role.

## Creating the agents (one-time, ~5 minutes)

1. In any Claude Code session, run `/agents`.
2. Choose **Create new agent**.
3. Choose scope:
   - **User level (`~/.claude/agents/`) — recommended**: agents work in every
     project on this machine. You'll do this once per machine (Mac and Linux).
   - Project level (`.claude/agents/`): travels with one repo via git.
4. When the editor opens, paste ONE agent's block from `AGENTS.txt` — everything
   between `--- BEGIN <name> ---` and `--- END <name> ---` (the block already
   contains the `---` frontmatter with name/description/tools; keep it intact).
5. Save. Repeat steps 2–4 for all five agents.
6. Verify: run `/agents` again — all five should be listed.

Alternative (faster): create the files directly —

```bash
mkdir -p ~/.claude/agents
# then save each block from AGENTS.txt as:
#   ~/.claude/agents/fw-architect.md
#   ~/.claude/agents/fw-implementer.md
#   ~/.claude/agents/fw-code-reviewer.md
#   ~/.claude/agents/fw-tester.md
#   ~/.claude/agents/fw-debugger.md
```

Ask Claude to do this for you: "create the five agents from
fw-porting-agents/AGENTS.txt at user level". Remember to repeat on your other
machine (user-level agents don't sync through the repo — or use project level
if you want them to).

## Files in this folder

| File | Purpose |
|------|---------|
| `AGENTS.txt` | The five agent definitions, copy/paste-ready |
| `README.md` | This file — setup |
| `ORCHESTRATOR_PROMPT.md` | Paste into a session to start/resume the pipeline |
| `USAGE_GUIDE.md` | How a full port actually runs, step by step, with the human gates |

## Design decisions (improvements over the raw idea)

- **One work item per implementation pass** — small diffs keep the review and
  test stages meaningful; "port the whole thing" passes make every stage rubber-stamp.
- **A dedicated debugger agent** — a failed test needs root-cause analysis and a
  discriminating experiment, not a blind re-implementation. The debugger is
  forbidden from fixing; the implementer is forbidden from diagnosing. Clean handoffs.
- **Reviewer re-verifies every hardware constant against the board reference** —
  the reviewer never trusts the implementer's pin numbers. Independent
  verification is the whole point of separate agents.
- **Explicit human gates** — agents can build, flash, and read logs, but only you
  can see the display or hear the speaker. The tester's job includes writing you
  a binary-observable checklist instead of pretending it verified the hardware.
- **Journal integration** — the orchestrator records outcomes in `docs/JOURNAL.md`
  (the cross-machine memory), so the pipeline survives session boundaries.
