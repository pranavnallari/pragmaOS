// terminal.c

#include <terminal.h>


S_TERMINAL_STATE *global_term = NULL;

static inline void set_pixel(struct limine_framebuffer *fb, uint32_t *fb_ptr, size_t x, size_t y, uint32_t val) {
    fb_ptr[y * (fb->pitch / 4) + x] = val;
}



int terminal_init(S_TERMINAL_STATE *term_state, struct limine_framebuffer *fb, S_PSF2_HEADER *header, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    if (header->magic != 0x864ab572) return -1;
    term_state->framebuffer = fb;
    term_state->fb_ptr = fb->address;
    term_state->height = fb->height;
    term_state->width = fb->width;
    term_state->bytesperglyph = header->bytesperglyph;
    term_state->glyph_height = header->height;
    term_state->glyph_width = header->width;
    term_state->fg_colour = fg;
    term_state->bg_colour = bg;
    term_state->curr_x = x;
    term_state->curr_y = y;
    term_state->start_x = x;
    term_state->glyphs = (uint8_t*)header + header->headersize;
    return 0;
}


void terminal_scroll(S_TERMINAL_STATE *term) {
    uint64_t pitch_pixels = term->framebuffer->pitch / 4;
    
    for (size_t y = 0; y < term->height - term->glyph_height; y++) {
        for (size_t x = 0; x < term->width; x++) {
            term->fb_ptr[y * pitch_pixels + x] = term->fb_ptr[(y + term->glyph_height) * pitch_pixels + x];
        }
    }

    for (size_t y = term->height - term->glyph_height; y < term->height; y++) {
        for (size_t x = 0; x < term->width; x++) {
            term->fb_ptr[y * pitch_pixels + x] = term->bg_colour;
        }
    }

    term->curr_y = term->height - term->glyph_height;
}

void terminal_putchar(S_TERMINAL_STATE *term, uint8_t ch) {
    if (ch == 0x0A) {
        term->curr_x = term->start_x;
        term->curr_y += term->glyph_height;
        if (term->curr_y + term->glyph_height > term->height) {
            terminal_scroll(term);
            term->curr_x = term->start_x;
        }
        return;
    }
    if (term->curr_x + term->glyph_width >= term->width) {
        term->curr_x = term->start_x;
        term->curr_y += term->glyph_height;
    }
    if (term->curr_y + term->glyph_height > term->height) {
        terminal_scroll(term);
        term->curr_x = term->start_x;
    }
    uint8_t *glyph = term->glyphs + (ch * term->bytesperglyph);
    for (size_t row = 0; row < term->glyph_height; row++) {
    for (size_t col = 0; col < term->glyph_width; col++) {
        size_t byte_index = row * ((term->glyph_width + 7) / 8) + col / 8;
        uint8_t row_data = glyph[byte_index];
        if ((row_data >> (7 - (col % 8))) & 1)
            set_pixel(term->framebuffer, term->fb_ptr, term->curr_x + col, term->curr_y + row, term->fg_colour);
        else
            set_pixel(term->framebuffer, term->fb_ptr, term->curr_x + col, term->curr_y + row, term->bg_colour);
    }
}

    term->curr_x += term->glyph_width;
}

void terminal_print(S_TERMINAL_STATE *term, const char *str) {
    while (*str) {
        terminal_putchar(term, (uint8_t)*str);
        str++;
    }
}


void terminal_print_hex(S_TERMINAL_STATE *term, uint64_t val) {
    char hex[] = "0123456789ABCDEF";

    terminal_print(term, "0x");

    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0xF;
        char c = hex[nibble];

        char str[2] = {c, 0};
        terminal_print(term, str);
    }
}