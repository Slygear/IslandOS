[BITS 16]
[ORG 0x7E00]

stage2_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Load kernel at 0x8000 BEFORE switching modes
    ; BIOS int 0x13 only works in real mode
    mov ah, 0x02
    mov al, 16          ; Read 16 sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Start at sector 10
    mov dh, 0           ; Head 0
    mov bx, 0x8000      ; Load kernel at 0x8000
    int 0x13
    jc disk_error

    ; Kernel loaded, now switch modes
    cli

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

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

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Set up paging for long mode
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 3072
    rep stosd
    mov edi, cr3

    ; PML4[0] -> PDPT at 0x2000
    mov dword [edi],        0x2003
    mov dword [edi + 4],    0x0

    ; PDPT[0] -> PDT at 0x3000
    add edi, 0x1000
    mov dword [edi],        0x3003
    mov dword [edi + 4],    0x0

    ; PDT[0] -> 2MB page (identity map)
    add edi, 0x1000
    mov dword [edi],        0x0083
    mov dword [edi + 4],    0x0

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable long mode in EFER
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_descriptor]
    jmp 0x08:long_mode

[BITS 64]
long_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000

    call 0x8000

halt64:
    cli
    hlt

; --- 32-bit GDT ---
gdt_start:
    dq 0
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; --- 64-bit GDT ---
gdt64_start:
    dq 0
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 10101111b, 0x00
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

err_msg db "Kernel load error!", 0x0D, 0x0A, 0