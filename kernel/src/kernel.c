#include "vga.h"
#include "pmm.h"
#include "vmm.h"

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
    vga_init();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("IslandOS v0.1");
    vga_println("==============");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("Bootloader:  OK");
    vga_println("Long mode:   OK");
    vga_println("Kernel:      OK");

    // PMM
    pmm_init(0x100000, 32 * 1024 * 1024);
    vga_set_color(VGA_LIGHT_BROWN, VGA_BLACK);
    vga_print("PMM:         OK — free pages: ");
    print_num(pmm_free_pages());
    vga_println("");

    // VMM
    vmm_init();
    vga_print("VMM:         OK — mapped: ");
    print_hex(vmm_get_phys(0x8000));
    vga_println("");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("\nWelcome, John.");
    vga_println("The Island is alive.");

    for (;;) __asm__("hlt");
}