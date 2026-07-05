# Firmware Porting Kit

A reusable system for porting firmware to new hardware with Claude Code, designed
so you can work in short sessions across multiple machines (Mac + Linux) and never
re-explain context.

## What it gives you

1. **A phased bring-up checklist** (`templates/docs/BRINGUP.md`) so nothing is
   missed — from toolchain to soak testing, each peripheral verified in order.
2. **Progressive-disclosure docs**: a lean `CLAUDE.md` that auto-loads every
   session (the source of truth for *where things are*), with detail offloaded to
   `docs/` files that are only read when a task needs them.
3. **Cross-machine sync via git**: pull at session start, push the journal at
   session end.

## Setup for a new porting project (3 steps)

1. Copy this whole `firmware-porting-kit/` folder into the root of the new
   project repo (which should already be a git repo with a remote):

   ```bash
   cp -r /path/to/firmware-porting-kit /path/to/new-project/
   ```

2. Start `claude` in the new project root and paste exactly this:

   > Read firmware-porting-kit/PROMPT.md and follow it.

3. Answer Claude's interview questions about the board (have schematics,
   datasheets, or a pin list handy — or point Claude at files/URLs containing
   them). Claude instantiates the docs, fills in what it can, and commits.

That's it. From then on, every session: state your goal; say "wrap up" when done.

## Board library (`boards/`)

Complete, verified hardware references for boards you own — full pin maps, I2C
addresses, init sequences, and known quirks. During bootstrap, Claude checks this
folder first: if the target board is here, it copies the reference into the
project (`docs/BOARD_REFERENCE.md`) and skips the hardware interview almost
entirely.

Currently in the library:
- **ES3C28P** — QDtech 2.8" ESP32-S3R8 IPS display module (ILI9341V, FT6336G
  capacitive touch, ES8311 audio, SDIO microSD, battery mgmt)
- **ESP32-2432S028 "CYD"** — Sunton Cheap Yellow Display, 2.8" ESP32 (ILI9341,
  XPT2046 resistive touch, DAC audio, SPI microSD)

**Add every new board you acquire**: drop a `<BOARD>_BOARD_REFERENCE.md` here
following the same structure (SoC → display → touch → audio → storage → complete
GPIO allocation table → flashing procedure). The complete GPIO table is the most
valuable section — it's what prevents pin-conflict debugging.

## The document architecture (why it's shaped this way)

| File | Loaded | Contains |
|------|--------|----------|
| `CLAUDE.md` | **Every session, automatically** | Project identity, build/flash commands, session protocols, pointers to the docs below. Kept SHORT deliberately. |
| `docs/JOURNAL.md` | Every session (protocol says read it) | Current Status block + dated session log. The resume mechanism. |
| `docs/BRINGUP.md` | When planning what to do next | Phased porting checklist with per-item status. The "nothing missed" mechanism. |
| `docs/HARDWARE.md` | Only when task touches hardware | Pinout, buses, addresses, memory map, discovered quirks. |

**Rule of thumb:** CLAUDE.md answers "what is this and how do I build it";
everything that answers "what happened" or "what are the details" is offloaded.
This keeps every session's baseline context small while guaranteeing the details
exist and are found when needed.

## Rules that keep it working

1. `docs/JOURNAL.md`'s Current Status block is the single source of "where are we".
2. Never delete journal entries — they prevent re-trying failed approaches.
3. Discoveries migrate up: journal entry → HARDWARE.md (once confirmed) →
   CLAUDE.md (only if needed every session).
4. Every wrap-up commits and pushes the docs. Session start always pulls.
5. Check off BRINGUP.md items only when verified **on hardware**, not when code compiles.

## Maintaining the kit itself

This folder is the master copy. When a porting project teaches you something that
would improve the *next* port (a checklist item you wish existed, a better
template section), update the templates here and commit — the kit gets better
with every board.
