# Hardware Reference — {{BOARD_NAME}}

Durable, verified hardware facts only. When debugging reveals a correction,
fix it HERE immediately so it's never re-discovered. `TBD` means unknown —
never replace a TBD with a guess; replace it with a measurement or a datasheet fact
(and cite the source).

## Board / SoC
- **{{BOARD_NAME}}**: {{MCU_VARIANT}}, {{FLASH_SIZE_TYPE_SPEED}}, {{RAM_PSRAM_DESC}}
- Console/debug channel: {{CONSOLE_DESC}}
- Download/recovery mode: {{RECOVERY_PROCEDURE}}
- Power: {{SUPPLY_NOTES: input voltage, regulators, current budget if known}}

## Pin map (complete — every used pin, mark unknowns TBD)
| Function | GPIO/Pin | Notes (polarity, pull, strapping conflicts) |
|----------|----------|---------------------------------------------|
| {{...}}  | {{...}}  | {{...}} |

**Strapping/boot pins on this SoC:** {{list them and whether this design touches them}}

## Buses
### {{BUS: e.g. I2C0}}
- Pins: SDA={{}}, SCL={{}}, speed={{}}
- Devices: {{part @ 7-bit addr, part @ addr}}
- Verified by bus scan on hardware: {{yes/no + date}}

### {{BUS: e.g. SPI2 — display}}
- Pins + clock speed: {{}}
- Devices/CS lines: {{}}

## Peripherals (one section per part)
### {{PART_NUMBER}} — {{role, e.g. LCD controller}}
- Interface: {{bus, pins, addr/CS}}
- Init quirks: {{rotation, gamma, polarity, timing — as discovered}}
- Driver location in tree: {{path}}
- Datasheet: {{link/filename if stored in repo}}

## Memory / flash map
- Partition/linker layout: {{table or pointer to partitions.csv/linker script}}
- {{Where assets/blobs live, mmap details}}

## Gotchas discovered (append as found, date each)
- _none yet_
