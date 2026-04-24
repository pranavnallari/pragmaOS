// idt.c

#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/pic.h>
#include <arch/x86/io.h>
#include <drivers/terminal.h>

#define SET_ISR(n) idt_set_entry(n, isr##n, 0x8E)

S_IDT_ENTRY idt[256];

static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point",
    "Virtualization",
    "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved"
};

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
    if (vector == 32) {
        pic_send_eoi(0);
        return;
    }
    if (vector == 33) {
        uint8_t scancode = inb(0x60);
        keyboard_handle(scancode);
        pic_send_eoi(1);
        return;
    }
    terminal_print(global_term, "\nEXCEPTION: ");
    terminal_print(global_term, exception_messages[vector]);
    terminal_print(global_term, "\n");
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
    SET_ISR(32); SET_ISR(33);

    S_IDTR idtr = {
    .size = sizeof(idt) - 1,
    .offset = (uint64_t)idt
    };

    __asm__ volatile ("lidt %0": : "m"(idtr));
}