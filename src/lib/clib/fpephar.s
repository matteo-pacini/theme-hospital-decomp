.intel_syntax noprefix

// __Phar_hook_init: save the FPU IRQ's vectors and install the FPU exception handler through Phar Lap's 25xxh calls (older versions also patch the IDT).
// FUNCTION: 0x000fdd2c

.section _TEXT,"ax",@progbits,unique,1039660
.globl "__Phar_hook_init_"
.type "__Phar_hook_init_",@function
"__Phar_hook_init_":
    push ds
    cmp byte ptr ["___MST_pic"], 0x20
    jne .L000fdd3e
    lea edx, ["__Enable_FPE_"+0x15]
    jmp .L000fdd44
.L000fdd3e:
    lea edx, ["__Enable_FPE_"+0x22]
.L000fdd44:
    cmp byte ptr ["__Extender"], 3
    jae .L000fdd6b
    mov cl, 0x50
    mov ax, 0x2503
    int 0x21
    push ebx
    mov ax, 0x2502
    int 0x21
    push ebx
    push es
    push ds
    mov ax, cs
    mov ds, eax
    mov ax, 0x2506
    int 0x21
    pop ds
.L000fdd6b:
    mov ax, 0x250c
    int 0x21
    mov cl, ah
    add cl, byte ptr ["___IRQ_num"]
    sub cl, 8
    mov ax, 0x2502
    int 0x21
    mov dword ptr ["___PMAddr"], ebx
    .byte 0x66; .byte 0x8c; .byte 0x05; .long "___PMSeg"  # mov word ptr [0x33ad4], es
    mov ax, 0x2503
    int 0x21
    mov dword ptr ["___RMAddr"], ebx
    cmp byte ptr ["__Extender"], 3
    jae .L000fdde2
    sub esp, 8
    sidt [esp]
    mov ax, 0x60
    mov es, eax
    mov ebx, dword ptr [esp + 2]
    add esp, 8
    mov eax, dword ptr es:[ebx + 0x280]
    mov edx, dword ptr es:[ebx + 0x284]
    movzx ecx, cl
    shl ecx, 3
    mov dword ptr es:[ebx + ecx], eax
    mov dword ptr es:[ebx + ecx + 4], edx
    mov cl, 0x50
    pop ds
    pop edx
    pop ebx
    mov ax, 0x2507
    int 0x21
    jmp .L000fdded
.L000fdde2:
    mov ax, cs
    mov ds, eax
    mov ax, 0x2506
    int 0x21
.L000fdded:
    pop ds
    ret
.size "__Phar_hook_init_",.-"__Phar_hook_init_"

// __Phar_hook_fini: restore the saved FPU IRQ vectors (Phar Lap 2507h).
// FUNCTION: 0x000fddef

.section _TEXT,"ax",@progbits,unique,1039855
.globl "__Phar_hook_fini_"
.type "__Phar_hook_fini_",@function
"__Phar_hook_fini_":
    push ds
    mov ax, 0x250c
    int 0x21
    mov cl, ah
    add cl, byte ptr ["___IRQ_num"]
    sub cl, 8
    mov ebx, dword ptr ["___RMAddr"]
    mov edx, dword ptr ["___PMAddr"]
    .byte 0x66; .byte 0x8e; .byte 0x1d; .long "___PMSeg"  # mov ds, word ptr [0x33ad4]
    mov ax, 0x2507
    int 0x21
    pop ds
    ret
.size "__Phar_hook_fini_",.-"__Phar_hook_fini_"
