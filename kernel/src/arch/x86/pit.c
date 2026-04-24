// pit.c

#include <arch/x86/pit.h>
#include <arch/x86/io.h>
#include<stdint.h>


void pit_init() {
    uint16_t divisor = 1193182 / PIT_FREQ;
    outb(0x43, 0x36);
    io_wait();
    outb(0x40, divisor & 0xFF);
    io_wait();
    outb(0x40, (divisor >> 8) & 0xFF);
    io_wait();
}