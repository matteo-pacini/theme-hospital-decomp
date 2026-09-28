.intel_syntax noprefix

// FUNCTION: 0x000c9372

.section _TEXT,"ax",@progbits,unique,824178
.globl "memmove_"
.type "memmove_",@function
"memmove_":
    push ecx
    push esi
    push edi
    mov ecx, ebx
    cmp edx, eax
    je .L000c93bb
    jae .L000c93a5
    lea edi, [edx + ebx]
    cmp edi, eax
    jbe .L000c93a5
    lea esi, [edi - 1]
    lea edi, [eax + ebx]
    mov edx, ds
    dec edi
    push es
    mov es, edx
    std
    dec esi
    dec edi
    shr ecx, 1
    rep movsw word ptr es:[edi], word ptr [esi]
    adc ecx, ecx
    inc esi
    inc edi
    .byte 0x66; .byte 0xf3; .byte 0xa4  # rep movsb byte ptr es:[edi], byte ptr [esi]
    pop es
    cld
    pop edi
    pop esi
    pop ecx
    ret
.L000c93a5:
    mov esi, edx
    mov edi, eax
    mov edx, ds
    push es
    mov es, edx
    push ecx
    shr ecx, 2
    rep movsd dword ptr es:[edi], dword ptr [esi]
    pop ecx
    and ecx, 3
    rep movsb byte ptr es:[edi], byte ptr [esi]
    pop es
.L000c93bb:
    pop edi
    pop esi
    pop ecx
    ret
.size "memmove_",.-"memmove_"
