#ifndef IDT_H
#define IDT_H

#include<stdint.h>

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) S_IDT_ENTRY;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) S_IDTR;

void idt_init(void);

void idt_set_entry(uint8_t vector, void *handler, uint8_t type_attr);

#endif