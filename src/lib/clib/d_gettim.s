.intel_syntax noprefix

// FUNCTION: 0x000dc94d

.section _TEXT,"ax",@progbits,unique,903501
.globl "fn_000dc94d_"
.type "fn_000dc94d_",@function
"fn_000dc94d_":
    push edx
    push ecx
    push ebx
    mov ebx, eax
    mov ah, 0x2c
    int 0x21
    mov byte ptr [ebx], ch
    mov byte ptr [ebx + 1], cl
    mov byte ptr [ebx + 2], dh
    mov byte ptr [ebx + 3], dl
    sub eax, eax
    pop ebx
    pop ecx
    pop edx
    ret
.size "fn_000dc94d_",.-"fn_000dc94d_"
