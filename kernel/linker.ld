ENTRY(kernel_main)

SECTIONS {
    . = 0x70000;

    .text : {
        KEEP(*(.text.kernel_main))
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
    }
}