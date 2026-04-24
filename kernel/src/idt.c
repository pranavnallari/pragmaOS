// idt.c

#include<idt.h>
#include<isr.h>

#define SET_ISR(n) idt_set_entry(n, isr##n, 0x8E)

S_IDT_ENTRY idt[256];


void idt_set_entry(uint8_t vector, void *handler, uint8_t type_attr) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].ist = 0;
    idt[vector].selector = 0x08;
    idt[vector].type_attr = type_attr;
    idt[vector].zero = 0;
    idt[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].offset_mid = (addr >> 16) & 0xFFFF;
    idt[vector].offset_low = addr & 0xFFFF;
}

void isr_handler(uint64_t vector) {
    (void)vector;
    for(;;) __asm__ volatile("hlt");
}

void idt_init(void) {
    SET_ISR(0);  SET_ISR(1);  SET_ISR(2);  SET_ISR(3);
    SET_ISR(4);  SET_ISR(5);  SET_ISR(6);  SET_ISR(7);
    SET_ISR(8);  SET_ISR(9);  SET_ISR(10); SET_ISR(11);
    SET_ISR(12); SET_ISR(13); SET_ISR(14); SET_ISR(15);
    SET_ISR(16); SET_ISR(17); SET_ISR(18); SET_ISR(19);
    SET_ISR(20); SET_ISR(21); SET_ISR(22); SET_ISR(23);
    SET_ISR(24); SET_ISR(25); SET_ISR(26); SET_ISR(27);
    SET_ISR(28); SET_ISR(29); SET_ISR(30); SET_ISR(31);

    S_IDTR idtr = {
    .size = sizeof(idt) - 1,
    .offset = (uint64_t)idt
    };

    __asm__ volatile ("lidt %0": : "m"(idtr));
}