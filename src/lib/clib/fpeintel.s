.intel_syntax noprefix

// __Intel_hook_init: save the __FPE_int protected- and real-mode vectors and set the protected-mode one to the FPU exception handler (DPMI 204h/200h/205h).
// FUNCTION: 0x000fdeba

.section _TEXT,"ax",@progbits,unique,1040058
.globl "__Intel_hook_init_"
.type "__Intel_hook_init_",@function
"__Intel_hook_init_":
    mov ax, 0x204
    mov bl, byte ptr ["___FPE_int"]
    int 0x31
    mov dword ptr ["___PMAddr"], edx
    mov word ptr ["___PMSeg"], cx
    mov ax, 0x200
    mov bl, byte ptr ["___FPE_int"]
    int 0x31
    shl ecx, 0x10
    mov cx, dx
    mov dword ptr ["___RMAddr"], ecx
    mov cx, cs
    cmp byte ptr ["___MST_pic"], 0x20
    jne .L000fdeff
    lea edx, ["__Enable_FPE_"+0x15]
    jmp .L000fdf05
.L000fdeff:
    lea edx, ["__Enable_FPE_"+0x22]
.L000fdf05:
    mov ax, 0x205
    mov bl, byte ptr ["___FPE_int"]
    int 0x31
    ret
.size "__Intel_hook_init_",.-"__Intel_hook_init_"

// __Intel_hook_fini: returns at once; the vector __Intel_hook_init set is not restored.
// FUNCTION: 0x000fdf12

.section _TEXT,"ax",@progbits,unique,1040146
.globl "__Intel_hook_fini_"
.type "__Intel_hook_fini_",@function
"__Intel_hook_fini_":
    ret
.size "__Intel_hook_fini_",.-"__Intel_hook_fini_"
