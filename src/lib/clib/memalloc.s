.intel_syntax noprefix

// FUNCTION: 0x000ebbb4

.section _TEXT,"ax",@progbits,unique,965556
.globl "__MemAllocator"
.type "__MemAllocator",@function
"__MemAllocator":
    push esi
    push edi
    push ecx
    push ds
    mov ds, edx
    or eax, eax
    je .L000ebc03
    add eax, 7
    jb .L000ebc03
    and al, 0xfc
    mov edx, 0xc
    sub eax, edx
    cmc
    sbb ecx, ecx
    and eax, ecx
    add eax, edx
    cmp eax, dword ptr [ebx + 0x14]
    ja .L000ebc03
    mov esi, dword ptr [ebx + 0xc]
    mov edi, dword ptr [ebx + 0x10]
    cmp eax, edi
    ja .L000ebbe7
    mov esi, dword ptr [ebx + 0x28]
    sub edi, edi
.L000ebbe7:
    mov edx, dword ptr [esi]
    cmp eax, edx
    jbe .L000ebc07
    sub edi, edx
    cmc
    sbb ecx, ecx
    and edi, ecx
    add edi, edx
    lea edx, [ebx + 0x20]
    mov esi, dword ptr [esi + 8]
    cmp esi, edx
    jne .L000ebbe7
    mov dword ptr [ebx + 0x14], edi
.L000ebc03:
    sub eax, eax
    jmp .L000ebc52
.L000ebc07:
    sub edx, eax
    mov dword ptr [ebx + 0x10], edi
    inc dword ptr [ebx + 0x18]
    mov ecx, dword ptr [esi + 8]
    cmp edx, 0xc
    jb .L000ebc35
    mov edi, esi
    add edi, eax
    mov dword ptr [ebx + 0xc], edi
    mov dword ptr [edi], edx
    mov dword ptr [esi], eax
    mov ebx, dword ptr [esi + 4]
    mov dword ptr [edi + 4], ebx
    mov dword ptr [edi + 8], ecx
    mov dword ptr [ebx + 8], edi
    mov ebx, ecx
    mov dword ptr [ebx + 4], edi
    jmp .L000ebc4a
.L000ebc35:
    dec dword ptr [ebx + 0x1c]
    mov eax, esi
    mov edi, ecx
    mov esi, dword ptr [esi + 4]
    mov dword ptr [esi + 8], edi
    mov dword ptr [edi + 4], esi
    mov dword ptr [ebx + 0xc], esi
    mov esi, eax
.L000ebc4a:
    or dword ptr [esi], 1
    mov eax, esi
    add eax, 4
.L000ebc52:
    pop ds
    pop ecx
    pop edi
    pop esi
    ret
    .byte 0xe9; .long "__MemFree"+0x106-.-4
.size "__MemAllocator",.-"__MemAllocator"

// FUNCTION: 0x000ebc5c

.section _TEXT,"ax",@progbits,unique,965724
.globl "__MemFree"
.type "__MemFree",@function
"__MemFree":
    push esi
    push edi
    push ecx
    push ds
    mov ds, edx
    or eax, eax
    .byte 0x74; .byte 0xf1  # je 0xebc57
    mov esi, eax
    sub esi, 4
    mov eax, dword ptr [esi]
    test al, 1
    .byte 0x74; .byte 0xe6  # je 0xebc57
    and al, 0xfe
    mov edi, esi
    add edi, eax
    test dword ptr [edi], 1
    jne .L000ebc9e
    cmp edi, dword ptr [ebx + 0xc]
    jne .L000ebc87
    mov dword ptr [ebx + 0xc], esi
.L000ebc87:
    add eax, dword ptr [edi]
    mov dword ptr [esi], eax
    push ebx
    mov ebx, dword ptr [edi + 4]
    mov edi, dword ptr [edi + 8]
    mov dword ptr [ebx + 8], edi
    mov dword ptr [edi + 4], ebx
    pop ebx
    dec dword ptr [ebx + 0x1c]
    jmp .L000ebd1a
.L000ebc9e:
    mov dword ptr [esi], eax
    mov edi, dword ptr [ebx + 0xc]
    cmp esi, edi
    jae .L000ebcb5
    cmp esi, dword ptr [edi + 4]
    ja .L000ebd1a
    mov edi, dword ptr [ebx + 0x28]
    cmp esi, edi
    jb .L000ebd1a
    jmp .L000ebcc4
.L000ebcb5:
    mov edi, dword ptr [edi + 8]
    cmp esi, edi
    jb .L000ebd1a
    lea edi, [ebx + 0x20]
    cmp esi, dword ptr [ebx + 0x24]
    ja .L000ebd1a
.L000ebcc4:
    sub edx, edx
    mov eax, dword ptr [ebx + 0x18]
    mov ecx, dword ptr [ebx + 0x1c]
    inc ecx
    div ecx
    dec ecx
    cmp eax, ecx
    jae .L000ebcf9
    mov edx, dword ptr [ebx + 0x18]
    sub edx, ecx
    add eax, eax
    cmp edx, ecx
    ja .L000ebce1
    sub eax, eax
.L000ebce1:
    mov edi, esi
    add edi, dword ptr [esi]
.L000ebce5:
    mov edx, dword ptr [edi]
    test dl, 1
    je .L000ebd1a
    cmp edx, -1
    je .L000ebcf9
    and dl, 0xfe
    add edi, edx
    dec eax
    jne .L000ebce5
.L000ebcf9:
    mov edi, dword ptr [ebx + 0xc]
    cmp esi, edi
    jae .L000ebd03
    mov edi, dword ptr [ebx + 0x28]
.L000ebd03:
    cmp esi, edi
    jb .L000ebd1a
    mov edi, dword ptr [edi + 8]
    cmp esi, edi
    jb .L000ebd1a
    mov edi, dword ptr [edi + 8]
    cmp esi, edi
    jb .L000ebd1a
    mov edi, dword ptr [edi + 8]
    jmp .L000ebd03
.L000ebd1a:
    mov edx, dword ptr [edi + 4]
    mov ecx, dword ptr [esi]
    xchg edx, edi
    mov eax, edi
    add eax, dword ptr [edi]
    cmp eax, esi
    jne .L000ebd39
    add ecx, dword ptr [edi]
    mov dword ptr [edi], ecx
    cmp esi, dword ptr [ebx + 0xc]
    jne .L000ebd35
    mov dword ptr [ebx + 0xc], edi
.L000ebd35:
    mov esi, edi
    jmp .L000ebd4a
.L000ebd39:
    inc dword ptr [ebx + 0x1c]
    mov dword ptr [esi + 8], edx
    mov dword ptr [esi + 4], edi
    mov dword ptr [edi + 8], esi
    mov edi, edx
    mov dword ptr [edi + 4], esi
.L000ebd4a:
    dec dword ptr [ebx + 0x18]
    cmp esi, dword ptr [ebx + 0xc]
    jae .L000ebd5a
    cmp ecx, dword ptr [ebx + 0x10]
    jbe .L000ebd5a
    mov dword ptr [ebx + 0x10], ecx
.L000ebd5a:
    cmp ecx, dword ptr [ebx + 0x14]
    jbe .L000ebd62
    mov dword ptr [ebx + 0x14], ecx
.L000ebd62:
    pop ds
    pop ecx
    pop edi
    pop esi
    ret
.size "__MemFree",.-"__MemFree"
