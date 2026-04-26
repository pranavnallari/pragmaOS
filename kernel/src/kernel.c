// kernel.c

#include<stdint.h>
#include<stddef.h>
#include<stdbool.h>

#include <limine.h>

#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <arch/x86/pit.h>
#include <arch/x86/io.h>
#include <arch/x86/tss.h>
#include <arch/x86/syscall.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drivers/terminal.h>
#include<lib/mem.h>

extern uint8_t _binary_kernel_src_fonts_font8_psf_start;
extern uint8_t _binary_kernel_src_fonts_font16_psf_start;
extern uint8_t _binary_kernel_src_fonts_font32_psf_start;

extern uint8_t _binary_kernel_src_fonts_font8_psf_end;
extern uint8_t _binary_kernel_src_fonts_font16_psf_end;
extern uint8_t _binary_kernel_src_fonts_font32_psf_end;


__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
   for(;;) __asm__ volatile ("hlt");
   
}

static uint8_t user_program[] = {
    0x48, 0x31, 0xC0,   // xor rax, rax
    0x0F, 0x05,         // syscall
    0xEB, 0xFE
};
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
    
    uint8_t* font8_ptr = &_binary_kernel_src_fonts_font8_psf_start;
    uint8_t* font16_ptr = &_binary_kernel_src_fonts_font16_psf_start;
    uint8_t* font32_ptr = &_binary_kernel_src_fonts_font32_psf_start;

    S_PSF2_HEADER *header8 = (S_PSF2_HEADER *)font8_ptr;  
    S_PSF2_HEADER *header16 = (S_PSF2_HEADER *)font16_ptr;
    S_PSF2_HEADER *header32 = (S_PSF2_HEADER *)font32_ptr;



    S_TERMINAL_STATE term;
    if (terminal_init(&term, framebuffer, header16, 0, 0, 0x00FF0000, 0x00000000) < 0) hcf();
    global_term = &term;


    struct limine_memmap_response *memmap_resp = memmap_request.response;
    if (memmap_resp == NULL) hcf();

    if (hhdm_request.response == NULL) hcf();
    uint64_t hhdm_offset = hhdm_request.response->offset;

    terminal_print(&term, "Before memory shenanigans\n");

    if (pmm_init(memmap_resp, hhdm_offset) < 0) {
        hcf();
    }
    terminal_print(&term, "PMM Init successfull\n");

    if (executable_address_request.response == NULL) {
        hcf();
    }
    terminal_print(&term, "executable address request recieved successfully\n");


    uint64_t *pml4_table = vmm_init(memmap_resp, executable_address_request.response, hhdm_offset);
    if (!pml4_table) {
        hcf();
    }
    
    gdt_init();
    tss_init();
    syscall_init();
    idt_init();
    pic_init();
    pit_init();
    while (inb(0x64) & 1) {
        inb(0x60);
    }
    __asm__ volatile("sti");
    terminal_print(&term, "Great Success!!. PML4 addr : ");
    terminal_print_hex(&term, (uint64_t)pml4_table);
    terminal_print(&term, "\n");

    uint64_t *user_pml4 = vmm_create_user_space(pml4_table);
    uint64_t* user_page = pmm_alloc();
    if (!user_page) hcf();
    uint64_t *virt_addr = (uint64_t *)((uint64_t)user_page + hhdm_offset);
    memcpy((void*)(virt_addr), user_program, sizeof(user_program));
    if (vmm_map(user_pml4, (uint64_t)user_page, 0x400000, 0x7) < 0) hcf();
    uint64_t* stack_page = pmm_alloc();
    if (!stack_page) hcf();

    if (vmm_map(user_pml4, (uint64_t)stack_page, 0x500000, 0x7) < 0) hcf();
    uint64_t user_rsp = 0x500000 + 0x1000;
    uint64_t user_pml4_phys = (uint64_t)user_pml4 - hhdm_offset;

    __asm__ volatile ("mov %0, %%cr3" :: "r"(user_pml4_phys) : "memory");

    __asm__ volatile (
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "pushq $0x23\n"          // SS
        "pushq %[rsp]\n"         // RSP
        "pushfq\n"
        "pushq $0x1B\n"          // CS
        "pushq $0x400000\n"      // RIP

        "iretq\n"
        :
        : [rsp]"r"(user_rsp)
        : "memory", "ax"
    );
    hcf();
}