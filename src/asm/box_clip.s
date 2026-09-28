.intel_syntax noprefix

// FUNCTION: 0x000e772c

.section _TEXT,"ax",@progbits,unique,948012
.globl "LbDrawBoxClip_"
.type "LbDrawBoxClip_",@function
"LbDrawBoxClip_":
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
    mov edi, dword ptr [ebp - 8]
    cmp edi, dword ptr [_D_00188c48]
    jge .L000e783e
    cmp edi, 0
    jge .L000e775d
    add dword ptr [ebp - 0x10], edi
    {load} xor edi, edi
.L000e775d:
    {load} mov edx, edi
    add edx, dword ptr [ebp - 0x10]
    cmp edx, dword ptr [_D_00188c48]
    jle .L000e7773
    sub edx, dword ptr [_D_00188c48]
    sub dword ptr [ebp - 0x10], edx
.L000e7773:
    cmp dword ptr [ebp - 0x10], 0
    jle .L000e783e
    add edi, dword ptr [_D_00188c40]
    imul edi, dword ptr ["W?research_names$npnpna"]
    mov eax, dword ptr [ebp - 4]
    cmp eax, dword ptr [_D_00188c44]
    jge .L000e783e
    cmp eax, 0
    jge .L000e77a3
    add dword ptr [ebp - 0xc], eax
    {load} xor eax, eax
.L000e77a3:
    {load} mov edx, eax
    add edx, dword ptr [ebp - 0xc]
    cmp edx, dword ptr [_D_00188c44]
    jle .L000e77b9
    sub edx, dword ptr [_D_00188c44]
    sub dword ptr [ebp - 0xc], edx
.L000e77b9:
    cmp dword ptr [ebp - 0xc], 0
    jle .L000e783e
    {load} add edi, eax
    add edi, dword ptr [_D_00188c3c]
    add edi, dword ptr ["_lbDisplay"+0x4]
    mov edx, dword ptr [ebp - 0x10]
    mov ebx, dword ptr ["W?research_names$npnpna"]
    sub ebx, dword ptr [ebp - 0xc]
    test word ptr [_D_00188c74], 4
    je .L000e7804
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
    shl eax, 8
.L000e77f1:
    mov ecx, dword ptr [ebp - 0xc]
.L000e77f4:
    mov al, byte ptr [edi]
    mov al, byte ptr [eax + esi]
    stosb byte ptr es:[edi], al
    dec ecx
    jne .L000e77f4
    {load} add edi, ebx
    dec edx
    jne .L000e77f1
    jmp .L000e783e
.L000e7804:
    test word ptr [_D_00188c74], 8
    je .L000e782f
    mov esi, dword ptr ["_lbDisplay"+0x8]
    movzx eax, byte ptr [ebp + 8]
.L000e7819:
    mov ecx, dword ptr [ebp - 0xc]
.L000e781c:
    mov al, byte ptr [ebp + 8]
    mov ah, byte ptr [edi]
    mov al, byte ptr [eax + esi]
    stosb byte ptr es:[edi], al
    dec ecx
    jne .L000e781c
    {load} add edi, ebx
    dec edx
    jne .L000e7819
    jmp .L000e783e
.L000e782f:
    mov al, byte ptr [ebp + 8]
.L000e7832:
    mov ecx, dword ptr [ebp - 0xc]
.L000e7835:
    stosb byte ptr es:[edi], al
    dec ecx
    jne .L000e7835
    {load} add edi, ebx
    dec edx
    jne .L000e7832
.L000e783e:
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 4
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x83; .byte 0xc4; .byte 0xec
    .byte 0x50
    .byte 0x53
    .byte 0x51
    .byte 0x52
    .byte 0x57
    .byte 0x56
    .byte 0x89; .byte 0x45; .byte 0xfc
    .byte 0x89; .byte 0x55; .byte 0xf8
    .byte 0x89; .byte 0x5d; .byte 0xf4
    .byte 0x89; .byte 0x4d; .byte 0xf0
    .byte 0x8b; .byte 0x7d; .byte 0xf8
    .byte 0x0f; .byte 0xaf; .byte 0x3d; .long "W?research_names$npnpna"
    .byte 0x03; .byte 0x7d; .byte 0xfc
    .byte 0x03; .byte 0x3d; .long "_lbDisplay"+0x10
    .byte 0x8b; .byte 0x55; .byte 0xf0
    .byte 0xa1; .long "W?research_names$npnpna"
    .byte 0x2b; .byte 0x45; .byte 0xf4
    .byte 0x89; .byte 0x45; .byte 0xec
    .byte 0x66; .byte 0xf7; .byte 0x05; .long _D_00188c74; .byte 0x04; .byte 0x00
    .byte 0x74; .byte .L000e78ad-.-1
    .byte 0x8b; .byte 0x35; .long "_lbDisplay"+0x8
    .byte 0x0f; .byte 0xb6; .byte 0x5d; .byte 0x08
    .byte 0xc1; .byte 0xe3; .byte 0x08
.L000e7899:
    .byte 0x8b; .byte 0x4d; .byte 0xf4
    .byte 0x8a; .byte 0x1f
    .byte 0x8a; .byte 0x04; .byte 0x33
    .byte 0xaa
    .byte 0xe2; .byte 0xf8
    .byte 0x03; .byte 0x7d; .byte 0xec
    .byte 0x66; .byte 0x4a
    .byte 0x75; .byte .L000e7899-.-1
    .byte 0xeb; .byte .L000e78e6-.-1
.L000e78ad:
    .byte 0x66; .byte 0xf7; .byte 0x05; .long _D_00188c74; .byte 0x08; .byte 0x00
    .byte 0x74; .byte .L000e78d6-.-1
    .byte 0x8b; .byte 0x35; .long "_lbDisplay"+0x8
    .byte 0x0f; .byte 0xb6; .byte 0x5d; .byte 0x08
.L000e78c2:
    .byte 0x8b; .byte 0x4d; .byte 0xf4
    .byte 0x8a; .byte 0x3f
    .byte 0x8a; .byte 0x04; .byte 0x33
    .byte 0xaa
    .byte 0xe2; .byte 0xf8
    .byte 0x03; .byte 0x7d; .byte 0xec
    .byte 0x66; .byte 0x4a
    .byte 0x75; .byte .L000e78c2-.-1
    .byte 0xeb; .byte .L000e78e6-.-1
.L000e78d6:
    .byte 0x0f; .byte 0xb6; .byte 0x45; .byte 0x08
.L000e78da:
    .byte 0x8b; .byte 0x4d; .byte 0xf4
    .byte 0xf3; .byte 0xaa
    .byte 0x03; .byte 0x7d; .byte 0xec
    .byte 0x66; .byte 0x4a
    .byte 0x75; .byte .L000e78da-.-1
.L000e78e6:
    .byte 0x5e
    .byte 0x5f
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0x58
    .byte 0xc9
    .byte 0xc2; .byte 0x04; .byte 0x00
.size "LbDrawBoxClip_",.-"LbDrawBoxClip_"
