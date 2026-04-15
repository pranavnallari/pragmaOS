// kernel.c

#include<stdint.h>
#include<stddef.h>
#include<limine.h>
#include<stdbool.h>

#define set_pixel(fb, fb_ptr, x, y, val) fb_ptr[y * (fb->pitch/4) + x] = val


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

void putchar(struct limine_framebuffer *framebuffer, uint32_t *fb_ptr, uint8_t *glyphs, char ch, size_t *cursor_x, size_t *cursor_y) {
        
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
    volatile uint32_t *fb_ptr = framebuffer->address;

    uint8_t* font_ptr = &_binary_kernel_src_font_psf_start;

    S_PSF2_HEADER *header = (S_PSF2_HEADER *)font_ptr;

    uint8_t *glyphs = font_ptr + header->headersize;
    uint8_t *glyph = glyphs + (65 * header->bytesperglyph);
    for (size_t row = 0; row < 16; row++) {
        uint8_t row_data = glyph[row];
        for (size_t col = 0; col < 8; col++){
            if ((row_data >> (7 - col)) & 1) set_pixel(framebuffer, fb_ptr, 500+col, 500+row, 0x00FF0000);
        }
    }

    hcf();
}