// kernel.c

#include<stdint.h>
#include<stddef.h>
#include<limine.h>
#include<stdbool.h>


extern uint8_t _binary_kernel_src_font_psf_start;
extern uint8_t _binary_kernel_src_font_psf_end;

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


typedef struct psf2_header {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyphs;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} S_PSF2_HEADER;


static inline void set_pixel(struct limine_framebuffer *fb, uint32_t *fb_ptr, size_t x, size_t y, uint32_t val) {
    fb_ptr[y * (fb->pitch / 4) + x] = val;
}

void putchar(struct limine_framebuffer *framebuffer, uint32_t *fb_ptr, S_PSF2_HEADER *header, uint8_t *glyphs, uint8_t ch, size_t *cursor_x, size_t *cursor_y, size_t start_x, uint32_t colour) {
    if (ch == 0x0A) {
        *cursor_x = start_x;
        *cursor_y += header->height;
        return;
    }
    uint8_t *glyph = glyphs + (ch * header->bytesperglyph);
    for (size_t row = 0; row < header->height; row++) {
        uint8_t row_data = glyph[row];
        for (size_t col = 0; col < header->width; col++) {
            if ((row_data >> (header->width - 1 - col)) & 1) set_pixel(framebuffer, fb_ptr, *cursor_x + col, *cursor_y + row, colour);
        }
    }

    *cursor_x += header->width;
}

void kprint(struct limine_framebuffer *fb, uint32_t *fb_ptr, S_PSF2_HEADER *header, uint8_t *glyphs, const char *str, size_t *cursor_x, size_t *cursor_y, size_t start_x, uint32_t colour) {
    while (*str) {
        putchar(fb, fb_ptr, header, glyphs, *str, cursor_x, cursor_y, start_x, colour);
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
    
    uint8_t* font_ptr = &_binary_kernel_src_font_psf_start;

    S_PSF2_HEADER *header = (S_PSF2_HEADER *)font_ptr;


    uint8_t *glyphs = font_ptr + header->headersize;
    size_t cursor_x = 100, cursor_y = 100, start_x = 100;

    kprint(framebuffer, fb_ptr, header, glyphs, "Hello\nWorld", &cursor_x, &cursor_y, start_x, 0x00FF0000);



    hcf();
}