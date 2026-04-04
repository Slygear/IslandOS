#include "pmm.h"

// Bitmap based physical memory manager
// Each bit represents one 4KB page: 0 = free, 1 = used

static uint8_t  bitmap[32768];   // Tracks up to 1GB of memory
static uint64_t total_pages = 0;
static uint64_t free_pages  = 0;
static uint64_t mem_base    = 0;

static void bitmap_set(uint64_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static void bitmap_clear(uint64_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static int bitmap_test(uint64_t page) {
    return bitmap[page / 8] & (1 << (page % 8));
}

void pmm_init(uint64_t mem_start, uint64_t mem_size) {
    mem_base    = mem_start;
    total_pages = mem_size / PAGE_SIZE;
    free_pages  = total_pages;

    // Mark all pages as free
    for (uint64_t i = 0; i < total_pages / 8; i++)
        bitmap[i] = 0;

    // Mark first 1MB as used (BIOS, VGA, bootloader)
    uint64_t reserved = 0x100000 / PAGE_SIZE;
    for (uint64_t i = 0; i < reserved; i++) {
        bitmap_set(i);
        free_pages--;
    }
}

void* pmm_alloc(void) {
    for (uint64_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_pages--;
            return (void*)(mem_base + i * PAGE_SIZE);
        }
    }
    return 0;   // Out of memory
}

void pmm_free(void* addr) {
    uint64_t page = ((uint64_t)addr - mem_base) / PAGE_SIZE;
    bitmap_clear(page);
    free_pages++;
}

uint64_t pmm_free_pages(void) {
    return free_pages;
}