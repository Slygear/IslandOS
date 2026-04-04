[BITS 16]
[ORG 0x7C00]

start:
    ; Clear segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Save boot drive number (BIOS puts it in DL)
    mov [boot_drive], dl

    ; Load stage2 from disk
    mov ah, 0x02
    mov al, 8
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x7E00
    int 0x13

    ; Check for error
    jc disk_error

    ; Jump to stage2
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

boot_drive db 0
err_msg db "Disk read error!", 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55