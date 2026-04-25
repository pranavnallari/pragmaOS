// syscall.c

#include <arch/x86/syscall.h>
#include <drivers/terminal.h>

#define IA32_EFER   0xC0000080
#define IA32_LSTAR  0xC0000082
#define IA32_STAR   0xC0000081
#define IA32_FMASK  0xC0000084

extern void syscall_entry();

static void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = val & 0xFFFFFFFF;
    uint32_t high = (val >> 32);

    __asm__ volatile("wrmsr"::"c"(msr), "a"(low), "d"(high));
}

static uint64_t rdmsr(uint32_t msr) {
    uint32_t high, low;
    __asm__ volatile("rdmsr" :"=a"(low), "=d"(high) : "c"(msr));
    return (((uint64_t)(high) << 32) | low);
}

void syscall_init() {
    uint64_t EFER = rdmsr(IA32_EFER);
    EFER |= 1;
    wrmsr(IA32_EFER, EFER);

    uint64_t star = ((uint64_t)0x08 << 32) | ((uint64_t)0x18 << 48);
    wrmsr(IA32_STAR, star);

    uint64_t fmask = (1 << 9);
    wrmsr(IA32_FMASK, fmask);
    
    wrmsr(IA32_LSTAR, (uint64_t)syscall_entry);
}

uint64_t syscall_handler(void) {
    terminal_print(global_term, "Congratulations!! You have won : A syscall\n");
    return 0;
}