#include "vga.h"

// Store state in a struct at a fixed known address
typedef struct {
    int cursor_x;
    int cursor_y;
    uint8_t color;
} vga_state_t;

static vga_state_t* const state = (vga_state_t*)0x500;

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void) {
    state->cursor_x = 0;
    state->cursor_y = 0;
    state->color    = (VGA_BLACK << 4) | VGA_WHITE;

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_ADDR[i] = vga_entry(' ', state->color);
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_ADDR[i] = vga_entry(' ', state->color);
    state->cursor_x = 0;
    state->cursor_y = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    state->color = (bg << 4) | fg;
}

static void vga_scroll(void) {
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_ADDR[y * VGA_WIDTH + x] = VGA_ADDR[(y + 1) * VGA_WIDTH + x];
    for (int x = 0; x < VGA_WIDTH; x++)
        VGA_ADDR[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', state->color);
    state->cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        state->cursor_x = 0;
        state->cursor_y++;
    } else if (c == '\r') {
        state->cursor_x = 0;
    } else {
        VGA_ADDR[state->cursor_y * VGA_WIDTH + state->cursor_x] = vga_entry(c, state->color);
        state->cursor_x++;
        if (state->cursor_x >= VGA_WIDTH) {
            state->cursor_x = 0;
            state->cursor_y++;
        }
    }
    if (state->cursor_y >= VGA_HEIGHT)
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