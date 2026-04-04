#include "vga.h"

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0;

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void) {
    current_color = (VGA_BLACK << 4) | VGA_WHITE;
    vga_clear();
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_ADDR[y * VGA_WIDTH + x] = vga_entry(' ', current_color);
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = (bg << 4) | fg;
}

static void vga_scroll(void) {
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_ADDR[y * VGA_WIDTH + x] = VGA_ADDR[(y + 1) * VGA_WIDTH + x];

    for (int x = 0; x < VGA_WIDTH; x++)
        VGA_ADDR[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);

    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        VGA_ADDR[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, current_color);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= VGA_HEIGHT)
        vga_scroll();
}

void vga_print(const char* str) {
    while (*str)
        vga_putchar(*str++);
}

void vga_println(const char* str) {
    vga_print(str);
    vga_putchar('\n');
}