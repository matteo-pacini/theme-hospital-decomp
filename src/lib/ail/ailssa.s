.intel_syntax noprefix

// FUNCTION: 0x000f2bd4

.section _TEXT,"ax",@progbits,unique,994260
.globl "_AILSSA_merge"
.type "_AILSSA_merge",@function
"_AILSSA_merge":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    mov ecx, dword ptr [ebp + 8]
    mov edi, dword ptr [ecx + 4]
    mov eax, 0
    cmp dword ptr [edi + 0x1c], 2
    je .L000f2bf3
    cmp dword ptr [edi + 0x1c], 3
    jne .L000f2bf6
.L000f2bf3:
    or eax, 1
.L000f2bf6:
    cmp dword ptr [ecx + 0x44], 2
    je .L000f2c02
    cmp dword ptr [ecx + 0x44], 3
    jne .L000f2c11
.L000f2c02:
    or eax, 2
    test dword ptr [ecx + 0x48], 2
    je .L000f2c11
    or eax, 4
.L000f2c11:
    cmp dword ptr [ecx + 0x44], 1
    je .L000f2c1d
    cmp dword ptr [ecx + 0x44], 3
    jne .L000f2c20
.L000f2c1d:
    or eax, 8
.L000f2c20:
    test eax, 2
    je .L000f2c2e
    test eax, 1
    je .L000f2c40
.L000f2c2e:
    cmp dword ptr [ecx + 0x50], 0x7f
    jne .L000f2c40
    cmp dword ptr [edi + 0x6c], 0x7f
    jne .L000f2c40
    cmp dword ptr [ecx + 0x54], 0x40
    je .L000f2c4a
.L000f2c40:
    or eax, 0x40
    test eax, 8
    je .L000f2c56
.L000f2c4a:
    test dword ptr [ecx + 0x48], 1
    je .L000f2c56
    or eax, 0x10
.L000f2c56:
    mov dword ptr ["_locked"+0x4], eax
    mov eax, 0
    mov edx, dword ptr [ecx + 0x4c]
    cmp edx, 0
    jg .L000f2c6d
    mov edx, 1
.L000f2c6d:
    mov ebx, dword ptr [edi + 0x18]
    shl ebx, 0x10
    div ebx
    {load} mov edx, eax
    sub edx, 0x10000
    jg .L000f2c81
    neg edx
.L000f2c81:
    cmp edx, dword ptr ["_AIL_preference"+0x18]
    jbe .L000f2ccb
    {load} mov ebx, eax
    shr ebx, 0x10
    shl eax, 0x10
    mov dword ptr ["_SmackTimerSetupAddr"+0x10], eax
    {load} mov eax, ebx
    inc ebx
    test dword ptr ["_locked"+0x4], 2
    je .L000f2ca9
    shl eax, 1
    shl ebx, 1
.L000f2ca9:
    test dword ptr ["_locked"+0x4], 8
    je .L000f2cb9
    shl eax, 1
    shl ebx, 1
.L000f2cb9:
    mov dword ptr ["_SmackTimerSetupAddr"+0x14], eax
    mov dword ptr ["_SmackTimerSetupAddr"+0x18], ebx
    or dword ptr ["_locked"+0x4], 0x20
.L000f2ccb:
    mov ebx, dword ptr [edi + 0x50]
    mov eax, dword ptr [edi + 0x54]
    mov dword ptr ["_SmackTimerSetupAddr"+0x8], eax
    {load} add ebx, eax
    mov dword ptr ["_SmackTimerSetupAddr"+0xc], ebx
    {load} mov edi, eax
    add edi, dword ptr [ebp + 0x10]
    mov ebx, dword ptr [ecx + 0x2c]
    mov esi, dword ptr [ecx + ebx*4 + 0xc]
    add esi, dword ptr [ecx + ebx*4 + 0x1c]
    {load} mov eax, esi
    add eax, dword ptr [ebp + 0xc]
    mov dword ptr ["_SmackTimerSetupAddr"+0x4], eax
    lea edx, [ecx + 0x58]
    mov eax, 0
    mov ebx, 0
    mov ecx, 0x80000000
    push ebp
    mov ebp, dword ptr ["_locked"+0x4]
    call dword ptr cs:[ebp*4 + "fn_000f0d80_"+0x228]
    pop ebp
    mov ecx, dword ptr [ebp + 8]
    mov ebx, dword ptr [ecx + 0x2c]
    sub esi, dword ptr [ecx + ebx*4 + 0xc]
    mov dword ptr [ecx + ebx*4 + 0x1c], esi
    {load} mov eax, edi
    sub eax, dword ptr ["_SmackTimerSetupAddr"+0x8]
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AILSSA_merge",.-"_AILSSA_merge"

// FUNCTION: 0x000f2d37

.section _TEXT,"ax",@progbits,unique,994615
.globl "_AILSSA_DMA_copy"
.type "_AILSSA_DMA_copy",@function
"_AILSSA_DMA_copy":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    mov ebx, dword ptr [ebp + 8]
    mov eax, dword ptr [ebx + 0x20]
    and eax, 0xf
    cmp dword ptr [ebx + 0x1c], 2
    je .L000f2d54
    cmp dword ptr [ebx + 0x1c], 3
    jne .L000f2d57
.L000f2d54:
    or eax, 0x10
.L000f2d57:
    cmp dword ptr [ebx + 0x1c], 1
    je .L000f2d63
    cmp dword ptr [ebx + 0x1c], 3
    jne .L000f2d66
.L000f2d63:
    or eax, 0x20
.L000f2d66:
    cmp dword ptr [ebx + 0x68], 1
    jbe .L000f2d6f
    or eax, 0x40
.L000f2d6f:
    mov dword ptr ["_locked"+0x4], eax
    mov esi, dword ptr [ebx + 0x54]
    mov ecx, dword ptr [ebx + 0x4c]
    test dword ptr [ebx + 0x20], 0x20
    jne .L000f2d9d
    mov eax, dword ptr [ebp + 0xc]
    mov edi, dword ptr [ebx + eax*4 + 0x30]
    test dword ptr [ebx + 0x20], 8
    je .L000f2db6
    mov eax, dword ptr [ebx + 0x14]
    shr eax, 1
    lea ebx, [eax + edi]
    jmp .L000f2db6
.L000f2d9d:
    or dword ptr ["_locked"+0x4], 8
    mov eax, dword ptr [ebx + 0x14]
    mov edi, dword ptr [ebx + 0x30]
    lea ebx, [eax + edi]
    shr eax, 1
    mul dword ptr [ebp + 0xc]
    {load} add edi, eax
    {load} add ebx, eax
.L000f2db6:
    mov eax, dword ptr ["_locked"+0x4]
    call dword ptr cs:[eax*4 + "fn_000f0d80_"+0x28]
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AILSSA_DMA_copy",.-"_AILSSA_DMA_copy"

// FUNCTION: 0x000f2dca

.section _TEXT,"ax",@progbits,unique,994762
.globl "_AILSSA_flush_buffer"
.type "_AILSSA_flush_buffer",@function
"_AILSSA_flush_buffer":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push ds
    push es
    cld
    push ds
    pop es
    mov ebx, dword ptr [ebp + 8]
    mov edi, dword ptr [ebx + 0x54]
    mov ecx, dword ptr [ebx + 0x50]
    mov eax, 0
    push ecx
    and ecx, 3
    rep stosb byte ptr es:[edi], al
    pop ecx
    shr ecx, 2
    rep stosd dword ptr es:[edi], eax
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a; .byte 0x4d
    .byte 0x69; .byte 0x6c; .byte 0x65; .byte 0x73; .byte 0x20; .byte 0x53; .byte 0x6f; .byte 0x75
    .byte 0x6e
    .byte 0x64; .byte 0x20; .byte 0x53; .byte 0x79
    .byte 0x73; .byte 0x74
    .byte 0x65; .byte 0x6d
    .byte 0x0d; .byte 0x0a; .byte 0x43; .byte 0x6f; .byte 0x70
    .byte 0x79; .byte 0x72
    .byte 0x69; .byte 0x67; .byte 0x68; .byte 0x74; .byte 0x20; .byte 0x28; .byte 0x43
    .byte 0x29; .byte 0x20
    .byte 0x31; .byte 0x39
    .byte 0x39; .byte 0x31
    .byte 0x2d; .byte 0x39; .byte 0x36; .byte 0x20; .byte 0x52
    .byte 0x41
    .byte 0x44
    .byte 0x20; .byte 0x47; .byte 0x61
    .byte 0x6d
    .byte 0x65; .byte 0x20; .byte 0x54; .byte 0x6f; .byte 0x6f
    .byte 0x6c
    .byte 0x73; .byte 0x2c
    .byte 0x20; .byte 0x49; .byte 0x6e
    .byte 0x63; .byte 0x2e
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a
.size "_AILSSA_flush_buffer",.-"_AILSSA_flush_buffer"

// FUNCTION: 0x000f2e3c

.section _TEXT,"ax",@progbits,unique,994876
.globl "_AILSSA_VMM_lock"
.type "_AILSSA_VMM_lock",@function
"_AILSSA_VMM_lock":
.L000f2e3c:
    push ebx
    push esi
    push edi
    push ds
    push es
    .byte 0x68; .long "___WD_Present"  # push 0x31b88
    .byte 0x68; .long "_locked"+0x4  # push 0x31b68
    call "_AIL_vmm_lock_range"
    add esp, 8
    .byte 0x68; .long .L000f2e3c  # push 0xe2e3c
    .byte 0x68; .long "fn_000f0d80_"+0x28  # push 0xe0da8
    call "_AIL_vmm_lock_range"
    add esp, 8
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    ret
.size "_AILSSA_VMM_lock",.-"_AILSSA_VMM_lock"
