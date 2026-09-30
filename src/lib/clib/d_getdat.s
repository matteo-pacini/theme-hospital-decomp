.intel_syntax noprefix

// FUNCTION: 0x000dc932

.section _TEXT,"ax",@progbits,unique,903474
.globl "_dos_getdate_"
.type "_dos_getdate_",@function
"_dos_getdate_":
    push edx
    push ecx
    push ebx
    mov ebx, eax
    mov ah, 0x2a
    int 0x21
    mov word ptr [ebx + 2], cx
    mov byte ptr [ebx + 1], dh
    mov byte ptr [ebx], dl
    mov byte ptr [ebx + 4], al
    sub eax, eax
    pop ebx
    pop ecx
    pop edx
    ret
.size "_dos_getdate_",.-"_dos_getdate_"
