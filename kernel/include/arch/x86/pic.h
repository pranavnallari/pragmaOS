#ifndef PIC_H
#define PIC_H

#include<stdint.h>

#define MASTER_COMMAND 0x20
#define MASTER_DATA 0x21
#define SLAVE_COMMAND 0xA0
#define SLAVE_DATA 0xA1

#define PIC_MASTER_OFFSET 32
#define PIC_SLAVE_OFFSET 40

void pic_init();
void pic_send_eoi(uint8_t irq);


#endif