ENTRY(kernel_main)

SECTIONS {
    . = 0x8000;

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
        *(COMMON)
        *(.bss*)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
    }
}