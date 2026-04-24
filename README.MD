# Pragma OS

My attempt at building a 64-bit hobby kernel written in C, booted via Limine (UEFI).

Built from scratch — no tutorials, no copy-paste.

---

## Architecture

| Property | Value |
|---|---|
| Target arch | x86-64 |
| Boot protocol | Limine (UEFI) |
| Language | C + x86-64 ASM |
| Build system | GNU Make |
| Emulator | QEMU  |

---

## Project Structure

```
.
├── Makefile
├── linker.ld
├── limine.conf
├── limine/
└── kernel/
    ├── include/
    │   ├── arch/x86/      # gdt.h, idt.h, isr.h, pic.h, pit.h, io.h
    │   ├── drivers/       # terminal.h, keyboard.h
    │   └── mm/            # pmm.h, vmm.h
    └── src/
        ├── arch/x86/      # gdt.c, idt.c, isr.asm, pic.c, pit.c
        ├── drivers/       # terminal.c, keyboard.c
        ├── mm/            # pmm.c, vmm.c
        ├── fonts/         # font8.psf, font16.psf, font32.psf
        └── kernel.c
```

---

## Building

**Requirements:** `x86_64-elf-gcc`, `nasm`, `xorriso`, `qemu-system-x86_64`, OVMF firmware

```bash
# Build and run in QEMU
make run

# Build ISO only
make
```

---

## Author

**Pranav Nallari** — [github.com/pranavnallari](https://github.com/pranavnallari)