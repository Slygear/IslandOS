#include "shell.h"
#include "vga.h"
#include "vfs.h"

#define MAX_CMD  256
#define MAX_ARGS 16

static char cmd_buf[MAX_CMD];
static int  cmd_len      = 0;
static char cwd[256]     = "/";
static int  cursor_blink = 0;
static int  cursor_shown = 0;
static int  saved_x      = 0;
static int  saved_y      = 0;

static int str_eq(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == 0 && *b == 0;
}

static int str_len(const char* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}

static void str_copy(char* dst, const char* src) {
    int i = 0;
    while (src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static int parse_args(char* cmd, char* args[], int max_args) {
    int argc = 0, i = 0, len = str_len(cmd);
    while (i < len && argc < max_args) {
        while (i < len && cmd[i] == ' ') i++;
        if (i >= len) break;
        args[argc++] = &cmd[i];
        while (i < len && cmd[i] != ' ') i++;
        if (i < len) cmd[i++] = 0;
    }
    return argc;
}

static void build_path(char* out, const char* base, const char* name) {
    if (name[0] == '/') { str_copy(out, name); return; }
    str_copy(out, base);
    int len = str_len(out);
    if (out[len - 1] != '/') { out[len] = '/'; out[len+1] = 0; len++; }
    str_copy(out + len, name);
}

static void cmd_help(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_println("IslandOS Shell — commands:");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_println("  help, clear, ls, pwd, cd, cat, echo, mkdir, write, uname");
}

static void cmd_ls(void) {
    vfs_node_t* dir = vfs_find(cwd);
    if (!dir) { vga_println("ls: not found"); return; }
    for (int i = 0; i < dir->child_count; i++) {
        vfs_node_t* child = dir->children[i];
        if (child->type == VFS_DIR) {
            vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
            vga_print(child->name);
            vga_println("/");
        } else {
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_println(child->name);
        }
    }
    vga_set_color(VGA_WHITE, VGA_BLACK);
}

static void cmd_cd(char* path) {
    if (!path) { str_copy(cwd, "/"); return; }
    char full[256];
    build_path(full, cwd, path);
    vfs_node_t* dir = vfs_find(full);
    if (!dir || dir->type != VFS_DIR) {
        vga_print("cd: not found: "); vga_println(path); return;
    }
    str_copy(cwd, full);
}

static void cmd_cat(char* path) {
    if (!path) { vga_println("cat: missing filename"); return; }
    char full[256];
    build_path(full, cwd, path);
    vfs_node_t* f = vfs_find(full);
    if (!f) { vga_print("cat: not found: "); vga_println(full); return; }
    if (f->type != VFS_FILE) { vga_println("cat: not a file"); return; }
    uint8_t buf[256] = {0};
    vfs_read(f, buf, 255, 0);
    vga_println((char*)buf);
}

static void cmd_echo(char** args, int argc) {
    for (int i = 1; i < argc; i++) {
        vga_print(args[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
}

static void cmd_mkdir(char* path) {
    if (!path) { vga_println("mkdir: missing name"); return; }
    char full[256];
    build_path(full, cwd, path);
    vfs_mkdir(full);
}

static void cmd_write(char* path, char* text) {
    if (!path || !text) { vga_println("write: missing args"); return; }
    char full[256];
    build_path(full, cwd, path);
    vfs_node_t* f = vfs_find(full);
    if (!f) f = vfs_mkfile(full);
    if (!f) { vga_println("write: failed"); return; }
    vfs_write(f, (uint8_t*)text, str_len(text), 0);
    vga_println("Written.");
}

static void execute(char* cmd) {
    char* args[MAX_ARGS];
    int   argc = parse_args(cmd, args, MAX_ARGS);
    if (argc == 0) return;

    if (str_eq(args[0], "help"))  { cmd_help(); return; }
    if (str_eq(args[0], "clear")) { vga_clear(); return; }
    if (str_eq(args[0], "ls"))    { cmd_ls();   return; }
    if (str_eq(args[0], "pwd"))   { vga_println(cwd); return; }
    if (str_eq(args[0], "cd"))    { cmd_cd(argc > 1 ? args[1] : 0); return; }
    if (str_eq(args[0], "cat"))   { cmd_cat(argc > 1 ? args[1] : 0); return; }
    if (str_eq(args[0], "echo"))  { cmd_echo(args, argc); return; }
    if (str_eq(args[0], "mkdir")) { cmd_mkdir(argc > 1 ? args[1] : 0); return; }
    if (str_eq(args[0], "write")) { cmd_write(argc > 1 ? args[1] : 0, argc > 2 ? args[2] : 0); return; }
    if (str_eq(args[0], "uname")) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        vga_println("IslandOS v0.1 x86_64");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        return;
    }

    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_print("Unknown command: ");
    vga_println(args[0]);
    vga_set_color(VGA_WHITE, VGA_BLACK);
}

static void print_prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print(cwd);
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print(" > ");
    saved_x      = vga_get_x();
    saved_y      = vga_get_y();
    cursor_shown = 0;
    cursor_blink = 0;
}

void shell_cursor_tick(void) {
    cursor_blink++;
    if (cursor_blink < 50) return;
    cursor_blink = 0;
    cursor_shown = !cursor_shown;
    vga_put_at(cursor_shown ? '_' : ' ', saved_x, saved_y, VGA_WHITE, VGA_BLACK);
}

void shell_init(void) {
    cmd_len = 0;
    vga_println("");
    print_prompt();
}

void shell_input(char c) {
    // Hide cursor while typing
    vga_put_at(' ', saved_x, saved_y, VGA_WHITE, VGA_BLACK);

    if (c == '\n' || c == '\r') {
        cmd_buf[cmd_len] = 0;
        vga_putchar('\n');
        if (cmd_len > 0) execute(cmd_buf);
        cmd_len = 0;
        print_prompt();
    } else if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            vga_move_back();
            vga_put_at(' ', vga_get_x(), vga_get_y(), VGA_WHITE, VGA_BLACK);
            saved_x = vga_get_x();
            saved_y = vga_get_y();
        }
        saved_x = vga_get_x();
        saved_y = vga_get_y();
    } else if (c >= 32 && c < 127 && cmd_len < MAX_CMD - 1) {
        cmd_buf[cmd_len++] = c;
        vga_putchar(c);
        saved_x = vga_get_x();
        saved_y = vga_get_y();
    }
}