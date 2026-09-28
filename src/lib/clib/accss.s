.intel_syntax noprefix

// FUNCTION: 0x000d0b5c

.section _TEXT,"ax",@progbits,unique,854876
.globl "access_"
.type "access_",@function
"access_":
    push ebx
    push ecx
    mov ebx, edx
    mov edx, eax
    xor eax, eax
    mov ah, 0x43
    int 0x21
    jb .L000d0b79
    test bl, 2
    je .L000d0b79
    test cl, 1
    je .L000d0b79
    mov al, 0
    mov ah, 6
    stc
.L000d0b79:
    sbb ecx, ecx
    mov cx, ax
    mov eax, ecx
    mov ebx, eax
    mov edx, eax
    and ebx, 0xffff
    shr edx, 0x10
    mov eax, ebx
    call "_dosret0_"
    pop ecx
    pop ebx
    ret
.size "access_",.-"access_"
