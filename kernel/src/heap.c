#include "heap.h"

typedef struct block {
    size_t        size;
    int           free;
    struct block* next;
} block_t;

static block_t* heap_head = 0;

void heap_init(void* start, size_t size) {
    heap_head        = (block_t*)start;
    heap_head->size  = size - sizeof(block_t);
    heap_head->free  = 1;
    heap_head->next  = 0;
}

void* kmalloc(size_t size) {
    if (!size) return 0;
    size = (size + 7) & ~7;

    block_t* curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            // Split if enough space
            if (curr->size >= size + sizeof(block_t) + 8) {
                block_t* next = (block_t*)((uint8_t*)curr + sizeof(block_t) + size);
                next->size    = curr->size - size - sizeof(block_t);
                next->free    = 1;
                next->next    = curr->next;
                curr->size    = size;
                curr->next    = next;
            }
            curr->free = 0;
            return (void*)((uint8_t*)curr + sizeof(block_t));
        }
        curr = curr->next;
    }
    return 0;
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    block->free = 1;

    // Merge adjacent free blocks
    block_t* curr = heap_head;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += sizeof(block_t) + curr->next->size;
            curr->next  = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr)  return kmalloc(size);
    if (!size) { kfree(ptr); return 0; }

    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    if (block->size >= size) return ptr;

    void* new_ptr = kmalloc(size);
    if (!new_ptr) return 0;

    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < block->size; i++)
        dst[i] = src[i];

    kfree(ptr);
    return new_ptr;
}