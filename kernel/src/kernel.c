#include "vga.h"
#include "pmm.h"
#include "heap.h"

extern uint64_t __bss_start;
extern uint64_t __bss_end;

static void print_num(uint64_t n) {
    char buf[21];
    int i = 20;
    buf[i] = 0;
    if (n == 0) { vga_putchar('0'); return; }
    while (n > 0) {
        buf[--i] = '0' + (n % 10);
        n /= 10;
    }
    vga_print(&buf[i]);
}

static void print_hex(uint64_t n) {
    char buf[17];
    int i = 16;
    buf[i] = 0;
    if (n == 0) { vga_print("0x0"); return; }
    while (n > 0) {
        int d = n % 16;
        buf[--i] = d < 10 ? '0' + d : 'A' + d - 10;
        n /= 16;
    }
    vga_print("0x");
    vga_print(&buf[i]);
}

void kernel_main(void) {
    uint64_t* bss = &__bss_start;
    while (bss < &__bss_end)
        *bss++ = 0;

    vga_init();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("IslandOS v0.1");
    vga_println("==============");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("Bootloader:  OK");
    vga_println("Long mode:   OK");
    vga_println("Kernel:      OK");

    pmm_init(0x200000, 32 * 1024 * 1024);
    vga_set_color(VGA_LIGHT_BROWN, VGA_BLACK);
    vga_print("PMM:         OK — free pages: ");
    print_num(pmm_free_pages());
    vga_println("");

    void* heap_mem = pmm_alloc();
    heap_init(heap_mem, PAGE_SIZE);
    uint32_t* a = (uint32_t*)kmalloc(4);
    uint32_t* b = (uint32_t*)kmalloc(4);
    a[0] = 0xDEAD;
    b[0] = 0xBEEF;
    kfree(a);
    vga_print("Heap:        OK — b[0]=");
    print_hex(b[0]);
    vga_println("");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("\nWelcome, John.");
    vga_println("The Island is alive.");

    for (;;) __asm__("hlt");
}