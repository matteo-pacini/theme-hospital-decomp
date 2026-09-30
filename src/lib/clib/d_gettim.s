.intel_syntax noprefix

// FUNCTION: 0x000dc94d

.section _TEXT,"ax",@progbits,unique,903501
.globl "_dos_gettime_"
.type "_dos_gettime_",@function
"_dos_gettime_":
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
.size "_dos_gettime_",.-"_dos_gettime_"
