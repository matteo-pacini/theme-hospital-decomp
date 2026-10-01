.intel_syntax noprefix

// FUNCTION: 0x000fde62

.section _TEXT,"ax",@progbits,unique,1039970
.globl "__Ergo_hook_init_"
.type "__Ergo_hook_init_",@function
"__Ergo_hook_init_":
    push ds
    mov ah, 0x35
    mov al, byte ptr ["___IRQ_int"]
    int 0x21
    mov dword ptr ["___PMAddr"], ebx
    .byte 0x66; .byte 0x8c; .byte 0x05; .long "___PMSeg"  # mov word ptr [0x33ad4], es
    mov ah, 0x25
    mov al, byte ptr ["___IRQ_int"]
    cmp byte ptr ["___MST_pic"], 0x20
    jne .L000fde91
    lea edx, ["__Enable_FPE_"+0x15]
    jmp .L000fde97
.L000fde91:
    lea edx, ["__Enable_FPE_"+0x22]
.L000fde97:
    mov bx, cs
    mov ds, ebx
    int 0x21
    pop ds
    ret
.size "__Ergo_hook_init_",.-"__Ergo_hook_init_"

// FUNCTION: 0x000fdea0

.section _TEXT,"ax",@progbits,unique,1040032
.globl "__Ergo_hook_fini_"
.type "__Ergo_hook_fini_",@function
"__Ergo_hook_fini_":
    push ds
    mov ah, 0x25
    mov al, byte ptr ["___IRQ_int"]
    mov edx, dword ptr ["___PMAddr"]
    .byte 0x66; .byte 0x8e; .byte 0x1d; .long "___PMSeg"  # mov ds, word ptr [0x33ad4]
    int 0x21
    pop ds
    ret
.size "__Ergo_hook_fini_",.-"__Ergo_hook_fini_"
