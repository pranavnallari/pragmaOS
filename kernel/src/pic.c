// pic.c

#include<pic.h>
#include<io.h>


void pic_init() {
    outb(MASTER_COMMAND, 0x11);
    io_wait();
    outb(SLAVE_COMMAND, 0x11);
    io_wait();

    outb(MASTER_DATA, PIC_MASTER_OFFSET);
    io_wait();
    outb(SLAVE_DATA, PIC_SLAVE_OFFSET);
    io_wait();

    outb(MASTER_DATA, 0x04);
    io_wait();

    outb(SLAVE_DATA, 0x02);
    io_wait();

    outb(MASTER_DATA, 0x01);
    io_wait();
    outb(SLAVE_DATA, 0x01);
    io_wait();

    outb(MASTER_DATA, 0xFC);
    io_wait();
    outb(SLAVE_DATA, 0xFF);
    io_wait();
}


void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(SLAVE_COMMAND, 0x20);
    }
    outb(MASTER_COMMAND, 0x20);
}