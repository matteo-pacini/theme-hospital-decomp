.intel_syntax noprefix

// FUNCTION: 0x000fce62

.section _TEXT,"ax",@progbits,unique,1035874
.globl "fn_000fce62_"
.type "fn_000fce62_",@function
"fn_000fce62_":
    push ebp
    push esi
    push edi
    push ecx
    push ebx
    push edx
    mov esi, eax
    sub edx, edx
    sub ecx, ecx
    sub eax, eax
.L000fce70:
    mov al, byte ptr [esi]
    cmp al, 0
    je .L000fce94
    mov edi, edx
    mov ebx, ecx
    add ecx, ecx
    adc edx, edx
    add ecx, ecx
    adc edx, edx
    add ecx, ebx
    adc edx, edi
    add ecx, ecx
    adc edx, edx
    and al, 0xf
    add ecx, eax
    adc edx, 0
    inc esi
    jmp .L000fce70
.L000fce94:
    mov eax, ecx
    mov edi, 0x43300000
    call "fn_000fcead_"
    pop ebp
    mov dword ptr [ebp + 4], edx
    mov dword ptr [ebp], eax
    pop ebx
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret
.size "fn_000fce62_",.-"fn_000fce62_"

// FUNCTION: 0x000fcead

.section _TEXT,"ax",@progbits,unique,1035949
.globl "fn_000fcead_"
.type "fn_000fcead_",@function
"fn_000fcead_":
    sub esi, esi
    or esi, eax
    or esi, edx
    je .L000fcefd
    test edx, 0xfff00000
    jne .L000fced1
.L000fcebd:
    sub edi, 0x100000
    add eax, eax
    adc edx, edx
    test edx, 0xfff00000
    je .L000fcebd
    jmp .L000fcef5
.L000fced1:
    test edx, 0xffe00000
    je .L000fcef5
.L000fced9:
    add edi, 0x100000
    shr edx, 1
    rcr eax, 1
    rcr esi, 1
    test edx, 0xffe00000
    jne .L000fced9
    adc esi, esi
    adc eax, 0
    adc edx, 0
.L000fcef5:
    and edx, 0xfffff
    or edx, edi
.L000fcefd:
    ret
    .byte 0x53
    .byte 0x51
    .byte 0x81; .byte 0x3d; .long "___FPE_handler_exit"; .long "fn_000fd8d8_"+0x5d
    .byte 0x74; .byte .L000fcf27-.-1
    .byte 0xbb; .long "fn_000fbb05_"
    .byte 0xb9; .long "fn_000fd8d8_"+0x5d
    .byte 0x89; .byte 0x1d; .long _D_001507c9
    .byte 0xe8; .long "fn_000fd8d8_"-.-4
    .byte 0x89; .byte 0x0d; .long "___FPE_handler_exit"
.L000fcf27:
    .byte 0xe8; .long "fn_000fd992_"-.-4
    .byte 0x59
    .byte 0x5b
    .byte 0xc3
.size "fn_000fcead_",.-"fn_000fcead_"
