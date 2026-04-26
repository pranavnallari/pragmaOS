#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>
#include <limine.h>


int vmm_map(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags);
uint64_t *vmm_init(struct limine_memmap_response *memmap_resp, struct limine_executable_address_response *executable_address_response, uint64_t hhdm_offset_val);
uint64_t* vmm_create_user_space(uint64_t *pml4);
#endif