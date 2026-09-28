.intel_syntax noprefix

// FUNCTION: 0x000d12aa

.section _TEXT,"ax",@progbits,unique,856746
.globl "rename_"
.type "rename_",@function
"rename_":
    push ecx
    push edi
    mov ecx, ds
    mov edi, edx
    mov edx, eax
    push ebx
    push es
    mov es, ecx
    mov ah, 0x56
    int 0x21
    rcl eax, 1
    ror eax, 1
    pop es
    pop ebx
    mov edx, eax
    test eax, eax
    jge .L000d12d3
    xor eax, eax
    mov ax, dx
    call "__set_errno_dos_"
    pop edi
    pop ecx
    ret
.L000d12d3:
    xor eax, eax
    pop edi
    pop ecx
    ret
.size "rename_",.-"rename_"
