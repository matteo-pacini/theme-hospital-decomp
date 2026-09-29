.intel_syntax noprefix

// FUNCTION: 0x000c9b40

.section _TEXT,"ax",@progbits,unique,826176
.globl "LbScreenCopyBox_"
.type "LbScreenCopyBox_",@function
"LbScreenCopyBox_":
    push ebp
    {load} mov ebp, esp
    add esp, -0x10
    push esi
    push edi
    push ecx
    push edx
    push eax
    push ebx
    {load} mov esi, eax
    {load} mov edi, edx
    mov eax, dword ptr ["_lbDisplay"+0x20]
    mov dword ptr [ebp - 0x10], eax
    mov eax, dword ptr ["_lbDisplay"+0x30]
    sub dword ptr [ebp - 0x10], eax
    shr eax, 2
.L000c9b63:
    {load} mov ecx, eax
    rep movsd dword ptr es:[edi], dword ptr [esi]
    add edi, dword ptr [ebp - 0x10]
    dec ebx
    {load} or ebx, ebx
    jne .L000c9b63
    pop ebx
    pop eax
    pop edx
    pop ecx
    pop edi
    pop esi
    leave
    ret
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x83; .byte 0xc4; .byte 0xf0
    .byte 0x56
    .byte 0x57
    .byte 0x51
    .byte 0x52
    .byte 0x50
    .byte 0x53
    .byte 0x89; .byte 0x45; .byte 0xfc
    .byte 0x89; .byte 0x55; .byte 0xf8
    .byte 0x89; .byte 0x5d; .byte 0xf4
    .byte 0x89; .byte 0x4d; .byte 0xf0
    .byte 0x8b; .byte 0x35; .long "_lbDisplay"+0x20
    .byte 0x0f; .byte 0xaf; .byte 0x75; .byte 0xf0
    .byte 0x03; .byte 0x75; .byte 0xf4
    .byte 0x03; .byte 0x75; .byte 0xfc
    .byte 0x8b; .byte 0x3d; .long "_lbDisplay"+0x20
    .byte 0x0f; .byte 0xaf; .byte 0x7d; .byte 0x0c
    .byte 0x03; .byte 0x7d; .byte 0x08
    .byte 0x03; .byte 0x7d; .byte 0xf8
    .byte 0x8b; .byte 0x55; .byte 0x10
    .byte 0xc1; .byte 0xea; .byte 0x02
    .byte 0xa1; .long "_lbDisplay"+0x20
    .byte 0x2b; .byte 0x45; .byte 0x10
    .byte 0x8b; .byte 0x5d; .byte 0x14
.L000c9bc0:
    .byte 0x8b; .byte 0xca
    .byte 0xf3; .byte 0xa5
    .byte 0x03; .byte 0xf0
    .byte 0x03; .byte 0xf8
    .byte 0x4b
    .byte 0x75; .byte .L000c9bc0-.-1
    .byte 0x5b
    .byte 0x58
    .byte 0x5a
    .byte 0x59
    .byte 0x5f
    .byte 0x5e
    .byte 0xc9
    .byte 0xc2; .byte 0x10; .byte 0x00
.size "LbScreenCopyBox_",.-"LbScreenCopyBox_"
