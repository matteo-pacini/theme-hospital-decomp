.intel_syntax noprefix

// FUNCTION: 0x000fd8d8

.section _TEXT,"ax",@progbits,unique,1038552
.globl "fn_000fd8d8_"
.type "fn_000fd8d8_",@function
"fn_000fd8d8_":
    cmp word ptr ["___PMSeg"], 0
    jne .L000fd8ed
    cmp byte ptr [_D_00150814], 0
    je .L000fd8ed
    jmp .L000fd8ee
.L000fd8ed:
    ret
.L000fd8ee:
    push eax
    push ebx
    push ecx
    push edx
    push es
    push ds
    cmp byte ptr ["__Extender"], 0
    jne .L000fd904
    call "__Ergo_hook_init_"
    jmp .L000fd929
.L000fd904:
    cmp byte ptr ["__Extender"], 1
    jne .L000fd914
    call "__DOS4G_hook_init_"
    jmp .L000fd929
.L000fd914:
    cmp byte ptr ["__Extender"], 9
    jne .L000fd924
    call "__Intel_hook_init_"
    jmp .L000fd929
.L000fd924:
    call "__Phar_hook_init_"
.L000fd929:
    pop ds
    call "fn_000fd992_"
    pop es
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
    .byte 0x66; .byte 0x83; .byte 0x3d; .long "___PMSeg"; .byte 0x00
    .byte 0x75; .byte .L000fd940-.-1
    .byte 0xc3
.L000fd940:
    .byte 0x50
    .byte 0x53
    .byte 0x52
    .byte 0x1e
    .byte 0x83; .byte 0xec; .byte 0x04
    .byte 0x9b
    .byte 0xd9; .byte 0x3c; .byte 0x24
    .byte 0x9b
    .byte 0xc6; .byte 0x04; .byte 0x24; .byte 0x7f
    .byte 0xd9; .byte 0x2c; .byte 0x24
    .byte 0x9b
    .byte 0x83; .byte 0xc4; .byte 0x04
    .byte 0x80; .byte 0x3d; .long "__Extender"; .byte 0x00
    .byte 0x75; .byte .L000fd967-.-1
    .byte 0xe8; .long "__Ergo_hook_fini_"-.-4
    .byte 0xeb; .byte .L000fd98c-.-1
.L000fd967:
    .byte 0x80; .byte 0x3d; .long "__Extender"; .byte 0x01
    .byte 0x75; .byte .L000fd977-.-1
    .byte 0xe8; .long "__DOS4G_hook_fini_"-.-4
    .byte 0xeb; .byte .L000fd98c-.-1
.L000fd977:
    .byte 0x80; .byte 0x3d; .long "__Extender"; .byte 0x09
    .byte 0x75; .byte .L000fd987-.-1
    .byte 0xe8; .long "__Intel_hook_fini_"-.-4
    .byte 0xeb; .byte .L000fd98c-.-1
.L000fd987:
    .byte 0xe8; .long "__Phar_hook_fini_"-.-4
.L000fd98c:
    .byte 0x1f
    .byte 0x5a
    .byte 0x5b
    .byte 0x58
    .byte 0xc3
.size "fn_000fd8d8_",.-"fn_000fd8d8_"

// FUNCTION: 0x000fd992

.section _TEXT,"ax",@progbits,unique,1038738
.globl "fn_000fd992_"
.type "fn_000fd992_",@function
"fn_000fd992_":
    sub esp, 4
    wait
    fnstcw word ptr [esp]
    wait
    and word ptr [esp], 0xfff2
    fldcw word ptr [esp]
    wait
    add esp, 4
    ret
    .byte 0x50
    .byte 0xb0; .byte 0x20
    .byte 0xe6; .byte 0x20
    .byte 0xe6; .byte 0xa0
    .byte 0x66; .byte 0x31; .byte 0xc0
    .byte 0xe6; .byte 0xf0
    .byte 0x58
    .byte 0x50
    .byte 0x53
    .byte 0x51
    .byte 0x52
    .byte 0x56
    .byte 0x57
    .byte 0x55
    .byte 0x1e
    .byte 0x06
    .byte 0x83; .byte 0xec; .byte 0x1c
    .byte 0x89; .byte 0xe5
    .byte 0xd9; .byte 0x75; .byte 0x00
    .byte 0x9b
    .byte 0x9b
    .byte 0xdb; .byte 0xe2
    .byte 0x9b
    .byte 0xdb; .byte 0xe1
    .byte 0xfb
    .byte 0xe8; .long "__GETDS"-.-4
    .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0xf7; .byte 0xd2
    .byte 0xb6; .byte 0xff
    .byte 0x23; .byte 0x55; .byte 0x04
    .byte 0x8e; .byte 0x45; .byte 0x10
    .byte 0x8b; .byte 0x7d; .byte 0x0c
.L000fd9e2:
    .byte 0x66; .byte 0x26; .byte 0x8b; .byte 0x1f
    .byte 0x47
    .byte 0x80; .byte 0xfb; .byte 0xd8
    .byte 0x72; .byte .L000fd9e2-.-1
    .byte 0x80; .byte 0xfb; .byte 0xdf
    .byte 0x77; .byte .L000fd9e2-.-1
    .byte 0x8e; .byte 0x45; .byte 0x18
    .byte 0x8b; .byte 0x7d; .byte 0x14
    .byte 0x86; .byte 0xfb
    .byte 0xb1; .byte 0x00
    .byte 0xf6; .byte 0xc2; .byte 0x20
    .byte 0x74; .byte .L000fda04-.-1
    .byte 0xb1; .byte 0x86
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda04:
    .byte 0xf6; .byte 0xc2; .byte 0x40
    .byte 0x74; .byte .L000fda18-.-1
    .byte 0x66; .byte 0xf7; .byte 0xc2; .byte 0x00; .byte 0x02
    .byte 0x75; .byte .L000fda14-.-1
    .byte 0xb1; .byte 0x8b
    .byte 0xeb; .byte .L000fda16-.-1
.L000fda14:
    .byte 0xb1; .byte 0x8a
.L000fda16:
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda18:
    .byte 0xf6; .byte 0xc2; .byte 0x01
    .byte 0x74; .byte .L000fda24-.-1
    .byte 0xe8; .long "fn_000fdaba_"-.-4
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda24:
    .byte 0xf6; .byte 0xc2; .byte 0x02
    .byte 0x74; .byte .L000fda2d-.-1
    .byte 0xb1; .byte 0x82
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda2d:
    .byte 0xf6; .byte 0xc2; .byte 0x08
    .byte 0x74; .byte .L000fda3b-.-1
    .byte 0xe8; .long "fn_000fdb47_"-.-4
    .byte 0xb1; .byte 0x84
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda3b:
    .byte 0xf6; .byte 0xc2; .byte 0x10
    .byte 0x74; .byte .L000fda44-.-1
    .byte 0xb1; .byte 0x85
    .byte 0xeb; .byte .L000fda50-.-1
.L000fda44:
    .byte 0xf6; .byte 0xc2; .byte 0x04
    .byte 0x74; .byte .L000fda50-.-1
    .byte 0xe8; .long "fn_000fdb99_"-.-4
    .byte 0xb1; .byte 0x83
.L000fda50:
    .byte 0x80; .byte 0xf9; .byte 0x00
    .byte 0x74; .byte .L000fdaa0-.-1
    .byte 0xff; .byte 0x35; .long "__STACKLOW"
    .byte 0x66; .byte 0x8c; .byte 0x15; .long _D_00153ae0+0x216
    .byte 0x89; .byte 0x25; .long _D_00153ae0+0x218
    .byte 0x1e
    .byte 0x17
    .byte 0x8d; .byte 0x25; .long _D_00153ae0+0x216
    .byte 0x8d; .byte 0x05; .long _D_00153ae0+0x16
    .byte 0xa3; .long "__STACKLOW"
    .byte 0x0f; .byte 0xb6; .byte 0xc1
    .byte 0xff; .byte 0x15; .long _D_001507c9
    .byte 0x66; .byte 0x8e; .byte 0x15; .long _D_00153ae0+0x216
    .byte 0x8b; .byte 0x25; .long _D_00153ae0+0x218
    .byte 0x8f; .byte 0x05; .long "__STACKLOW"
    .byte 0x66; .byte 0xc7; .byte 0x05; .long _D_00153ae0+0x216; .byte 0x00; .byte 0x00
.L000fdaa0:
    .byte 0x9b
    .byte 0xdb; .byte 0xe2
    .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x00; .byte 0x72; .byte 0xff
    .byte 0xd9; .byte 0x6d; .byte 0x00
    .byte 0x9b
    .byte 0x83; .byte 0xc4; .byte 0x1c
    .byte 0x07
    .byte 0x1f
    .byte 0x5d
    .byte 0x5f
    .byte 0x5e
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0x58
    .byte 0xcf
.size "fn_000fd992_",.-"fn_000fd992_"

// FUNCTION: 0x000fdaba

.section _TEXT,"ax",@progbits,unique,1039034
.globl "fn_000fdaba_"
.type "fn_000fdaba_",@function
"fn_000fdaba_":
    mov cl, 0x81
    cmp bx, 0xd9fa
    jne .L000fdac6
    mov cl, 0x88
    ret
.L000fdac6:
    cmp bx, 0xd9f1
    jne .L000fdad0
    mov cl, 0x8e
    ret
.L000fdad0:
    cmp bx, 0xd9f8
    je .L000fdadc
    cmp bx, 0xd9f5
.L000fdadc:
    jne .L000fdae2
    mov cl, 0x8f
    jmp .L000fdb46
.L000fdae2:
    mov dx, bx
    and dx, 0x310
    cmp dx, 0x310
    jne .L000fdaf5
    mov cl, 0x8d
    jmp .L000fdb46
.L000fdaf5:
    and dx, 0x110
    cmp dx, 0x110
    jne .L000fdb1f
    fstp st(0)
    fldz
    mov dl, bl
    and dl, 0xc0
    cmp dl, 0xc0
    je .L000fdb14
    call "fn_000fdbe4_"
.L000fdb14:
    test bl, 8
    je .L000fdb1b
    fstp st(0)
.L000fdb1b:
    mov cl, 0x85
    jmp .L000fdb46
.L000fdb1f:
    mov dx, bx
    and dx, 0x130
    cmp dx, 0x30
    jne .L000fdb46
    mov dx, word ptr [ebp + 8]
    mov cl, ah
    and cl, 0x38
    shr cl, 2
    ror dx, cl
    and dl, 5
    cmp dl, 5
    jne .L000fdb46
    mov cl, 0x83
.L000fdb46:
    ret
.size "fn_000fdaba_",.-"fn_000fdaba_"

// FUNCTION: 0x000fdb47

.section _TEXT,"ax",@progbits,unique,1039175
.globl "fn_000fdb47_"
.type "fn_000fdb47_",@function
"fn_000fdb47_":
    cmp bx, 0xd9fd
    jne .L000fdb50
    jmp .L000fdb98
.L000fdb50:
    mov dx, bx
    and dx, 0x110
    cmp dx, 0x110
    jne .L000fdb7c
    call "fn_000fdb99_"
    mov dl, bl
    and dl, 0xc0
    cmp dl, 0xc0
    je .L000fdb73
    call "fn_000fdbe4_"
.L000fdb73:
    test bl, 8
    je .L000fdb7a
    fstp st(0)
.L000fdb7a:
    jmp .L000fdb98
.L000fdb7c:
    mov dl, bl
    and dl, 0xc0
    cmp dl, 0xc0
    jne .L000fdb93
    lea esi, [_D_00153ae0]
    call "fn_000fdbbf_"
    jmp .L000fdb98
.L000fdb93:
    call "fn_000fdb99_"
.L000fdb98:
    ret
.size "fn_000fdb47_",.-"fn_000fdb47_"

// FUNCTION: 0x000fdb99

.section _TEXT,"ax",@progbits,unique,1039257
.globl "fn_000fdb99_"
.type "fn_000fdb99_",@function
"fn_000fdb99_":
    ftst
    wait
    fnstsw word ptr [ebp + 0x14]
    fstp st(0)
    test bh, 4
    je .L000fdbae
    fld qword ptr [_D_00153ae0+0xa]
    jmp .L000fdbb4
.L000fdbae:
    fld dword ptr [_D_00153ae0+0x12]
.L000fdbb4:
    test word ptr [ebp + 0x14], 0x100
    je .L000fdbbe
    fchs
.L000fdbbe:
    ret
.size "fn_000fdb99_",.-"fn_000fdb99_"

// FUNCTION: 0x000fdbbf

.section _TEXT,"ax",@progbits,unique,1039295
.globl "fn_000fdbbf_"
.type "fn_000fdbbf_",@function
"fn_000fdbbf_":
    test bh, 4
    jne .L000fdbc8
    mov dl, 0
    jmp .L000fdbcd
.L000fdbc8:
    mov dl, bl
    and dl, 7
.L000fdbcd:
    push edx
.L000fdbce:
    dec dl
    jl .L000fdbd6
    fincstp
    jmp .L000fdbce
.L000fdbd6:
    fstp st(0)
    .byte 0xdb; .byte 0x2e  # fld xword ptr [esi]
    pop edx
.L000fdbdb:
    dec dl
    jl .L000fdbe3
    fdecstp
    jmp .L000fdbdb
.L000fdbe3:
    ret
.size "fn_000fdbbf_",.-"fn_000fdbbf_"

// FUNCTION: 0x000fdbe4

.section _TEXT,"ax",@progbits,unique,1039332
.globl "fn_000fdbe4_"
.type "fn_000fdbe4_",@function
"fn_000fdbe4_":
    test bl, 4
    je .L000fdbee
    fst qword ptr es:[edi]
    jmp .L000fdbf1
.L000fdbee:
    fst dword ptr es:[edi]
.L000fdbf1:
    ret
.size "fn_000fdbe4_",.-"fn_000fdbe4_"

// The 80x87 exception interrupt and the interrupt controllers' layout.
.section _DATA,"aw",@progbits
.globl "___FPE_int"
"___FPE_int":
    .byte 0x02
.globl "___IRQ_num"
"___IRQ_num":
    .byte 0x0d
.globl "___IRQ_int"
"___IRQ_int":
    .byte 0x75
.globl "___MST_pic"
"___MST_pic":
    .byte 0x20
.globl "___SLV_pic"
"___SLV_pic":
    .byte 0xa0

// fpeinth's data, a segment of its own: the constants its exception
// handler loads (extended-precision infinity, DBL_MAX, FLT_MAX), and the
// handler's stack (0x200 bytes, then the saved ss:esp).
.section _DATA,"aw",@progbits,unique,1
.globl _D_00153ae0
_D_00153ae0:
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
    .byte 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    .byte 0xef, 0x7f, 0xff, 0xff, 0x7f, 0x7f, 0x00, 0x00
    .zero 0x206
