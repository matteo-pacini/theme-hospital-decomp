.intel_syntax noprefix

// FUNCTION: 0x000c9abc

.section _TEXT,"ax",@progbits,unique,826044
.globl "LbSqrL_"
.type "LbSqrL_",@function
"LbSqrL_":
    push ebp
    {load} mov ebp, esp
    add esp, -4
    push ebx
    push ecx
    push edx
    push esi
    push edi
    mov dword ptr [ebp - 4], eax
    mov ecx, dword ptr [ebp - 4]
    {load} or ecx, ecx
    je .L000c9af5
    bsr eax, ecx
    movzx ebx, word ptr [eax*2 + .L000c9afe]
.L000c9adc:
    {load} mov eax, ecx
    {load} xor edx, edx
    div ebx
    {load} cmp eax, ebx
    jge .L000c9aec
    {load} add ebx, eax
    shr ebx, 1
    jmp .L000c9adc
.L000c9aec:
    {load} mov eax, ebx
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret
.L000c9af5:
    {load} xor eax, eax
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret
.L000c9afe:
    .byte 0x01; .byte 0x00
    .byte 0x02; .byte 0x00
    .byte 0x02; .byte 0x00
    .byte 0x04; .byte 0x00
    .byte 0x05; .byte 0x00; .byte 0x08; .byte 0x00; .byte 0x0b
    .byte 0x00; .byte 0x10
    .byte 0x00; .byte 0x16
    .byte 0x00; .byte 0x20
    .byte 0x00; .byte 0x2d; .byte 0x00; .byte 0x40; .byte 0x00; .byte 0x5a
    .byte 0x00; .byte 0x80; .byte 0x00; .byte 0xb5; .byte 0x00; .byte 0x00
    .byte 0x01; .byte 0x6a; .byte 0x01
    .byte 0x00; .byte 0x02
    .byte 0xd4; .byte 0x02
    .byte 0x00; .byte 0x04; .byte 0xa8
    .byte 0x05; .byte 0x00; .byte 0x08; .byte 0x50; .byte 0x0b
    .byte 0x00; .byte 0x10
    .byte 0xa0; .byte 0x16; .byte 0x00; .byte 0x20; .byte 0x41
    .byte 0x2d; .byte 0x00; .byte 0x40; .byte 0x82; .byte 0x5a
    .byte 0x00; .byte 0x80; .byte 0x04; .byte 0xb5; .byte 0xff; .byte 0xff
    .byte 0x00; .byte 0x00
.size "LbSqrL_",.-"LbSqrL_"
