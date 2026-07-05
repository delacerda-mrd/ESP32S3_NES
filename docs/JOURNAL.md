# Troubleshooting Journal

> **How this file works:** The *Current Status* block at the top is always the
> latest truth — Claude reads it first every session. Below it, dated entries are
> appended (newest first) recording what was tried and what happened. Never delete
> old entries; they prevent re-trying failed approaches.

---

## Current Status (updated 2026-07-05)

**Phase:** Initial setup — persistence/docs system just created. No troubleshooting sessions logged yet.

**Known working:**
- Project builds as ESP-IDF v5.x CMake project (per README; verify with `idf.py build`)

**Known broken / unverified:**
- Nothing logged yet — first real debugging session should update this.

**Next steps:**
- [ ] Verify clean build on this machine (`idf.py set-target esp32s3 && idf.py build`)
- [ ] Confirm which ESP-IDF version is installed and works with this tree
- [ ] Flash and record baseline behavior (LCD init? touch? audio? emulator boot?)

**Environment notes:**
- macOS (Darwin 25.5), port will be `/dev/cu.usbmodem*` not `/dev/ttyACM0`
- ESP-IDF version in use: _not yet recorded_

---

## Session Log (newest first)

### 2026-07-05 — Project bootstrap
- Created persistence system: `CLAUDE.md`, `docs/JOURNAL.md`, `docs/HARDWARE.md`,
  `CLAUDE_WORKFLOW_README.md`.
- No code changes. No hardware testing yet.
- Repo state: clean master at `6fe4827` (CMake migration commit).

<!-- TEMPLATE for new entries — copy this:
### YYYY-MM-DD — <one-line summary>
**Goal:**
**Tried:**
**Result / evidence:** (include exact error messages, monitor output snippets)
**Conclusion:**
**Next steps:**
-->
