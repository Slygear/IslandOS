#include "vga.h"

void kernel_main(void) {
    vga_init();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("IslandOS v0.1");
    vga_println("==============");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("Bootloader:  OK");
    vga_println("Long mode:   OK");
    vga_println("Kernel:      OK");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("\nWelcome, John.");
    vga_println("The Island is alive.");

    for (;;) __asm__("hlt");
}