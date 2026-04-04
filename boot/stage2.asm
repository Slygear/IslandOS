[BITS 16]
[ORG 0x7E00]

stage2_start:
    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print "S2:OK"
    mov ah, 0x0E
    mov al, 'S'
    int 0x10
    mov al, '2'
    int 0x10
    mov al, ':'
    int 0x10
    mov al, 'O'
    int 0x10
    mov al, 'K'
    int 0x10

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

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Zero out BSS region
    mov edi, 0x78000
    mov ecx, 0x2000
    xor eax, eax
    rep stosd

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

    ; Identity map first 8MB (4 x 2MB pages)
    add edi, 0x1000
    mov dword [edi],        0x0083
    mov dword [edi + 4],    0x0
    mov dword [edi + 8],    0x200083
    mov dword [edi + 12],   0x0
    mov dword [edi + 16],   0x400083
    mov dword [edi + 20],   0x0
    mov dword [edi + 24],   0x600083
    mov dword [edi + 28],   0x0

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

    mov rax, 0x70000
    call rax

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

boot_drive db 0