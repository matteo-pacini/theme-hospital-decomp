.intel_syntax noprefix

// FUNCTION: 0x000ce546

.section _TEXT,"ax",@progbits,unique,845126
.globl "_harderr_"
.type "_harderr_",@function
"_harderr_":
    push edx
    mov dword ptr [_D_001505ca], eax
    mov word ptr [_D_001505ce], dx
    push ds
    mov ax, cs
    mov ds, eax
    .byte 0xba; .long .L000ce582  # mov edx, 0xbe582
    mov al, byte ptr ["__Extender"]
    cmp al, 2
    jb .L000ce577
    cmp al, 7
    ja .L000ce577
    push ecx
    mov cl, 0x24
    mov al, 6
    mov ah, 0x25
    int 0x21
    pop ecx
    jmp .L000ce57d
.L000ce577:
    mov al, 0x24
    mov ah, 0x25
    int 0x21
.L000ce57d:
    sub eax, eax
    pop ds
    pop edx
    ret
.L000ce582:
    .byte 0x06
    .byte 0x1e
    .byte 0x55
    .byte 0x57
    .byte 0x56
    .byte 0x52
    .byte 0x51
    .byte 0x53
    .byte 0x50
    .byte 0x89; .byte 0xc2
    .byte 0xe8; .long "__GETDS"-.-4
    .byte 0x66; .byte 0x8c; .byte 0xd8
    .byte 0x8e; .byte 0xc0
    .byte 0x89; .byte 0x25; .long _D_001505c4
    .byte 0x66; .byte 0x8c; .byte 0x15; .long _D_001505c8
    .byte 0x8b; .byte 0x0d; .long "__STACKLOW"
    .byte 0x89; .byte 0x0d; .long _D_001505d0
    .byte 0x3e; .byte 0x8d; .byte 0x0d; .long _D_0018a784
    .byte 0x89; .byte 0x0d; .long "__STACKLOW"
    .byte 0x8e; .byte 0xd0
    .byte 0x3e; .byte 0x8d; .byte 0x25; .long "_lbVesaBytesPerLine"
    .byte 0x29; .byte 0xdb
    .byte 0x66; .byte 0x89; .byte 0xeb
    .byte 0xc1; .byte 0xe3; .byte 0x04
    .byte 0x81; .byte 0xe6; .byte 0xff; .byte 0xff; .byte 0x00; .byte 0x00
    .byte 0x01; .byte 0xf3
    .byte 0x29; .byte 0xc9
    .byte 0xa0; .long "__Extender"
    .byte 0x3c; .byte 0x02
    .byte 0x72; .byte .L000ce5e9-.-1
    .byte 0x3c; .byte 0x07
    .byte 0x77; .byte .L000ce5e9-.-1
    .byte 0xb1; .byte 0x34
    .byte 0xeb; .byte .L000ce5f4-.-1
.L000ce5e9:
    .byte 0x3c; .byte 0x01
    .byte 0x75; .byte .L000ce5f2-.-1
    .byte 0x66; .byte 0x8c; .byte 0xd9
    .byte 0xeb; .byte .L000ce5f4-.-1
.L000ce5f2:
    .byte 0xb1; .byte 0x34
.L000ce5f4:
    .byte 0x0f; .byte 0xb7; .byte 0xc2
    .byte 0x0f; .byte 0xb7; .byte 0xd7
    .byte 0x51
    .byte 0x53
    .byte 0x52
    .byte 0x50
    .byte 0x29; .byte 0xed
    .byte 0xff; .byte 0x1d; .long _D_001505ca
    .byte 0x0f; .byte 0xb2; .byte 0x25; .long _D_001505c4
    .byte 0x8b; .byte 0x0d; .long _D_001505d0
    .byte 0x89; .byte 0x0d; .long "__STACKLOW"
    .byte 0x5b
    .byte 0x5b
    .byte 0x59
    .byte 0x5a
    .byte 0x5e
    .byte 0x5f
    .byte 0x5d
    .byte 0x1f
    .byte 0x07
    .byte 0xcf
.size "_harderr_",.-"_harderr_"
