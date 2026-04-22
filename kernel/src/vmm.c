// vmm.c

#include "vmm.h"
#include "pmm.h"

static uint64_t hhdm_offset;

extern uint64_t _kernel_start;
extern uint64_t _kernel_end;

static uint64_t *vmm_get_or_create_next_table(uint64_t *table, uint16_t index) {
    uint64_t entry = table[index];
    uint64_t *next_table;

    if (entry & 1) {
        uint64_t phys = entry & 0x000FFFFFFFFFF000;
        next_table = (uint64_t *)(phys + hhdm_offset);
    } else {
        uint64_t phys = (uint64_t)pmm_alloc();
        if (!phys) return NULL;

        next_table = (uint64_t *)(phys + hhdm_offset);

        for (uint64_t i = 0; i < 512; i++) {
            next_table[i] = 0;
        }

        table[index] = phys | 0x3;
    }

    return next_table;
}

int vmm_map(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags) {

    if ((phys & 0xFFF) || (virt & 0xFFF)) {
        return -1;
    }


    uint16_t pml4_index = (virt >> 39) & 0x1FF;
    uint16_t pdpt_index = (virt >> 30) & 0x1FF;
    uint16_t pd_index   = (virt >> 21) & 0x1FF;
    uint16_t pt_index   = (virt >> 12) & 0x1FF;

    uint64_t *pdpt_table = vmm_get_or_create_next_table(pml4, pml4_index);
    if (!pdpt_table) return -1;

    uint64_t *pd_table = vmm_get_or_create_next_table(pdpt_table, pdpt_index);
    if (!pd_table) return -1;

    uint64_t *pt_table = vmm_get_or_create_next_table(pd_table, pd_index);
    if (!pt_table) return -1;

    if (pt_table[pt_index] & 1) {
        return -1;
    }

    pt_table[pt_index] = phys | flags;

    return 0;
}


uint64_t *vmm_init(struct limine_memmap_response *memmap_resp, struct limine_executable_address_response *executable_address_response, uint64_t hhdm_offset_val) {
    hhdm_offset = hhdm_offset_val;
    uint64_t *pml4_phys = pmm_alloc();
    uint64_t *pml4_virt = (uint64_t *)((uint64_t)pml4_phys + hhdm_offset);
    uint64_t flags = 0x3;
    for (uint64_t i = 0; i < 512; i++) {
            pml4_virt[i] = 0;
    }

    for (uint64_t i = 0; i < memmap_resp->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            for (uint64_t page = entry->base; page < entry->base + entry->length; page += 0x1000) {
                if (vmm_map(pml4_virt, page, page + hhdm_offset, flags) < 0) return NULL;
            }
        }
    }

    uint64_t kernel_size = (uint64_t)&_kernel_end - (uint64_t)&_kernel_start;
    uint64_t kernel_pages = (kernel_size + 0xFFF) / 0x1000;

    for (uint64_t i = 0; i < kernel_pages; i++) {
        vmm_map(pml4_virt, executable_address_response->physical_base + i * 0x1000, executable_address_response->virtual_base + i * 0x1000, flags);
    }


    __asm__ volatile ("mov %0, %%cr3" :: "r"((uint64_t)pml4_phys) : "memory");

    return pml4_virt;
}