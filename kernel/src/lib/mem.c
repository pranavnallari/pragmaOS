// mem.c

#include <lib/mem.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* d = dest;
    const uint8_t* s = src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return d;
}

void* memset(void* dest, int c, size_t size) {
    uint8_t *d = dest;
    for (size_t i = 0; i < size; i++) {
        d[i] = c;
    }
    return d;
}