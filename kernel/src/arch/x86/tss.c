// tss.c

#include <arch/x86/tss.h>
#include <arch/x86/gdt.h>

static S_TSS tss = {0};
__attribute__((aligned(16)))
static uint8_t temp_stack[4096];


void tss_set_rsp0(uint64_t rsp) {
    tss.rsp0 = rsp;
}


void tss_init(void) {
    tss.iopb_offset = sizeof(S_TSS);
    uint64_t base = (uint64_t)&tss;
    uint32_t limit = sizeof(tss) - 1;
    gdt[5] = 0;
    gdt[5] |= (uint64_t)(limit & 0xFFFF);
    gdt[5] |= (uint64_t)(base & 0xFFFFFF) << 16;
    gdt[5] |= (uint64_t)((uint64_t)(0x89) << 40);
    gdt[5] |= (uint64_t)((limit >> 16) & 0xF) << 48;
    gdt[5] |= (uint64_t)((base >> 24) & 0xFF) << 56;

    gdt[6] = 0;
    gdt[6] = (uint64_t)((base >> 32) & 0xFFFFFFFF);

    __asm__ volatile ("ltr %0" : : "r"((uint16_t)0x28));

    tss_set_rsp0((uint64_t)(temp_stack + 4096));
}


