#include "vga.h"
#include "pmm.h"
#include "heap.h"
#include "idt.h"

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

static void pic_remap(void) {
    // Remap PIC IRQs to 0x20-0x2F
    __asm__("outb %0, %1" :: "a"((uint8_t)0x11), "Nd"((uint16_t)0x20));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x11), "Nd"((uint16_t)0xA0));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x20), "Nd"((uint16_t)0x21));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x28), "Nd"((uint16_t)0xA1));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x04), "Nd"((uint16_t)0x21));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x02), "Nd"((uint16_t)0xA1));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x01), "Nd"((uint16_t)0x21));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x01), "Nd"((uint16_t)0xA1));
    // Unmask all IRQs
    __asm__("outb %0, %1" :: "a"((uint8_t)0x00), "Nd"((uint16_t)0x21));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x00), "Nd"((uint16_t)0xA1));
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
    vga_println("Heap:        OK");

    pic_remap();
    idt_init();
    __asm__("sti");
    vga_println("IDT:         OK");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_println("Welcome, John.");
    vga_println("The Island is alive.");
    vga_println("");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("> ");

    for (;;) __asm__("hlt");
}