# {{PROJECT_NAME}} — Project Context for Claude Code

## SESSION START PROTOCOL (do this first, every session)
1. `git pull` — the user works from multiple machines; the journal on disk may be
   stale. On journal conflict: keep both sides' entries, merge Current Status by date.
2. Read `docs/JOURNAL.md` — the Current Status block says exactly where we are.
3. Read `docs/BRINGUP.md` when deciding what to work on next.
4. Read `docs/HARDWARE.md` only when the task touches pins/buses/peripherals.
5. Do NOT re-derive project context by exploring; these docs are the source of truth.

## SESSION END PROTOCOL ("wrap up" / "save state")
1. Update `docs/JOURNAL.md`: refresh Current Status, append a dated entry noting
   which machine the session ran on.
2. Update `docs/BRINGUP.md` checkboxes for anything verified ON HARDWARE this session.
3. Commit docs updates and push to origin (code changes only if the user asked).

## What this project is
Port of {{FIRMWARE_NAME}} ({{UPSTREAM_ORIGIN}}) to **{{BOARD_NAME}}**:
{{MCU_VARIANT}}, {{FLASH_DESC}}, {{RAM_DESC}}.
Key peripherals: {{PERIPHERAL_SUMMARY_ONE_LINE}}.
Toolchain: {{SDK_NAME_AND_EXACT_VERSION}}.

## Layout
{{2-6 bullet map of the source tree: where hardware glue lives vs. portable core,
where configs live. Point at directories, not every file.}}

## Build / flash / debug
```bash
{{BUILD_COMMANDS}}
{{FLASH_COMMANDS}}
{{MONITOR_COMMAND}}
```
- Serial port: {{MAC_PORT_PATTERN}} (Mac) / {{LINUX_PORT_PATTERN}} (Linux)
- Download/recovery mode: {{RECOVERY_PROCEDURE}}
- After switching machines, if the build misbehaves: clean build dir first
  ({{CLEAN_COMMAND}}); record IDF/SDK version mismatches in the journal.

## Hard rules for this project
- Never guess pin numbers or I2C addresses — `docs/HARDWARE.md` or measurement only.
- A bring-up item is done when verified on hardware, not when it compiles.
- {{PROJECT_SPECIFIC_RULES: e.g. "don't edit the GPL core", "sdkconfig handling"}}
- Durable hardware discoveries → `docs/HARDWARE.md`. Narrative → `docs/JOURNAL.md`.
  Keep THIS file short; offload anything not needed every session.
