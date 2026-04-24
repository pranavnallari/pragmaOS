// gdt.c

#include <arch/x86/gdt.h>

static uint64_t gdt[5];

void gdt_init(void) {
    gdt[0] = 0x0000000000000000;
    gdt[1] = 0x00AF9A000000FFFF;
    gdt[2] = 0x00AF92000000FFFF;
    gdt[3] = 0x00AFFA000000FFFF;
    gdt[4] = 0x00AFF2000000FFFF;

    S_GDTR gdtr = {
        .limit = sizeof(gdt) - 1,
        .base = (uint64_t)gdt
    };

    __asm__ volatile ("lgdt %0" : : "m"(gdtr));

    // Reload CS via Far Jump
    __asm__ volatile (
    "pushq $0x08\n"
    "lea 1f(%%rip), %%rax\n"
    "pushq %%rax\n"
    "lretq\n"
    "1:\n"
    ::: "rax", "memory"
    );

    // reload data segment registers
    __asm__ volatile (
    "mov $0x10, %%ax\n"
    "mov %%ax, %%ds\n"
    "mov %%ax, %%es\n"
    "mov %%ax, %%ss\n"
    ::: "ax", "memory"
    );
}