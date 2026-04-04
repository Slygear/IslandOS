[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; Load stage2 (sector 2, 1 sector)
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_stage2
    int 0x13
    jc disk_error

    ; Load kernel (sector 3, 32 sectors) at 0x70000
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_kernel
    int 0x13
    jc disk_error

    mov dl, [boot_drive]
    jmp 0x7E00

disk_error:
    mov si, err_msg
.loop:
    lodsb
    or al, al
    jz halt
    mov ah, 0x0E
    int 0x10
    jmp .loop

halt:
    cli
    hlt

dap_stage2:
    db 0x10
    db 0x00
    dw 1            ; 1 sector
    dw 0x7E00       ; offset
    dw 0x0000       ; segment
    dq 1            ; LBA sector 1

dap_kernel:
    db 0x10
    db 0x00
    dw 32           ; 32 sectors
    dw 0x0000       ; offset
    dw 0x7000       ; segment = 0x70000
    dq 3            ; LBA sector 3

boot_drive db 0
err_msg db "Disk error!", 0

times 510 - ($ - $$) db 0
dw 0xAA55