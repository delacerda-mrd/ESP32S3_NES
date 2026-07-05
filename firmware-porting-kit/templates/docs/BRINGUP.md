# Bring-up Checklist — {{PROJECT_NAME}}

The "nothing missed" document. Work phases in order — each phase assumes the ones
before it are solid. A box gets checked ONLY when verified on hardware (with the
evidence noted), never merely when code compiles. Tailor this list to the board
during bootstrap: delete inapplicable items, add every peripheral in the inventory.

Status legend: `[ ]` not started · `[~]` in progress (note blocker) · `[x]` verified on HW

## Phase 0 — Groundwork
- [ ] Schematics / board pin list obtained and stored in repo (or link recorded)
- [ ] Datasheets for every peripheral IC collected
- [ ] Upstream firmware builds unmodified for its ORIGINAL target (baseline sanity)
- [ ] SDK/toolchain installed on every dev machine; exact versions recorded in CLAUDE.md

## Phase 1 — Toolchain, boot, and console
- [ ] Project builds clean for the new target
- [ ] Flash procedure works; recovery/download mode procedure confirmed
- [ ] Console output visible (correct port on each machine recorded)
- [ ] Correct flash mode/size/speed configured and detected at boot
- [ ] External RAM (if any) detected at full size and speed
- [ ] CPU clock as intended; brownout detector behavior sane
- [ ] Boot reaches application `main` with no watchdog resets at idle

## Phase 2 — Pin audit (before touching peripherals)
- [ ] Every pin in HARDWARE.md map cross-checked against schematic/datasheet
- [ ] Strapping/boot pins identified; no conflict with peripheral use
- [ ] Pins shared between peripherals identified; arbitration plan noted
- [ ] All active-low / open-drain / pull requirements captured in the pin table

## Phase 3 — Buses
- [ ] Each I2C bus scans and finds every expected device at its expected address
- [ ] Each SPI bus: clock/mode verified against slowest device datasheet limit
- [ ] {{other buses: I2S, SDIO, UART, CAN, USB — as applicable}}

## Phase 4 — Peripherals, one at a time (isolate: minimal test before integration)
<!-- One block per peripheral from the inventory. Examples: -->
### Display
- [ ] Controller responds (ID readback or equivalent)
- [ ] Init sequence produces stable blank screen (no noise/tearing)
- [ ] Test pattern correct: orientation, colors (RGB vs BGR), full extents
- [ ] Sustained full-frame updates at target rate without artifacts
### Input ({{touch/buttons/controller}})
- [ ] Raw events received; coordinates/values sane and calibrated
- [ ] Mapped to firmware input; latency acceptable
### Audio
- [ ] Codec/DAC responds; amp enable polarity confirmed
- [ ] Test tone clean at speaker/jack (no clicks, correct rate — measure, don't trust config)
### Storage / {{SD, ROM partition, external flash}}
- [ ] Read verified against known data; write+readback if applicable
### {{Radio / sensors / power management — add per inventory}}

## Phase 5 — Firmware integration
- [ ] Memory budget: heap/stack headroom measured under load, not estimated
- [ ] RTOS task priorities/affinities reviewed for the new SoC's core count
- [ ] Performance target met ({{frame rate / throughput / latency metric}})
- [ ] All peripherals working SIMULTANEOUSLY (bus contention, IRQ load, DMA conflicts)

## Phase 6 — Validation
- [ ] 30+ minute soak: no resets, no watchdogs, no heap creep
- [ ] Power-cycle test: cold boots reliably ×10
- [ ] Recovery paths still work (can always reflash a bricked-looking board)
- [ ] Thermal check under sustained load
- [ ] JOURNAL.md Current Status updated to "port complete"; gotchas migrated to HARDWARE.md
