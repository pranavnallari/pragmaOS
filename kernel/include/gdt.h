#ifndef GDT_H
#define GDT_H
#include <stdint.h>

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) S_GDTR;

void gdt_init(void);

#endif