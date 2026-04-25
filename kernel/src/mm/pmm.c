// pmm.c

#include <mm/pmm.h>
#include<stddef.h>


static uint8_t *bitmap;
static uint64_t max_pages;
static uint64_t page_size = 4096;



int pmm_init(struct limine_memmap_response *memmap_resp, uint64_t hhdm_offset) {
    uint64_t max_address = 0;
    for (uint64_t i = 0; i < memmap_resp->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_resp->entries[i];
        if (max_address < (entry->base + entry->length))
            max_address = entry->base + entry->length;
    }
    page_size = 4096;
    max_pages = max_address / page_size;
    uint64_t bitmap_size = (max_pages + 7) / 8;

    bitmap = NULL;
    for (uint64_t i = 0; i < memmap_resp->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            uint64_t aligned = (entry->base + page_size - 1) & ~(page_size - 1);
            bitmap = (uint8_t *)(aligned + hhdm_offset);
            break;
        }
    }
    if (bitmap == NULL) return -1;

    for (uint64_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0xFF;

    for (uint64_t i = 0; i < memmap_resp->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t curr = entry->base; curr < entry->base + entry->length; curr += page_size) {
                uint64_t page = curr / page_size;
                if (page >= max_pages) continue;
                bitmap[page / 8] &= ~(1U << (page % 8));
            }
        }
    }
    
    uint64_t bitmap_phys = (uint64_t)bitmap - hhdm_offset;
    uint64_t bitmap_pages = (bitmap_size + page_size - 1) / page_size;
    uint64_t bitmap_start_page = bitmap_phys / page_size;
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        uint64_t page = bitmap_start_page + i;
        bitmap[page / 8] |= (1U << (page % 8));
    }

    return 0;
}

void *pmm_alloc(void) {
    uint64_t *bm64 = (uint64_t *)bitmap;
    uint64_t total_u64 = (max_pages + 63) / 64;
    for (uint64_t i = 0; i < total_u64; i++) {
        if (bm64[i] == 0xFFFFFFFFFFFFFFFF) continue;
        for (uint64_t bit = 0; bit < 64; bit++) {
            if (!(bm64[i] & (1ULL << bit))) {
                bm64[i] |= (1ULL << bit);
                uint64_t page = i * 64 + bit;
                return (void *)(page * page_size);
            }
        }
    }
    return NULL;
}

void pmm_free(void *addr) {
    uint64_t page_index = (uint64_t)addr / page_size;
    if (page_index >= max_pages) return;
    bitmap[page_index / 8] &= ~(1U << (page_index % 8));
}