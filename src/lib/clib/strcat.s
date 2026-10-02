.intel_syntax noprefix

// strcat: append string src to dst; returns dst.
// FUNCTION: 0x000c93bf

.section _TEXT,"ax",@progbits,unique,824255
.globl "strcat_"
.type "strcat_",@function
"strcat_":
    push ecx
    push esi
    push edi
    mov esi, edx
    mov edi, eax
    push es
    push ds
    pop es
    push edi
    {load} sub ecx, ecx
    dec ecx
    mov al, 0
    repne scasb al, byte ptr es:[edi]
    dec edi
.L000c93d2:
    mov al, byte ptr [esi]
    mov byte ptr [edi], al
    cmp al, 0
    je .L000c93ea
    mov al, byte ptr [esi + 1]
    add esi, 2
    mov byte ptr [edi + 1], al
    add edi, 2
    cmp al, 0
    jne .L000c93d2
.L000c93ea:
    pop edi
    pop es
    mov eax, edi
    pop edi
    pop esi
    pop ecx
    ret
.size "strcat_",.-"strcat_"
