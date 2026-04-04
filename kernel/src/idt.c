#include "idt.h"
#include "vga.h"

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

// Exception names
static const char* exception_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Math Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
};

// ISR stubs declared in isr.asm
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void irq0(void);
extern void irq1(void);

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].offset_mid  = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector    = sel;
    idt[num].ist         = 0;
    idt[num].type_attr   = flags;
    idt[num].zero        = 0;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint64_t)&idt;

    // CPU exceptions
    idt_set_gate(0,  (uint64_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint64_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint64_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint64_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint64_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint64_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint64_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint64_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint64_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint64_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);

    // IRQs (remapped to 0x20-0x2F)
    idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);  // Keyboard

    __asm__("lidt %0" :: "m"(idt_ptr));
}

// Called from isr.asm
void isr_handler(registers_t* regs) {
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_print("EXCEPTION: ");
    if (regs->int_no < 20)
        vga_println(exception_names[regs->int_no]);
    else
        vga_println("Unknown");
    for (;;) __asm__("hlt");
}

// Called from isr.asm for IRQs
void irq_handler(registers_t* regs) {
    if (regs->int_no == 33) {
        // IRQ1 — keyboard
        uint8_t scancode;
        __asm__("inb %1, %0" : "=a"(scancode) : "Nd"((uint16_t)0x60));
        
        // Simple scancode to ASCII (US layout, no shift)
        static const char scancode_map[] = {
            0, 0, '1','2','3','4','5','6','7','8','9','0','-','=',
            '\b', '\t',
            'q','w','e','r','t','y','u','i','o','p','[',']','\n',
            0,
            'a','s','d','f','g','h','j','k','l',';','\'','`',
            0,'\\',
            'z','x','c','v','b','n','m',',','.','/',
            0, '*', 0, ' '
        };

        if (scancode < sizeof(scancode_map) && scancode_map[scancode])
            vga_putchar(scancode_map[scancode]);
    }

    // Send EOI
    if (regs->int_no >= 40)
        __asm__("outb %0, %1" :: "a"((uint8_t)0x20), "Nd"((uint16_t)0xA0));
    __asm__("outb %0, %1" :: "a"((uint8_t)0x20), "Nd"((uint16_t)0x20));
}