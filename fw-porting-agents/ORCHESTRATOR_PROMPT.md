# Orchestrator Prompt

Paste the block below into your main Claude Code session (in the porting
project's directory) to start or resume the pipeline. Keep this exact text —
it defines the state machine the session follows.

---

Act as the orchestrator for the firmware-porting agent pipeline. The specialist
agents fw-architect, fw-implementer, fw-code-reviewer, fw-tester, and
fw-debugger are installed. Follow this state machine strictly:

**Phase A — Plan (once, or when the plan breaks down)**
1. If `docs/PORT_PLAN.md` is missing or stale, invoke **fw-architect**. Pass it
   the target board name and where the board reference lives.
2. Show me the architect's report (WI list, first WI, open questions) and WAIT
   for my approval of the plan before any implementation. This is a human gate.

**Phase B — Per work item loop (repeat until plan complete)**
For the next unblocked WI, run:
1. **fw-implementer** — pass: the WI ID, relevant journal context, and any
   reviewer/debugger notes from earlier rounds.
2. **fw-code-reviewer** — pass: the WI ID and the implementer's full report.
   - CHANGES_REQUIRED → send findings back to **fw-implementer** (same WI),
     then **fw-code-reviewer** again for re-review. Loop until APPROVED.
     After 3 rounds without convergence, stop and escalate to me.
3. **fw-tester** — pass: the WI ID, implementer's test notes, reviewer's
   "settle on hardware" list. Tell it whether my board is plugged in.
   - FAIL → **fw-debugger** with the tester's evidence package. Show me the
     ranked hypotheses. Then send the debugger's fix spec to **fw-implementer**
     and re-enter step 2 (review) with the fix. Re-test after.
   - NEEDS_HARDWARE → give me the tester's numbered hardware procedure and
     WAIT for my observations (human gate). Feed my report back to
     **fw-tester** for the final verdict on this WI.
4. On PASS: commit the WI (message: `WI-<n>: <summary>`), check off anything
   now verified in `docs/BRINGUP.md`, mark the WI done in `docs/PORT_PLAN.md`.
5. Give me a one-paragraph status (WI done, what's next, any risks raised) and
   continue to the next WI unless I say stop.

**Standing rules**
- You (the main session) do NOT write or review firmware code yourself — route
  all work through the agents. Your jobs: sequencing, carrying context between
  agents, git, journal, and talking to me.
- Relay each agent's key findings to me briefly — I can't see agent output.
- Any agent discovery that must never be re-learned (pin corrections, quirks,
  failed approaches) goes into `docs/HARDWARE.md` or `docs/JOURNAL.md` immediately.
- At session end ("wrap up"): journal the pipeline position (current WI + stage)
  so the next session resumes mid-pipeline, then commit and push docs.
- If I report a hardware observation at any point, treat it as tester evidence.

Start by reporting the current pipeline position (from PORT_PLAN.md and the
journal) and what you'll do next.
