#ifndef PMM_H
#define PMM_H

#include <limine.h>
#include <stdint.h>

int pmm_init(struct limine_memmap_response *memmap_resp, uint64_t hhdm_offset);
void *pmm_alloc(void);
void  pmm_free(void *addr);


#endif