.intel_syntax noprefix

// FUNCTION: 0x000cd350

.section _TEXT,"ax",@progbits,unique,840528
.globl "strcmp_"
.type "strcmp_",@function
"strcmp_":
    push ebx
    push ecx
    mov ebx, eax
    cmp eax, edx
    je .L000cd3c4
.L000cd358:
    mov eax, dword ptr [ebx]
    mov ecx, dword ptr [edx]
    cmp ecx, eax
    jne .L000cd3c9
    not ecx
    add eax, 0xfefefeff
    and eax, ecx
    and eax, 0x80808080
    jne .L000cd3c4
    mov eax, dword ptr [ebx + 4]
    mov ecx, dword ptr [edx + 4]
    cmp ecx, eax
    jne .L000cd3c9
    not ecx
    add eax, 0xfefefeff
    and eax, ecx
    and eax, 0x80808080
    jne .L000cd3c4
    mov eax, dword ptr [ebx + 8]
    mov ecx, dword ptr [edx + 8]
    cmp ecx, eax
    jne .L000cd3c9
    not ecx
    add eax, 0xfefefeff
    and eax, ecx
    and eax, 0x80808080
    jne .L000cd3c4
    mov eax, dword ptr [ebx + 0xc]
    mov ecx, dword ptr [edx + 0xc]
    cmp ecx, eax
    jne .L000cd3c9
    add ebx, 0x10
    add edx, 0x10
    not ecx
    add eax, 0xfefefeff
    and eax, ecx
    and eax, 0x80808080
    je .L000cd358
.L000cd3c4:
    sub eax, eax
    pop ecx
    pop ebx
    ret
.L000cd3c9:
    cmp al, cl
    jne .L000cd3ea
    cmp al, 0
    je .L000cd3c4
    cmp ah, ch
    jne .L000cd3ea
    cmp ah, 0
    je .L000cd3c4
    shr eax, 0x10
    shr ecx, 0x10
    cmp al, cl
    jne .L000cd3ea
    cmp al, 0
    je .L000cd3c4
    cmp ah, ch
.L000cd3ea:
    sbb eax, eax
    or al, 1
    pop ecx
    pop ebx
    ret
.size "strcmp_",.-"strcmp_"
