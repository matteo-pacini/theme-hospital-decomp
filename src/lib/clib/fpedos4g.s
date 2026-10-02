.intel_syntax noprefix

// __DOS4G_hook_init: save the __FPE_int interrupt vector and point it at the FPU exception handler (DOS/4G).
// FUNCTION: 0x000fde1c

.section _TEXT,"ax",@progbits,unique,1039900
.globl "__DOS4G_hook_init_"
.type "__DOS4G_hook_init_",@function
"__DOS4G_hook_init_":
    push ds
    mov ah, 0x35
    mov al, byte ptr ["___FPE_int"]
    int 0x21
    mov dword ptr ["___PMAddr"], ebx
    .byte 0x66; .byte 0x8c; .byte 0x05; .long "___PMSeg"  # mov word ptr [0x33ad4], es
    mov ah, 0x25
    mov al, byte ptr ["___FPE_int"]
    mov dx, cs
    mov ds, edx
    lea edx, ["__Enable_FPE_"+0x22]
    int 0x21
    pop ds
    ret
.size "__DOS4G_hook_init_",.-"__DOS4G_hook_init_"

// __DOS4G_hook_fini: restore the saved __FPE_int interrupt vector.
// FUNCTION: 0x000fde49

.section _TEXT,"ax",@progbits,unique,1039945
.globl "__DOS4G_hook_fini_"
.type "__DOS4G_hook_fini_",@function
"__DOS4G_hook_fini_":
    push ds
    mov ah, 0x25
    mov al, byte ptr ["___FPE_int"]
    mov edx, dword ptr ["___PMAddr"]
    .byte 0x66; .byte 0x8e; .byte 0x1d; .long "___PMSeg"  # mov ds, word ptr [0x33ad4]
    int 0x21
    pop ds
    ret
.size "__DOS4G_hook_fini_",.-"__DOS4G_hook_fini_"
