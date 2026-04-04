#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void pmm_init(uint64_t mem_start, uint64_t mem_size);
void* pmm_alloc(void);
void  pmm_free(void* addr);
uint64_t pmm_free_pages(void);

#endif