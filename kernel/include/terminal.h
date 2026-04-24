// terminal.h
#ifndef TERMINAL_H
#define TERMINAL_H

#include<stdint.h>
#include<stddef.h>
#include<limine.h>
#include<stdbool.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyphs;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} S_PSF2_HEADER;


typedef struct {
    struct limine_framebuffer *framebuffer;
    uint32_t *fb_ptr;
    uint32_t height;
    uint32_t width;
    uint32_t bytesperglyph;
    uint32_t glyph_height;
    uint32_t glyph_width;
    uint32_t fg_colour;
    uint32_t bg_colour;
    size_t curr_x;
    size_t curr_y;
    size_t start_x;
    uint8_t *glyphs;
} S_TERMINAL_STATE;

extern S_TERMINAL_STATE *global_term;


int terminal_init(S_TERMINAL_STATE *term_state, struct limine_framebuffer *fb, S_PSF2_HEADER *header, size_t x, size_t y, uint32_t fg, uint32_t bg);
void terminal_scroll(S_TERMINAL_STATE *term);
void terminal_putchar(S_TERMINAL_STATE *term, uint8_t ch);
void terminal_print(S_TERMINAL_STATE *term, const char *str);
void terminal_print_hex(S_TERMINAL_STATE *term, uint64_t val);


#endif