.intel_syntax noprefix

// FUNCTION: 0x000ecc8c

.section _TEXT,"ax",@progbits,unique,969868
.globl "fn_000ecc8c_"
.type "fn_000ecc8c_",@function
"fn_000ecc8c_":
    push ebp
    {load} mov ebp, esp
    add esp, -0x10
    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
    mov dword ptr [ebp - 4], eax
    mov dword ptr [ebp - 8], edx
    mov dword ptr [ebp - 0xc], ebx
    mov byte ptr [ebp - 0xd], cl
    mov esi, dword ptr [ebp - 0xc]
    mov edi, dword ptr [ebp - 8]
    imul edi, dword ptr ["_lbDisplay"+0x20]
    add edi, dword ptr [ebp - 4]
    add edi, dword ptr ["_lbDisplay"+0x10]
    {load} xor edx, edx
    mov dl, byte ptr [ebp - 0xd]
    {load} mov dh, dl
.L000eccc1:
    lodsb al, byte ptr [esi]
    cmp al, 0
    je .L000ecd3c
    sub al, 0x20
    push esi
    .byte 0xbe; .long "___EFG_scanf"+0x4  # mov esi, 0x3170c
    movzx eax, al
    {load} add eax, eax
    {load} add esi, eax
    {load} add esi, eax
    {load} add esi, eax
    mov ecx, 6
    movzx ebx, byte ptr [esi]
    inc esi
    jmp dword ptr [ebx + .L000ecce8]
.L000ecce8:
    .long .L000ecd13
    .long .L000ecd08
    .long .L000ecd1f
    .long .L000ecd0d
    .long .L000ecd11
    .long .L000ecd18
    .long .L000ecd1d
    .long .L000ecd23
.L000ecd08:
    mov byte ptr [edi + 1], dl
    jmp .L000ecd23
.L000ecd0d:
    mov byte ptr [edi], dl
    jmp .L000ecd23
.L000ecd11:
    mov byte ptr [edi], dl
.L000ecd13:
    mov byte ptr [edi + 2], dl
    jmp .L000ecd23
.L000ecd18:
    mov word ptr [edi], dx
    jmp .L000ecd23
.L000ecd1d:
    mov byte ptr [edi], dl
.L000ecd1f:
    mov word ptr [edi + 1], dx
.L000ecd23:
    add edi, dword ptr ["_lbDisplay"+0x20]
    .byte 0xe2; .byte 0xb3  # loop 0xeccde
    mov esi, dword ptr ["_lbDisplay"+0x20]
    imul esi, esi, 6
    {load} sub edi, esi
    add edi, 4
    pop esi
    jmp .L000eccc1
.L000ecd3c:
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret
.size "fn_000ecc8c_",.-"fn_000ecc8c_"
