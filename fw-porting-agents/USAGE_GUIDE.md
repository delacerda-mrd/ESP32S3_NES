# Usage Guide — Running a Port Through the Agent Pipeline

This walks through a real port end-to-end. Prerequisites: the five agents are
installed (see README.md), and the project was bootstrapped with
`firmware-porting-kit` (so `docs/BOARD_REFERENCE.md`, `docs/BRINGUP.md`,
`docs/JOURNAL.md` exist).

## The flow at a glance

```
 You                Orchestrator (main session)         Agents
 ───                ───────────────────────────         ──────
 "start the port" ─▶ checks docs, no plan yet ────────▶ fw-architect
                     ◀─ plan + WI list ─────────────────┘
 approve plan ────▶ GATE 1 (you approve/adjust plan)
                     picks WI-1 ──────────────────────▶ fw-implementer
                     ◀─ diff + build result ────────────┘
                     sends diff for review ───────────▶ fw-code-reviewer
                     ◀─ APPROVED or CHANGES_REQUIRED ───┘
                     (loops implement↔review until approved)
                     sends for test ──────────────────▶ fw-tester
                     ◀─ PASS / FAIL / NEEDS_HARDWARE ───┘
 run HW checklist ▶ GATE 2 (you observe the hardware)
                     on FAIL ─────────────────────────▶ fw-debugger
                     ◀─ ranked causes + fix spec ───────┘
                     (fix goes back through implement → re-review → re-test)
                     on PASS: commit, update BRINGUP/PORT_PLAN, next WI…
```

## Step by step

### 1. Start
```
cd <porting-project>  &&  claude
```
Paste the block from `ORCHESTRATOR_PROMPT.md`. The session reports pipeline
position and begins. On a fresh project it will call **fw-architect** first.

### 2. Gate 1 — approve the plan
The orchestrator shows you the architect's work-item list. Read it. Push back
freely ("split WI-3", "audio last, I care about video first") — reordering now
is cheap, mid-pipeline is not. Say "approved" to start implementation.

### 3. The loop runs
For each WI you'll see: implement → review (maybe a fix round) → test. You only
get pulled in when:
- **Review deadlocks** (3 rounds without APPROVED) — you arbitrate.
- **The tester needs hardware** — see Gate 2.
- **A WI completes** — you get a one-paragraph status; say "continue" or stop.

### 4. Gate 2 — hardware verification (your most important job)
The tester gives you numbered steps with *binary* expected observations:

> 1. Power-cycle the board. EXPECT: boot log on console within 3s, no reset loop.
> 2. Observe screen. EXPECT: full-screen red test pattern, no off-colored band
>    at any edge, no flicker.

Do the steps, report what you saw — including "step 2: left edge has a green
stripe". Exact symptoms are debugger fuel. Photos described in words are fine
("colors look swapped, red shows as blue" → that's an RGB/BGR finding).

### 5. When something fails
FAIL routes to **fw-debugger**, which returns ranked hypotheses, each with the
cheapest experiment to confirm or kill it. The orchestrator shows you these.
Sometimes the experiment is yours to run ("does the stripe move when rotation
changes?"); sometimes it's a code probe the implementer adds. Confirmed fixes
become a mini WI through the normal implement → review → test loop — no fix
skips review.

### 6. Pausing and resuming (any machine)
Stop anytime with **"wrap up"** — the orchestrator journals the exact pipeline
position (WI + stage), commits, pushes. To resume, even on the other machine:
`git pull`, start `claude`, paste the orchestrator prompt. It reads the journal
and picks up mid-pipeline.

## Practical tips

- **Trust the lane boundaries.** Don't ask the implementer "why is it broken?"
  (debugger's job) or tell the orchestrator to "just quickly fix it inline"
  (skips review — the pipeline's value is the independent verification).
- **Feed observations, not conclusions,** at Gate 2. "Screen white, backlight
  on, console says ILI9341 init OK" beats "the display driver is broken".
- **Cost control:** each agent invocation is a fresh context that reads the
  docs it needs. For a trivial WI (rename, comment fix) it's fine to tell the
  orchestrator "handle WI-7 inline, skip the pipeline".
- **The plan is living.** If a discovery invalidates it ("PSRAM too slow for
  double-buffering"), tell the orchestrator to re-run fw-architect to revise —
  it preserves WI IDs and completed status.
- **Journal is the safety net.** The debugger reads it to avoid re-proposing
  failed approaches; the orchestrator writes it so machine-switches are seamless.
  If in doubt whether something's journal-worthy, it is.

## FAQ

**Can I run steps manually without the orchestrator prompt?** Yes — in any
session: "use the fw-code-reviewer agent to review my current diff for WI-2".
The agents work standalone; the prompt just automates the sequencing.

**Where do agent definitions live?** User level: `~/.claude/agents/*.md`
(per machine — install on both Mac and Linux). Project level: `.claude/agents/`
(syncs via git with the repo).

**How do I change an agent's behavior?** Edit its `.md` file (or `/agents` →
select → edit), and update `AGENTS.txt` here so the master copy stays current.
