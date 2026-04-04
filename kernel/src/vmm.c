#include "vmm.h"
#include "pmm.h"

// Page table entry helpers
#define PML4_INDEX(addr)  (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr)  (((addr) >> 30) & 0x1FF)
#define PDT_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)    (((addr) >> 12) & 0x1FF)

static uint64_t* pml4 = 0;

static uint64_t* get_or_create(uint64_t* table, uint64_t index, uint64_t flags) {
    if (!(table[index] & PAGE_PRESENT)) {
        uint64_t* new_table = (uint64_t*)pmm_alloc();

        // Zero out new table
        for (int i = 0; i < 512; i++)
            new_table[i] = 0;

        table[index] = (uint64_t)new_table | flags;
    }

    return (uint64_t*)(table[index] & ~0xFFF);
}

void vmm_init(void) {
    // Get current PML4 from CR3 — don't clear it
    // Bootloader already set up identity mapping, keep it
    __asm__("mov %%cr3, %0" : "=r"(pml4));
}

void vmm_map(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t* pdpt = get_or_create(pml4, PML4_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pdt  = get_or_create(pdpt, PDPT_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pt   = get_or_create(pdt,  PDT_INDEX(virt),  PAGE_PRESENT | PAGE_WRITABLE);

    pt[PT_INDEX(virt)] = phys | flags;
}

void vmm_unmap(uint64_t virt) {
    uint64_t* pdpt = get_or_create(pml4, PML4_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pdt  = get_or_create(pdpt, PDPT_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pt   = get_or_create(pdt,  PDT_INDEX(virt),  PAGE_PRESENT | PAGE_WRITABLE);

    pt[PT_INDEX(virt)] = 0;

    // Flush TLB for this page
    __asm__("invlpg (%0)" :: "r"(virt) : "memory");
}

uint64_t vmm_get_phys(uint64_t virt) {
    uint64_t* pdpt = get_or_create(pml4, PML4_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pdt  = get_or_create(pdpt, PDPT_INDEX(virt), PAGE_PRESENT | PAGE_WRITABLE);
    uint64_t* pt   = get_or_create(pdt,  PDT_INDEX(virt),  PAGE_PRESENT | PAGE_WRITABLE);

    return pt[PT_INDEX(virt)] & ~0xFFF;
}