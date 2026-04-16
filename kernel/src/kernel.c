// kernel.c

#include<stdint.h>
#include<stddef.h>
#include<limine.h>
#include<stdbool.h>


extern uint8_t _binary_kernel_src_font8_psf_start;
extern uint8_t _binary_kernel_src_font8_psf_end;

extern uint8_t _binary_kernel_src_font16_psf_start;
extern uint8_t _binary_kernel_src_font16_psf_end;

extern uint8_t _binary_kernel_src_font32_psf_start;
extern uint8_t _binary_kernel_src_font32_psf_end;

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
   for(;;) __asm__ volatile ("hlt");
   
}


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

static inline void set_pixel(struct limine_framebuffer *fb, uint32_t *fb_ptr, size_t x, size_t y, uint32_t val) {
    fb_ptr[y * (fb->pitch / 4) + x] = val;
}



void terminal_init(S_TERMINAL_STATE *term_state, struct limine_framebuffer *fb, S_PSF2_HEADER *header, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    if (header->magic != 0x864ab572) hcf();
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
        return;
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

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t *fb_ptr = framebuffer->address;
    for (size_t i = 0; i < (framebuffer->pitch / 4) * framebuffer->height; i++)
        fb_ptr[i] = 0x00000000;
    
    uint8_t* font8_ptr = &_binary_kernel_src_font8_psf_start;
    uint8_t* font16_ptr = &_binary_kernel_src_font16_psf_start;
    uint8_t* font32_ptr = &_binary_kernel_src_font32_psf_start;

    S_PSF2_HEADER *header8 = (S_PSF2_HEADER *)font8_ptr;
        
    S_PSF2_HEADER *header16 = (S_PSF2_HEADER *)font16_ptr;
        if (header16->magic != 0x864ab572) hcf();
    S_PSF2_HEADER *header32 = (S_PSF2_HEADER *)font32_ptr;
        if (header32->magic != 0x864ab572) hcf();



    S_TERMINAL_STATE term;
    terminal_init(&term, framebuffer, header32, 0, 0, 0x00FF0000, 0x00000000);


    terminal_print(&term, "Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20\nLine 21\nLine 22\nLine 23\nLine 24\nLine 25\nLine 26\nLine 27\nLine 28\nLine 29\nLine 30");



    hcf();
}