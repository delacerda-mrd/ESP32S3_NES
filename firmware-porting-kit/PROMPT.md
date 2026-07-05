# Bootstrap Prompt — Firmware Porting Project

You (Claude) have been asked to set up this repository as a firmware porting
project using the templates in `firmware-porting-kit/templates/`. Follow these
steps exactly.

## Step 1 — Gather facts

Inspect the repo first (build files, existing configs, source tree) and extract
whatever you can without asking. Then interview the user ONLY for what you could
not determine. Required facts:

- **Target hardware**: board name, MCU/SoC (exact variant), flash size/type, RAM/PSRAM
- **What's being ported**: the firmware/app, and where it came from (upstream repo, previous board)
- **Toolchain**: SDK/framework + version (e.g. ESP-IDF 5.x, Zephyr, STM32Cube, bare metal), host OS(es)
- **Flash/debug path**: how code gets onto the board, what the console/debug channel is
- **Peripheral inventory**: every peripheral the firmware must drive (display, input, audio, storage, radio, sensors, power management) with part numbers if known
- **Pin map**: as much as is known; mark unknowns explicitly as `TBD` — never guess pins
- **Machines**: which machines the user will work from (affects port names, sync workflow)

If the user has schematics, datasheets, or a vendor pin list, ask them to drop
the files in the repo or paste the relevant tables — capture pin data from
primary sources, not memory.

## Step 2 — Instantiate the docs

Copy each template out of `firmware-porting-kit/templates/` to its destination
(`CLAUDE.md` → repo root; `docs/*` → `docs/`), then replace every `{{...}}`
placeholder with real facts from Step 1. Leave `TBD` where genuinely unknown —
a visible TBD is correct; a plausible guess is a bug.

While filling in `docs/BRINGUP.md`: delete phases/items for peripherals this
board doesn't have, and ADD items for anything in the peripheral inventory the
template doesn't cover. The checklist must match THIS board exactly.

## Step 3 — Verify the environment

- Confirm the repo has a git remote (`git remote -v`). If not, stop and ask the
  user to create one — cross-machine sync depends on it.
- Confirm the toolchain is installed and record its exact version in CLAUDE.md.
- If a build can be attempted now, attempt it and record the result as the first
  journal entry. If not, record why not.

## Step 4 — Commit and push

Commit `CLAUDE.md` and `docs/` with message
`Bootstrap porting docs from firmware-porting-kit` and push to origin.
Ask the user whether to keep `firmware-porting-kit/` in this repo (useful for
re-reading the README) or delete it (it lives in the master project anyway).

## Step 5 — Confirm the workflow to the user

Briefly restate: docs auto-load each session, "wrap up" saves and pushes state,
same flow works on all their machines after `git pull`. Then ask what phase of
`docs/BRINGUP.md` they want to start with.

## Standing improvements (apply during setup, not just literally)

- Anything you learn during setup that contradicts a template assumption:
  fix the instantiated doc AND mention it so the user can improve the kit.
- Keep the instantiated CLAUDE.md under ~70 lines. If it's growing, offload to a
  docs/ file and leave a pointer.
