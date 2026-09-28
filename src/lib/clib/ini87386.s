.intel_syntax noprefix

// FUNCTION: 0x000ef0a8

.section _TEXT,"ax",@progbits,unique,979112
.globl "fn_000ef0a8_"
.type "fn_000ef0a8_",@function
"fn_000ef0a8_":
    push es
    push ecx
    push ebx
    push edx
    fninit
    push eax
    fnstcw word ptr [esp]
    pop eax
    cmp ah, 3
    je .L000ef0b9
    inc ebp
.L000ef0b9:
    or ebp, ebp
    je .L000ef0c2
    call "fn_000ef0d8_"
.L000ef0c2:
    wait
    fninit
    fldcw word ptr [_D_00152a7c]
    fldz
    fldz
    fldz
    fldz
    pop edx
    pop ebx
    pop ecx
    pop es
    ret
.size "fn_000ef0a8_",.-"fn_000ef0a8_"

// FUNCTION: 0x000ef0d8

.section _TEXT,"ax",@progbits,unique,979160
.globl "fn_000ef0d8_"
.type "fn_000ef0d8_",@function
"fn_000ef0d8_":
    mov byte ptr [_D_00150814], 3
    mov byte ptr [_D_00151b54], 1
    .byte 0x66; .byte 0x0f; .byte 0x01; .byte 0x25; .long _D_00151b52  # smsw word ptr [0x31b52]
    and word ptr [_D_00151b52], 6
    sub esp, 8
    sidt [esp]
    mov ebx, dword ptr [esp + 2]
    add ebx, 0x38
    add esp, 8
    cmp byte ptr ["__X32VM"], 0
    je .L000ef12a
    call "fn_000ef1a3_"
    .byte 0x66; .byte 0x8e; .byte 0x05; .long _D_0014d5da  # mov es, word ptr [0x2d5da]
    mov dword ptr es:[ebx], edx
    mov dword ptr es:[ebx + 4], ecx
    call "fn_000ef20d_"
    jmp .L000ef1a2
.L000ef12a:
    cmp byte ptr ["__Extender"], 0
    jne .L000ef14e
    mov ax, 0x2507
    push ds
    mov cx, cs
    mov ds, ecx
    lea edx, ["__int7_X32VM"+0x330]
    int 0x21
    pop ds
    mov al, 4
    mov ah, 0xf3
    int 0x21
    jmp .L000ef1a2
.L000ef14e:
    cmp byte ptr ["__Extender"], 9
    jne .L000ef176
    call "fn_000ef1a3_"
    mov dword ptr [ebx], edx
    mov dword ptr [ebx + 4], ecx
    mov eax, 0xe02
    mov ebx, 4
    mov ecx, dword ptr ["__GDAptr"]
    call dword ptr [ecx + 0x30]
    jmp .L000ef1a2
.L000ef176:
    cmp byte ptr ["__Extender"], 1
    jne .L000ef18f
    mov dx, word ptr ["__D16Infoseg"]
    sub eax, eax
    call "__hook387"
    jmp .L000ef1a2
.L000ef18f:
    sub dx, dx
    sub eax, eax
    call "__hook387"
    cmp al, 1
    je .L000ef1a2
    call "fn_000ef1c0_"
.L000ef1a2:
    ret
.size "fn_000ef0d8_",.-"fn_000ef0d8_"

// FUNCTION: 0x000ef1a3

.section _TEXT,"ax",@progbits,unique,979363
.globl "fn_000ef1a3_"
.type "fn_000ef1a3_",@function
"fn_000ef1a3_":
    lea ecx, ["__int7_X32VM"+0x330]
    mov dx, cs
    shl edx, 0x10
    mov dx, cx
    mov cx, cs
    and cl, 3
    shl cx, 0xd
    or ch, 0x8e
    ret
.size "fn_000ef1a3_",.-"fn_000ef1a3_"

// FUNCTION: 0x000ef1c0

.section _TEXT,"ax",@progbits,unique,979392
.globl "fn_000ef1c0_"
.type "fn_000ef1c0_",@function
"fn_000ef1c0_":
    mov cl, 7
    mov ax, 0x2502
    int 0x21
    mov dword ptr [_D_00151b4c], ebx
    .byte 0x66; .byte 0x8c; .byte 0x05; .long _D_00151b50  # mov word ptr [0x31b50], es
    cmp byte ptr ["__Extender"], 3
    mov cl, 7
    mov ax, 0x2504
    push ds
    push cs
    pop ds
    jl .L000ef1f7
    lea edx, ["__int7_X32VM"+0x2c0]
    int 0x21
    pop ds
    call "fn_000ef20d_"
    jmp .L000ef20c
.L000ef1f7:
    lea edx, ["__int7_X32VM"+0x330]
    int 0x21
    pop ds
    mov ecx, cr0
    or ecx, 4
    and ecx, 0xfffffffd
    mov cr0, ecx
.L000ef20c:
    ret
.size "fn_000ef1c0_",.-"fn_000ef1c0_"

// FUNCTION: 0x000ef20d

.section _TEXT,"ax",@progbits,unique,979469
.globl "fn_000ef20d_"
.type "fn_000ef20d_",@function
"fn_000ef20d_":
    xor ebx, ebx
    lea edx, [_D_00151b0c]
    mov ax, 0x2535
    int 0x21
    or dword ptr [_D_00151b0c], 4
    and dword ptr [_D_00151b0c], 0xfffffffd
    inc ebx
    int 0x21
    ret
    .byte 0x80; .byte 0x3d; .long _D_00151b54; .byte 0x01
    .byte 0x74; .byte .L000ef237-.-1
    .byte 0xc3
.L000ef237:
    .byte 0x51
    .byte 0x53
    .byte 0x52
    .byte 0xa0; .long "__Extender"
    .byte 0x80; .byte 0x3d; .long "__X32VM"; .byte 0x00
    .byte 0x74; .byte .L000ef24f-.-1
    .byte 0xe8; .long "fn_000ef2e2_"-.-4
    .byte 0xeb; .byte .L000ef29c-.-1
.L000ef24f:
    .byte 0x3c; .byte 0x09
    .byte 0x75; .byte .L000ef268-.-1
    .byte 0xb8; .byte 0x02; .byte 0x0e; .byte 0x00; .byte 0x00
    .byte 0xbb; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x8b; .byte 0x0d; .long "__GDAptr"
    .byte 0xff; .byte 0x51; .byte 0x30
    .byte 0xeb; .byte .L000ef29c-.-1
.L000ef268:
    .byte 0x3c; .byte 0x00
    .byte 0x75; .byte .L000ef278-.-1
    .byte 0x66; .byte 0xa1; .long _D_00151b52
    .byte 0xb4; .byte 0xf3
    .byte 0xcd; .byte 0x21
    .byte 0xeb; .byte .L000ef29c-.-1
.L000ef278:
    .byte 0x3c; .byte 0x01
    .byte 0x75; .byte .L000ef28c-.-1
    .byte 0x66; .byte 0x8b; .byte 0x15; .long "__D16Infoseg"
    .byte 0x29; .byte 0xc0
    .byte 0xe8; .long "__unhook387"-.-4
    .byte 0xeb; .byte .L000ef29c-.-1
.L000ef28c:
    .byte 0x29; .byte 0xd2
    .byte 0xe8; .long "__unhook387"-.-4
    .byte 0x3c; .byte 0x01
    .byte 0x74; .byte .L000ef29c-.-1
    .byte 0xe8; .long "fn_000ef2a7_"-.-4
.L000ef29c:
    .byte 0xc6; .byte 0x05; .long _D_00151b54; .byte 0x00
    .byte 0x5a
    .byte 0x5b
    .byte 0x59
    .byte 0xc3
.size "fn_000ef20d_",.-"fn_000ef20d_"

// FUNCTION: 0x000ef2a7

.section _TEXT,"ax",@progbits,unique,979623
.globl "fn_000ef2a7_"
.type "fn_000ef2a7_",@function
"fn_000ef2a7_":
    mov cl, 7
    mov ax, 0x2504
    mov edx, dword ptr [_D_00151b4c]
    push ds
    .byte 0x66; .byte 0x8e; .byte 0x1d; .long _D_00151b50  # mov ds, word ptr [0x31b50]
    int 0x21
    pop ds
    cmp byte ptr ["__Extender"], 3
    jl .L000ef2ce
    call "fn_000ef2e2_"
    jmp .L000ef2e1
.L000ef2ce:
    mov ecx, cr0
    and ecx, 0xfffffff9
    mov dx, word ptr [_D_00151b52]
    or cx, dx
    mov cr0, ecx
.L000ef2e1:
    ret
.size "fn_000ef2a7_",.-"fn_000ef2a7_"

// FUNCTION: 0x000ef2e2

.section _TEXT,"ax",@progbits,unique,979682
.globl "fn_000ef2e2_"
.type "fn_000ef2e2_",@function
"fn_000ef2e2_":
    xor ebx, ebx
    lea edx, [_D_00151b0c]
    mov ax, 0x2535
    int 0x21
    xor ecx, ecx
    mov cx, word ptr [_D_00151b52]
    mov dword ptr [_D_00151b0c], ecx
    inc ebx
    int 0x21
    ret
    .byte 0x9b
    .byte 0xdd; .byte 0x30
    .byte 0x9b
    .byte 0xc3
    .byte 0xdd; .byte 0x20
    .byte 0x9b
    .byte 0xc3
.size "fn_000ef2e2_",.-"fn_000ef2e2_"

// FUNCTION: 0x000ef30c

.section _TEXT,"ax",@progbits,unique,979724
.globl "fn_000ef30c_"
.type "fn_000ef30c_",@function
"fn_000ef30c_":
    push ebx
    push edx
    cmp byte ptr [_D_00150815], 0
    je .L000ef32d
    .byte 0xba; .long "fn_000ef2e2_"+0x21  # mov edx, 0xdf303
    .byte 0xbb; .long "fn_000ef2e2_"+0x26  # mov ebx, 0xdf308
    mov dword ptr [_D_00152a80], edx
    mov dword ptr [_D_00152a84], ebx
.L000ef32d:
    xor eax, eax
    mov ax, word ptr [_D_00152a7c]
    call "fn_000fa8f0_"
    pop edx
    pop ebx
    ret
    .byte 0x80; .byte 0x3d; .long _D_00150815; .byte 0x00
    .byte 0x75; .byte 0xc6
    .byte 0xc3
    .byte 0x52
    .byte 0x8a; .byte 0x25; .long _D_00150814
    .byte 0x84; .byte 0xe4
    .byte 0x75; .byte .L000ef38b-.-1
    .byte 0x88; .byte 0x25; .long _D_00150815
    .byte 0x2b; .byte 0xc0
    .byte 0x50
    .byte 0xdb; .byte 0xe3
    .byte 0xd9; .byte 0x3c; .byte 0x24
    .byte 0x58
    .byte 0x8a; .byte 0xc4
    .byte 0x30; .byte 0xd2
    .byte 0x25; .byte 0xff; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x83; .byte 0xf8; .byte 0x03
    .byte 0x75; .byte .L000ef376-.-1
    .byte 0xe8; .long -4; .reloc .-4, R_386_PC32, "fn_000ef30c_"
    .byte 0x88; .byte 0xc2
.L000ef376:
    .byte 0x80; .byte 0x3d; .long "__no87"; .byte 0x00
    .byte 0x75; .byte .L000ef38b-.-1
    .byte 0x88; .byte 0x15; .long _D_00150814
    .byte 0x88; .byte 0x15; .long _D_00150815
.L000ef38b:
    .byte 0x5a
    .byte 0xc3
.size "fn_000ef30c_",.-"fn_000ef30c_"
