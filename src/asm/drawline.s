.intel_syntax noprefix

// FUNCTION: 0x000ccac4

.section _TEXT,"ax",@progbits,unique,838340
.globl "LbDrawLine_"
.type "LbDrawLine_",@function
"LbDrawLine_":
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
    mov dword ptr [ebp - 0x10], ecx
    mov eax, dword ptr ["_lbDisplay"+0x30]
    dec eax
    mov ebx, dword ptr ["_lbDisplay"+0x34]
    dec ebx
    mov edi, dword ptr [ebp - 4]
    mov esi, dword ptr [ebp - 0xc]
    {load} cmp edi, esi
    jg .L000ccb25
    cmp esi, 0
    jge .L000ccb02
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb02:
    {load} cmp edi, eax
    jle .L000ccb10
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb10:
    cmp edi, 0
    jge .L000ccb1c
    mov dword ptr [ebp - 4], 0
.L000ccb1c:
    {load} cmp esi, eax
    jle .L000ccb58
    mov dword ptr [ebp - 0xc], eax
    jmp .L000ccb58
.L000ccb25:
    cmp edi, 0
    jge .L000ccb34
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb34:
    {load} cmp esi, eax
    jle .L000ccb42
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb42:
    cmp esi, 0
    jge .L000ccb4c
    mov esi, 0
.L000ccb4c:
    {load} cmp edi, eax
    jle .L000ccb52
    {load} mov edi, eax
.L000ccb52:
    mov dword ptr [ebp - 4], esi
    mov dword ptr [ebp - 0xc], edi
.L000ccb58:
    mov edi, dword ptr [ebp - 8]
    mov esi, dword ptr [ebp - 0x10]
    {load} cmp edi, esi
    jg .L000ccb94
    cmp esi, 0
    jge .L000ccb71
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb71:
    {load} cmp edi, ebx
    jle .L000ccb7f
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccb7f:
    cmp edi, 0
    jge .L000ccb8b
    mov dword ptr [ebp - 8], 0
.L000ccb8b:
    {load} cmp esi, ebx
    jle .L000ccbc7
    mov dword ptr [ebp - 0x10], ebx
    jmp .L000ccbc7
.L000ccb94:
    cmp edi, 0
    jge .L000ccba3
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccba3:
    {load} cmp esi, ebx
    jle .L000ccbb1
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.L000ccbb1:
    cmp esi, 0
    jge .L000ccbbb
    mov esi, 0
.L000ccbbb:
    {load} cmp edi, ebx
    jle .L000ccbc1
    {load} mov edi, ebx
.L000ccbc1:
    mov dword ptr [ebp - 8], esi
    mov dword ptr [ebp - 0x10], edi
.L000ccbc7:
    mov edi, dword ptr [ebp - 8]
    imul edi, dword ptr ["_lbDisplay"+0x20]
    add edi, dword ptr [ebp - 4]
    add edi, dword ptr ["_lbDisplay"+0x10]
    mov ecx, dword ptr [ebp - 0xc]
    sub ecx, dword ptr [ebp - 4]
    jne .L000ccc4b
    mov ecx, dword ptr [ebp - 0x10]
    sub ecx, dword ptr [ebp - 8]
    inc ecx
    test word ptr ["_lbDisplay"+0x60], 4
    je .L000ccc15
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
    shl eax, 8
    mov al, byte ptr [edi]
    mov bl, byte ptr [eax + esi]
    mov byte ptr [edi], bl
    add edi, dword ptr ["_lbDisplay"+0x20]
    .byte 0xe2; .byte 0xf1  # loop 0xccc01
    jmp .L000ccc9b
.L000ccc15:
    test word ptr ["_lbDisplay"+0x60], 8
    je .L000ccc3b
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
    mov ah, byte ptr [edi]
    mov bl, byte ptr [eax + esi]
    mov byte ptr [edi], bl
    add edi, dword ptr ["_lbDisplay"+0x20]
    .byte 0xe2; .byte 0xf1  # loop 0xccc2a
    jmp .L000ccc9b
.L000ccc3b:
    movzx eax, byte ptr [ebp + 8]
    mov byte ptr [edi], al
    add edi, dword ptr ["_lbDisplay"+0x20]
    .byte 0xe2; .byte 0xf6  # loop 0xccc3f
    jmp .L000ccc9b
.L000ccc4b:
    inc ecx
    test word ptr ["_lbDisplay"+0x60], 4
    je .L000ccc72
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
    shl eax, 8
    mov al, byte ptr [edi]
    mov bl, byte ptr [eax + esi]
    mov byte ptr [edi], bl
    add edi, 1
    .byte 0xe2; .byte 0xf4  # loop 0xccc64
    jmp .L000ccc9b
.L000ccc72:
    test word ptr ["_lbDisplay"+0x60], 8
    je .L000ccc95
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
    mov ah, byte ptr [edi]
    mov bl, byte ptr [eax + esi]
    mov byte ptr [edi], bl
    add edi, 1
    .byte 0xe2; .byte 0xf4  # loop 0xccc87
    jmp .L000ccc9b
.L000ccc95:
    movzx eax, byte ptr [ebp + 8]
    rep stosb byte ptr es:[edi], al
.L000ccc9b:
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
.size "LbDrawLine_",.-"LbDrawLine_"
