.intel_syntax noprefix

// fn_000f75a0: the emulator's constant tables (status/control/tag masks), not code; the instructions are a disassembly of data.
// FUNCTION: 0x000f75a0

.section _TEXT,"ax",@progbits,unique,1013152
.globl "fn_000f75a0_"
.type "fn_000f75a0_",@function
"fn_000f75a0_":
    add byte ptr [eax], al
    add byte ptr [eax], al
    add byte ptr [eax], al
    or al, byte ptr [eax]
    inc esi
    add byte ptr [eax], al
    add byte ptr [eax], al
    add byte ptr [eax], al
    add byte ptr [eax + eax], dl
    add byte ptr [eax], al
    add byte ptr [eax], cl
    add dword ptr [eax], eax
    add byte ptr [eax], al
    push ds
    add byte ptr [edx], cl
    add byte ptr [eax], al
    adc byte ptr [edx], al
    add byte ptr [eax], al
    add byte ptr [eax], ch
    add byte ptr [eax + eax], dl
    add byte ptr [eax], bl
    add eax, dword ptr [eax]
    add byte ptr [eax], al
    xor al, byte ptr [eax]
    push ds
    add byte ptr [eax], al
    and byte ptr [eax + eax], al
    add byte ptr [eax], al
    cmp al, 0
    sub byte ptr [eax], al
    add byte ptr [eax], ch
    add eax, 0x46000000
    add byte ptr [edx], dh
    add byte ptr [eax], al
    xor byte ptr [esi], al
    add byte ptr [eax], al
    add byte ptr [eax], al
    add byte ptr [eax + eax], bh
    add byte ptr [eax], bh
    pop es
    add byte ptr [eax], al
    add ah, bh
    inc dword ptr [ecx]
    add byte ptr [eax], al
    add byte ptr [edx], al
    add byte ptr [ebx], al
    add bl, dh
    inc dword ptr [eax + eax]
    add byte ptr [eax], al
    or byte ptr [eax], al
    or al, 0
    iretd
    .byte 0xff; .byte 0x10
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x20
    .byte 0x00; .byte 0x30
    .byte 0x00; .byte 0x3f
    .byte 0xff; .byte 0x40; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x80; .byte 0x00; .byte 0xc0
    .byte 0x00; .byte 0xff
    .byte 0xfc
    .byte 0x00; .byte 0x01
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x02
    .byte 0x00; .byte 0x03
    .byte 0xff; .byte 0xf3
    .byte 0x00; .byte 0x04; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x08; .byte 0x00
    .byte 0x0c; .byte 0xff
    .byte 0xcf
    .byte 0x00; .byte 0x10
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x20
    .byte 0x00; .byte 0x30
    .byte 0xff
    .byte 0x3f
    .byte 0x00; .byte 0x40; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x80; .byte 0x00; .byte 0xc0
    .byte 0x01; .byte 0x40; .byte 0x00
    .byte 0x45
    .byte 0x00; .byte 0x02
    .byte 0x40
    .byte 0x42
    .byte 0x01; .byte 0x03
    .byte 0x41
    .byte 0x43
    .byte 0x87; .byte 0xdb
.size "fn_000f75a0_",.-"fn_000f75a0_"

// __int7_X32VM: the emulator's dispatch tables (addressing-mode and per-opcode handler addresses), followed by its INT 7 entry stubs, which save the registers and call fn_000f79a8.
// FUNCTION: 0x000f7654

.section _TEXT,"ax",@progbits,unique,1013332
.globl "__int7_X32VM"
.type "__int7_X32VM",@function
"__int7_X32VM":
    .long "fn_000f7bb8_"+0x4e
    .long "fn_000f7bb8_"+0x54
    .long "fn_000f7bb8_"+0x5a
    .long "fn_000f7bb8_"+0x60
    .long "fn_000f7bb8_"+0x110
    .long "fn_000f7bb8_"+0x6e
    .long "fn_000f7bb8_"+0x74
    .long "fn_000f7bb8_"+0x7a
    .long "fn_000f7bb8_"+0x80
    .long "fn_000f7bb8_"+0x88
    .long "fn_000f7bb8_"+0x90
    .long "fn_000f7bb8_"+0x98
    .long "fn_000f7bb8_"+0x110
    .long "fn_000f7bb8_"+0xaa
    .long "fn_000f7bb8_"+0xb4
    .long "fn_000f7bb8_"+0xbc
    .long "fn_000f7bb8_"+0xc4
    .long "fn_000f7bb8_"+0xcd
    .long "fn_000f7bb8_"+0xd6
    .long "fn_000f7bb8_"+0xdf
    .long "fn_000f7bb8_"+0x110
    .long "fn_000f7bb8_"+0xf3
    .long "fn_000f7bb8_"+0xfe
    .long "fn_000f7bb8_"+0x107
    .long "fn_000f7bb8_"+0x4e
    .long "fn_000f7bb8_"+0x54
    .long "fn_000f7bb8_"+0x5a
    .long "fn_000f7bb8_"+0x60
    .long "fn_000f7bb8_"+0x66
    .long "fn_000f7bb8_"+0x6e
    .long "fn_000f7bb8_"+0x74
    .long "fn_000f7bb8_"+0x7a
    .long "fn_000f7bb8_"+0x80
    .long "fn_000f7bb8_"+0x88
    .long "fn_000f7bb8_"+0x90
    .long "fn_000f7bb8_"+0x98
    .long "fn_000f7bb8_"+0xa0
    .long "fn_000f7bb8_"+0xaa
    .long "fn_000f7bb8_"+0xb4
    .long "fn_000f7bb8_"+0xbc
    .long "fn_000f7bb8_"+0xc4
    .long "fn_000f7bb8_"+0xcd
    .long "fn_000f7bb8_"+0xd6
    .long "fn_000f7bb8_"+0xdf
    .long "fn_000f7bb8_"+0xe8
    .long "fn_000f7bb8_"+0xf3
    .long "fn_000f7bb8_"+0xfe
    .long "fn_000f7bb8_"+0x107
    .long "fn_000f7bb8_"+0x14d
    .long "fn_000f7bb8_"+0x23f
    .long "fn_000f7bb8_"+0x31b
    .long "fn_000f7bb8_"+0x40d
    .long "fn_000f7bb8_"+0x4a7
    .long "fn_000f7bb8_"+0x600
    .long "fn_000f7bb8_"+0x706
    .long "fn_000f7bb8_"+0x808
    .long "fn_000f7bb8_"+0x16c
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x33a
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x4ca
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x727
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x18b
    .long "fn_000f7bb8_"+0x279
    .long "fn_000f7bb8_"+0x359
    .long "fn_000f7bb8_"+0x447
    .long "fn_000f7bb8_"+0x4ed
    .long "fn_000f7bb8_"+0x63e
    .long "fn_000f7bb8_"+0x748
    .long "fn_000f7bb8_"+0x844
    .long "fn_000f7bb8_"+0x1a7
    .long "fn_000f7bb8_"+0x28a
    .long "fn_000f7bb8_"+0x375
    .long "fn_000f7bb8_"+0x483
    .long "fn_000f7bb8_"+0x50d
    .long "fn_000f7bb8_"+0x671
    .long "fn_000f7bb8_"+0x766
    .long "fn_000f7bb8_"+0x88c
    .long "fn_000f7bb8_"+0x1c3
    .long "fn_000f7bb8_"+0x29b
    .long "fn_000f7bb8_"+0x391
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x52d
    .long "fn_000f7bb8_"+0x686
    .long "fn_000f7bb8_"+0x784
    .long "fn_000f7bb8_"+0x8bc
    .long "fn_000f7bb8_"+0x1e2
    .long "fn_000f7bb8_"+0x2b9
    .long "fn_000f7bb8_"+0x3b0
    .long "fn_000f7bb8_"+0x5b9
    .long "fn_000f7bb8_"+0x550
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x7a5
    .long "fn_000f7bb8_"+0x8c1
    .long "fn_000f7bb8_"+0x201
    .long "fn_000f7bb8_"+0x2f9
    .long "fn_000f7bb8_"+0x3cf
    .long "fn_000f7bb8_"+0x148
    .long "fn_000f7bb8_"+0x573
    .long "fn_000f7bb8_"+0x6cd
    .long "fn_000f7bb8_"+0x7c6
    .long "fn_000f7bb8_"+0x8fb
    .long "fn_000f7bb8_"+0x220
    .long "fn_000f7bb8_"+0x30d
    .long "fn_000f7bb8_"+0x3ee
    .long "fn_000f7bb8_"+0x653
    .long "fn_000f7bb8_"+0x596
    .long "fn_000f7bb8_"+0x6f8
    .long "fn_000f7bb8_"+0x7e7
    .long "fn_000f7bb8_"+0x900
    .long "fn_000f7bb8_"+0x956
    .long "fn_000f7bb8_"+0x9f8
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x1009
    .long "fn_000f7bb8_"+0x1087
    .long "fn_000f7bb8_"+0x1122
    .long "fn_000f7bb8_"+0x1087
    .long "fn_000f7bb8_"+0x96b
    .long "fn_000f7bb8_"+0xa40
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x101e
    .long "fn_000f7bb8_"+0xa40
    .long "fn_000f7bb8_"+0x116d
    .long "fn_000f7bb8_"+0xa40
    .long "fn_000f7bb8_"+0x980
    .long "fn_000f7bb8_"+0x929
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x980
    .long "fn_000f7bb8_"+0x10a6
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x10a6
    .long "fn_000f7bb8_"+0x992
    .long "fn_000f7bb8_"+0x10f6
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x992
    .long "fn_000f7bb8_"+0x10f6
    .long "fn_000f7bb8_"+0x11b8
    .long "fn_000f7bb8_"+0x10f6
    .long "fn_000f7bb8_"+0x9a4
    .long "fn_000f7bb8_"+0xa68
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0xfaf
    .long "fn_000f7bb8_"+0x1033
    .long "fn_000f7bb8_"+0x10d2
    .long "fn_000f7bb8_"+0x1241
    .long "fn_000f7bb8_"+0x136d
    .long "fn_000f7bb8_"+0x9b9
    .long "fn_000f7bb8_"+0xae9
    .long "fn_000f7bb8_"+0xf26
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x1048
    .long "fn_000f7bb8_"+0x10e4
    .long "fn_000f7bb8_"+0x128c
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x9ce
    .long "fn_000f7bb8_"+0xd04
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x105d
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x12d7
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x9e3
    .long "fn_000f7bb8_"+0xe0e
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x1072
    .long "fn_000f7bb8_"+0x924
    .long "fn_000f7bb8_"+0x1322
    .long "fn_000f7bb8_"+0x924
    .byte 0xfb
    .byte 0x60
    .byte 0x8b; .byte 0xec
    .byte 0xff; .byte 0x75; .byte 0x40
    .byte 0xff; .byte 0x75; .byte 0x44
    .byte 0x66; .byte 0x8b; .byte 0x4d; .byte 0x3c
    .byte 0x66; .byte 0x89; .byte 0x4d; .byte 0xfa
    .byte 0x8b; .byte 0x4d; .byte 0x38
    .byte 0x89; .byte 0x4d; .byte 0x0c
    .byte 0xe8; .long "fn_000f79a8_"-.-4
    .byte 0x1f
    .byte 0x07
    .byte 0x61
    .byte 0xcf
    .byte 0x87; .byte 0xdb
    .byte 0x90
    .byte 0xfb
    .byte 0x8b; .byte 0x44; .byte 0x24; .byte 0x0c
    .byte 0x36; .byte 0xff; .byte 0x70; .byte 0x14
    .byte 0x36; .byte 0xff; .byte 0x70; .byte 0x10
    .byte 0x50
    .byte 0x50
    .byte 0x50
    .byte 0x36; .byte 0xff; .byte 0x30
    .byte 0x51
    .byte 0x52
    .byte 0x53
    .byte 0x36; .byte 0xff; .byte 0x70; .byte 0x1c
    .byte 0x8b; .byte 0xc5
    .byte 0x50
    .byte 0x56
    .byte 0x57
    .byte 0x06
    .byte 0x1e
    .byte 0x8c; .byte 0x54; .byte 0x24; .byte 0x02
    .byte 0xe8; .long "fn_000f79a8_"-.-4
    .byte 0x1f
    .byte 0x07
    .byte 0x5f
    .byte 0x5e
    .byte 0x58
    .byte 0x8b; .byte 0xe8
    .byte 0x58
    .byte 0x5b
    .byte 0x5a
    .byte 0x59
    .byte 0x58
    .byte 0x87; .byte 0x04; .byte 0x24
    .byte 0x36; .byte 0x8f; .byte 0x00
    .byte 0x83; .byte 0xc4; .byte 0x08
    .byte 0x36; .byte 0x8f; .byte 0x40; .byte 0x10
    .byte 0x83; .byte 0xc4; .byte 0x04
    .byte 0x36; .byte 0x8b; .byte 0x00
    .byte 0xcf
    .byte 0x87; .byte 0xdb
    .byte 0xfb
    .byte 0x83; .byte 0xec; .byte 0x0c
    .byte 0x50
    .byte 0x8b; .byte 0xc4
    .byte 0x83; .byte 0xc0; .byte 0x1c
    .byte 0x51
    .byte 0x52
    .byte 0x53
    .byte 0x50
    .byte 0x55
    .byte 0x56
    .byte 0x57
    .byte 0x06
    .byte 0x1e
    .byte 0x8c; .byte 0x54; .byte 0x24; .byte 0x02
    .byte 0xe8; .long "fn_000f79a8_"-.-4
    .byte 0x1f
    .byte 0x07
    .byte 0x61
    .byte 0x83; .byte 0xc4; .byte 0x0c
    .byte 0xcf
.size "__int7_X32VM",.-"__int7_X32VM"

// fn_000f79a8: emulate the x87 instructions at the trapping CS:EIP (skipping prefixes, decoding ModR/M) one after another, returning at the first non-FPU instruction.
// FUNCTION: 0x000f79a8

.section _TEXT,"ax",@progbits,unique,1014184
.globl "fn_000f79a8_"
.type "fn_000f79a8_",@function
"fn_000f79a8_":
.L000f79a8:
    {load} mov ebp, esp
    mov esi, dword ptr [ebp + 0x38]
    mov ds, word ptr [ebp + 0x3c]
    mov cx, word ptr [ebp + 4]
    mov dx, word ptr [ebp + 6]
    mov al, byte ptr [esi]
    cmp al, 0x9b
    .byte 0x0f; .byte 0x86; .long .L000f7a34-.-4  # jbe 0xf7a34
.L000f79c2:
    {load} mov bl, al
    and bl, 0xf8
    cmp bl, 0xd8
    jne .L000f7adb
    mov ah, byte ptr [esi + 1]
    add esi, 2
    and al, 7
    cmp ah, 0xc0
    jae .L000f7a91
    {load} mov bl, ah
    {load} mov bh, ah
    shr bh, 3
    and ebx, 0x1807
    {load} or bl, bh
    mov bh, 0
    call dword ptr cs:[ebx*4 + "__int7_X32VM"]
    .byte 0x66; .byte 0x8e; .byte 0xc1  # mov es, cx
    mov dword ptr [ebp + 0x38], esi
    {load} mov esi, ebx
    {load} mov bl, ah
    and ebx, 0x38
    {load} or bl, al
    call "__GETDS"
    lea ebp, [_D_001529fc]
    mov edi, dword ptr ds:[ebp + 4]
    and edi, 0x3800
    shr edi, 0xb
    lea edi, [edi + edi*4]
    {load} add edi, edi
    jmp dword ptr cs:[ebx*4 + "__int7_X32VM"+0xc0]
.L000f7a2e:
    mov al, byte ptr [esi]
    cmp al, 0x9b
    ja .L000f79c2
.L000f7a34:
    je .L000f7a5a
    cmp al, 0x26
    je .L000f7a78
    cmp al, 0x2e
    je .L000f7a5d
    cmp al, 0x36
    je .L000f7a6f
    cmp al, 0x3e
    je .L000f7a66
    cmp al, 0x64
    je .L000f7a81
    cmp al, 0x65
    je .L000f7a89
    cmp al, 0x66
    je .L000f7a5a
    cmp al, 0x67
    jne .L000f79c2
.L000f7a5a:
    inc esi
    jmp .L000f7a2e
.L000f7a5d:
    mov cx, word ptr [ebp + 0x3c]
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a66:
    mov cx, word ptr [ebp + 4]
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a6f:
    mov cx, word ptr [ebp + 6]
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a78:
    mov cx, word ptr [ebp + 8]
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a81:
    mov cx, fs
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a89:
    mov cx, gs
    {load} mov edx, ecx
    inc esi
    jmp .L000f7a2e
.L000f7a91:
    mov dword ptr [ebp + 0x38], esi
    call "__GETDS"
    lea ebp, [_D_001529fc]
    {load} mov dl, ah
    and edx, 0x38
    {load} or dl, al
    mov edi, dword ptr ds:[ebp + 4]
    and edi, 0x3800
    shr edi, 0xb
    {load} mov bl, ah
    and ebx, 7
    {load} mov esi, ebx
    cmp ax, 0xf801
    jne .L000f7ac4
    mov si, 1
.L000f7ac4:
    {load} add esi, edi
    and esi, 7
    lea edi, [edi + edi*4]
    {load} add edi, edi
    lea esi, [esi + esi*4]
    {load} add esi, esi
    jmp dword ptr cs:[edx*4 + "__int7_X32VM"+0x1c0]
.L000f7adb:
    ret
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xba; .byte 0x00; .byte 0xff; .byte 0xff; .byte 0xff
    .byte 0x8b; .byte 0x43; .byte 0x04
    .byte 0xc1; .byte 0xe0; .byte 0x19
    .byte 0x73; .byte .L000f7b0a-.-1
    .byte 0x75; .byte .L000f7af6-.-1
    .byte 0x83; .byte 0x3b; .byte 0x00
    .byte 0x75; .byte .L000f7af6-.-1
    .byte 0x03; .byte 0xd2
.L000f7af6:
    .byte 0x81; .byte 0x43; .byte 0x04; .byte 0x00; .byte 0x01; .byte 0x00; .byte 0x00
    .byte 0x73; .byte .L000f7b0a-.-1
    .byte 0xc7; .byte 0x43; .byte 0x04; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x80
    .byte 0x66; .byte 0xff; .byte 0x43; .byte 0x08
.L000f7b0a:
    .byte 0x21; .byte 0x53; .byte 0x04
    .byte 0xc7; .byte 0x03; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0xe9; .long .L000f79a8-.-4
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xba; .byte 0x00; .byte 0xf8; .byte 0xff; .byte 0xff
    .byte 0x8b; .byte 0x03
    .byte 0xc1; .byte 0xe0; .byte 0x16
    .byte 0x73; .byte .L000f7b43-.-1
    .byte 0x75; .byte .L000f7b2c-.-1
    .byte 0x03; .byte 0xd2
.L000f7b2c:
    .byte 0x81; .byte 0x03; .byte 0x00; .byte 0x08; .byte 0x00; .byte 0x00
    .byte 0x83; .byte 0x53; .byte 0x04; .byte 0x00
    .byte 0x73; .byte .L000f7b43-.-1
    .byte 0xc7; .byte 0x43; .byte 0x04; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x80
    .byte 0x66; .byte 0xff; .byte 0x43; .byte 0x08
.L000f7b43:
    .byte 0x21; .byte 0x13
    .byte 0xe9; .long .L000f79a8-.-4
    .byte 0x40
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x04
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0xff; .byte 0xb8
    .byte 0x2e; .byte 0x0a; .byte 0xb0; .long "fn_000f75a0_"+0xa6
    .byte 0x3e; .byte 0x66; .byte 0x89; .byte 0x55; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0xe9; .long .L000f79a8-.-4
    .byte 0x40
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x04
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0xff; .byte 0xb8
    .byte 0x2e; .byte 0x0a; .byte 0xb0; .long "fn_000f75a0_"+0xa6
    .byte 0x3e; .byte 0x66; .byte 0x89; .byte 0x55; .byte 0x04
    .byte 0xe9; .long .L000f79a8-.-4
.size "fn_000f79a8_",.-"fn_000f79a8_"

// fn_000f7bb8: raise FPU exception al: set it in the status word and, if unmasked, call __FPE_handler(ah) with the register image copied to the stack; the addressing-mode and instruction handlers follow it.
// FUNCTION: 0x000f7bb8

.section _TEXT,"ax",@progbits,unique,1014712
.globl "fn_000f7bb8_"
.type "fn_000f7bb8_",@function
"fn_000f7bb8_":
    or byte ptr ds:[ebp + 4], al
    test byte ptr ds:[ebp], al
    jne .L000f7c05
    push esi
    push edi
    push ecx
    push es
    push ds
    sub esp, 0x6c
    mov cx, ss
    .byte 0x66; .byte 0x8e; .byte 0xc1  # mov es, cx
    {load} mov esi, ebp
    {load} mov edi, esp
    mov ecx, 0x1b
    cld
    rep movsd dword ptr es:[edi], dword ptr [esi]
    movzx eax, ah
    call dword ptr ["__FPE_handler"]
    mov cx, ds
    .byte 0x66; .byte 0x8e; .byte 0xc1  # mov es, cx
    mov cx, ss
    .byte 0x66; .byte 0x8e; .byte 0xd9  # mov ds, cx
    {load} mov edi, ebp
    {load} mov esi, esp
    mov ecx, 0x1b
    cld
    rep movsd dword ptr es:[edi], dword ptr [esi]
    add esp, 0x6c
    pop ds
    pop es
    pop ecx
    pop edi
    pop esi
.L000f7c05:
    ret
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x28
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x24
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x20
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x1c
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x18
    .byte 0x8b; .byte 0xca
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x10
    .byte 0xc3
    .byte 0x33; .byte 0xdb
    .byte 0x03; .byte 0x5d; .byte 0x0c
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x28
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x24
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x20
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x1c
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x18
    .byte 0x8b; .byte 0xca
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x14
    .byte 0x8b; .byte 0xca
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x10
    .byte 0xc3
    .byte 0x0f; .byte 0xbe; .byte 0x1e
    .byte 0x46
    .byte 0x03; .byte 0x5d; .byte 0x0c
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x28
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x24
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x20
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x1c
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x18
    .byte 0x8b; .byte 0xca
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x14
    .byte 0x8b; .byte 0xca
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x10
    .byte 0xc3
    .byte 0x8b; .byte 0x1e
    .byte 0x83; .byte 0xc6; .byte 0x04
    .byte 0x03; .byte 0x5d; .byte 0x0c
    .byte 0xc3
    .byte 0x8a; .byte 0x1e
    .byte 0x46
    .byte 0x53
    .byte 0x80; .byte 0xe3; .byte 0x07
    .byte 0x8a; .byte 0xfc
    .byte 0xc0; .byte 0xef; .byte 0x03
    .byte 0x80; .byte 0xe7; .byte 0x18
    .byte 0x0a; .byte 0xdf
    .byte 0xb7; .byte 0x00
    .byte 0x2e; .byte 0xff; .byte 0x14; .byte 0x9d; .long "__int7_X32VM"+0x60
    .byte 0x5a
    .byte 0x51
    .byte 0x8b; .byte 0xca
    .byte 0x80; .byte 0xe2; .byte 0x38
    .byte 0x80; .byte 0xfa; .byte 0x20
    .byte 0x74; .byte .L000f7cfe-.-1
    .byte 0xd1; .byte 0xea
    .byte 0xf7; .byte 0xda
    .byte 0x8b; .byte 0x54; .byte 0x15; .byte 0x28
    .byte 0xc0; .byte 0xe9; .byte 0x06
    .byte 0xd3; .byte 0xe2
    .byte 0x03; .byte 0xda
.L000f7cfe:
    .byte 0x59
    .byte 0xc3
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f98f6_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f97b0_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f97b0_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0xb9; .byte 0x07; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x1e
    .byte 0x06
    .byte 0x66; .byte 0x8c; .byte 0xc0
    .byte 0x66; .byte 0x8c; .byte 0xda
    .byte 0x66; .byte 0x8e; .byte 0xc2
    .byte 0x66; .byte 0x8e; .byte 0xd8
    .byte 0x8b; .byte 0xfd
    .byte 0xf3; .byte 0xa5
    .byte 0x07
    .byte 0x1f
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x3e; .byte 0x66; .byte 0x89; .byte 0x45; .byte 0x00
    .byte 0x66; .byte 0x25; .byte 0x00; .byte 0x03
    .byte 0x66; .byte 0x3d; .byte 0x00; .byte 0x03
    .byte 0x75; .byte .L000f7e90-.-1
    .byte 0x8d; .byte 0x15; .long "fn_000f79a8_"
    .byte 0x3e; .byte 0x89; .byte 0x55; .byte 0x76
    .byte 0xeb; .byte .L000f7eac-.-1
.L000f7e90:
    .byte 0x66; .byte 0x3d; .byte 0x00; .byte 0x02
    .byte 0x75; .byte .L000f7ea2-.-1
    .byte 0x8d; .byte 0x15; .long "fn_000f79a8_"+0x170
    .byte 0x3e; .byte 0x89; .byte 0x55; .byte 0x76
    .byte 0xeb; .byte .L000f7eac-.-1
.L000f7ea2:
    .byte 0x8d; .byte 0x15; .long "fn_000f79a8_"+0x134
    .byte 0x3e; .byte 0x89; .byte 0x55; .byte 0x76
.L000f7eac:
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0xb9; .byte 0x07; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x8b; .byte 0xdf
    .byte 0x8b; .byte 0xfe
    .byte 0x8b; .byte 0xf5
    .byte 0xf3; .byte 0xa5
    .byte 0x8b; .byte 0xfb
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x45; .byte 0x00
    .byte 0x26; .byte 0x66; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xff; .byte 0x33
    .byte 0xff; .byte 0x73; .byte 0x04
    .byte 0xff; .byte 0x73; .byte 0x06
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0x00; .byte 0x0c
    .byte 0xe8; .long "fn_000f99bb_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f967b_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0x8f; .byte 0x43; .byte 0x06
    .byte 0x8f; .byte 0x43; .byte 0x04
    .byte 0x8f; .byte 0x03
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0x00; .byte 0x0c
    .byte 0xe8; .long "fn_000f99bb_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f967b_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5d; .byte 0x6c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x26; .byte 0x8b; .byte 0x46; .byte 0x04
    .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x20
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x46; .byte 0x08
    .byte 0x66; .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x24
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x26; .byte 0x8b; .byte 0x56; .byte 0x04
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9730_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f982f_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0x26; .byte 0x89; .byte 0x56; .byte 0x04
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x20
    .byte 0x26; .byte 0x89; .byte 0x46; .byte 0x04
    .byte 0x66; .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x24
    .byte 0x26; .byte 0x66; .byte 0x89; .byte 0x46; .byte 0x08
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f982f_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0x26; .byte 0x89; .byte 0x56; .byte 0x04
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0xb9; .byte 0x07; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x1e
    .byte 0x06
    .byte 0x66; .byte 0x8c; .byte 0xc0
    .byte 0x66; .byte 0x8c; .byte 0xda
    .byte 0x66; .byte 0x8e; .byte 0xc2
    .byte 0x66; .byte 0x8e; .byte 0xd8
    .byte 0x8b; .byte 0xfd
    .byte 0xf3; .byte 0xa5
    .byte 0x26; .byte 0x8b; .byte 0x5d; .byte 0x04
    .byte 0x81; .byte 0xe3; .byte 0x00; .byte 0x38; .byte 0x00; .byte 0x00
    .byte 0xc1; .byte 0xeb; .byte 0x0b
    .byte 0x8d; .byte 0x1c; .byte 0x9b
    .byte 0x03; .byte 0xdb
    .byte 0xb9; .byte 0x08; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x8d; .byte 0x7c; .byte 0x1d; .byte 0x1c
    .byte 0xa5
    .byte 0xa5
    .byte 0x66; .byte 0xa5
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0x9b; .long "fn_000f75a0_"+0x6
    .byte 0xe2; .byte 0xee
    .byte 0x07
    .byte 0x1f
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0xb9; .byte 0x07; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x8b; .byte 0xdf
    .byte 0x8b; .byte 0xfe
    .byte 0x8b; .byte 0xf5
    .byte 0xf3; .byte 0xa5
    .byte 0xb9; .byte 0x08; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x8d; .byte 0x74; .byte 0x2b; .byte 0x1c
    .byte 0xa5
    .byte 0xa5
    .byte 0x66; .byte 0xa5
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0x9b; .long "fn_000f75a0_"+0x6
    .byte 0xe2; .byte 0xee
    .byte 0x8b; .byte 0xfb
    .byte 0xe9; .long .L000f8b9b-.-4
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x45; .byte 0x04
    .byte 0x26; .byte 0x66; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x55; .byte 0x6c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0x8d; .byte 0x45; .byte 0x6c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x66; .byte 0x8b; .byte 0x06
    .byte 0x98
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f96ea_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xff; .byte 0x33
    .byte 0xff; .byte 0x73; .byte 0x04
    .byte 0xff; .byte 0x73; .byte 0x06
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0x00; .byte 0x0c
    .byte 0xe8; .long "fn_000f99bb_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f967b_"-.-4
    .byte 0x8b; .byte 0xd0
    .byte 0x98
    .byte 0x3b; .byte 0xc2
    .byte 0x74; .byte .L000f842f-.-1
    .byte 0x66; .byte 0xb8; .byte 0x00; .byte 0x80
.L000f842f:
    .byte 0x26; .byte 0x66; .byte 0x89; .byte 0x06
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0x8f; .byte 0x43; .byte 0x06
    .byte 0x8f; .byte 0x43; .byte 0x04
    .byte 0x8f; .byte 0x03
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0x00; .byte 0x0c
    .byte 0xe8; .long "fn_000f99bb_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f967b_"-.-4
    .byte 0x8b; .byte 0xd0
    .byte 0x98
    .byte 0x3b; .byte 0xc2
    .byte 0x74; .byte .L000f846b-.-1
    .byte 0x66; .byte 0xb8; .byte 0x00; .byte 0x80
.L000f846b:
    .byte 0x26; .byte 0x66; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x26; .byte 0x8b; .byte 0x06
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f96fb_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x3e; .byte 0x66; .byte 0x8b; .byte 0x55; .byte 0x00
    .byte 0x66; .byte 0x81; .byte 0xe2; .byte 0x00; .byte 0x0c
    .byte 0xe8; .long "fn_000f99bb_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9680_"-.-4
    .byte 0x26; .byte 0x89; .byte 0x06
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
.L000f84dc:
    jmp "fn_000f79a8_"
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f84e9
.L000f84e9:
    .long .L000f8509
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
.L000f8509:
    jmp "fn_000f79a8_"
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x8
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x8b; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8b; .byte 0x44; .byte 0x2e; .byte 0x20
    .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x20
    .byte 0x66; .byte 0x8b; .byte 0x44; .byte 0x2e; .byte 0x24
    .byte 0x66; .byte 0x89; .byte 0x44; .byte 0x2f; .byte 0x24
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x5c; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x74; .byte 0x2e; .byte 0x1c
    .byte 0x8b; .byte 0x06
    .byte 0x87; .byte 0x03
    .byte 0x89; .byte 0x06
    .byte 0x8b; .byte 0x46; .byte 0x04
    .byte 0x87; .byte 0x43; .byte 0x04
    .byte 0x89; .byte 0x46; .byte 0x04
    .byte 0x66; .byte 0x8b; .byte 0x46; .byte 0x08
    .byte 0x66; .byte 0x87; .byte 0x43; .byte 0x08
    .byte 0x66; .byte 0x89; .byte 0x46; .byte 0x08
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f8628
.L000f8628:
    .long .L000f8648
    .long .L000f8655
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f8662
    .long .L000f8693
    .long .L000f84dc
    .long .L000f84dc
.L000f8648:
    lea eax, [edi + ebp + 0x1c]
    xor byte ptr [eax + 9], 0x80
    jmp "fn_000f79a8_"
.L000f8655:
    lea eax, [edi + ebp + 0x1c]
    and byte ptr [eax + 9], 0x7f
    jmp "fn_000f79a8_"
.L000f8662:
    mov eax, dword ptr cs:[_C_000f759c]
    mov dword ptr ds:[ebp + 0x6c], eax
    mov eax, dword ptr cs:["fn_000f75a0_"]
    mov dword ptr ds:[ebp + 0x70], eax
    mov ax, word ptr cs:["fn_000f75a0_"+0x4]
    mov word ptr ds:[ebp + 0x74], ax
    lea eax, [edi + ebp + 0x1c]
    lea edx, [ebp + 0x6c]
    call "fn_000f9292_"
    jmp "fn_000f79a8_"+0x1f4
.L000f8693:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000f9e42_"
    jmp "fn_000f79a8_"
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f86a9
.L000f86a9:
    .long .L000f86c9
    .long .L000f870c
    .long .L000f8751
    .long .L000f8796
    .long .L000f87db
    .long .L000f8820
    .long .L000f8865
    .long .L000f84dc
.L000f86c9:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    {load} sub eax, eax
    mov dword ptr [esi], eax
    mov dword ptr [esi + 4], 0x80000000
    mov word ptr [esi + 8], 0x3fff
    jmp "fn_000f79a8_"
.L000f870c:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    mov dword ptr [esi], 0xcd1b8afe
    mov dword ptr [esi + 4], 0xd49a784b
    mov word ptr [esi + 8], 0x4000
    jmp "fn_000f79a8_"
.L000f8751:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    mov dword ptr [esi], 0x5c17f0bc
    mov dword ptr [esi + 4], 0xb8aa3b29
    mov word ptr [esi + 8], 0x3fff
    jmp "fn_000f79a8_"
.L000f8796:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    mov dword ptr [esi], 0x2168c235
    mov dword ptr [esi + 4], 0xc900daa2
    mov word ptr [esi + 8], 0x4000
    jmp "fn_000f79a8_"
.L000f87db:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    mov dword ptr [esi], 0xfbcff799
    mov dword ptr [esi + 4], 0x9a209a84
    mov word ptr [esi + 8], 0x3ffd
    jmp "fn_000f79a8_"
.L000f8820:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    mov dword ptr [esi], 0xd1cf79ac
    mov dword ptr [esi + 4], 0xb17217f7
    mov word ptr [esi + 8], 0x3ffe
    jmp "fn_000f79a8_"
.L000f8865:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    {load} sub eax, eax
    mov dword ptr [esi], eax
    mov dword ptr [esi + 4], eax
    mov word ptr [esi + 8], ax
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x58]
    or word ptr ds:[ebp + 8], bx
    jmp "fn_000f79a8_"
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f88c4
.L000f88c4:
    .long .L000f88e4
    .long .L000f88f1
    .long .L000f8938
    .long .L000f8f4d
    .long .L000f8f96
    .long .L000f8ffb
    .long .L000f8984
    .long .L000f89a5
.L000f88e4:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa468_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f88f1:
    lea eax, [edi + ebp + 0x1c]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    or word ptr ds:[ebp + 8], bx
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    lea edx, [edi + ebp + 0x1c]
    call "fn_000f9f49_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8938:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa432_"
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea esi, [edi + ebp + 0x1c]
    {load} sub eax, eax
    mov dword ptr [esi], eax
    mov dword ptr [esi + 4], 0x80000000
    mov word ptr [esi + 8], 0x3fff
    jmp "fn_000f79a8_"
.L000f8984:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    jmp "fn_000f79a8_"
.L000f89a5:
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    jmp "fn_000f79a8_"
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f89ce
.L000f89ce:
    .long .L000f89ee
    .long .L000f8a18
    .long .L000f8a5f
    .long .L000f8a6c
    .long .L000f9025
    .long .L000f903d
    .long .L000f8ac4
    .long .L000f8ad1
.L000f89ee:
    lea eax, [edi + ebp + 0x1c]
    lea edx, [esi + ebp + 0x1c]
    call "fn_000f9d61_"
    and eax, 7
    mov dx, word ptr ds:[ebp + 4]
    and dx, 0xb8ff
    or dh, byte ptr cs:[eax + "fn_000f75a0_"+0xaa]
    mov word ptr ds:[ebp + 4], dx
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8a18:
    lea eax, [edi + ebp + 0x1c]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    or word ptr ds:[ebp + 8], bx
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    lea edx, [edi + ebp + 0x1c]
    call "fn_000f9f30_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8a5f:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa63c_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8a6c:
    lea ebx, [edi + ebp + 0x1c]
    push dword ptr [ebx]
    push dword ptr [ebx + 4]
    push dword ptr [ebx + 6]
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa41b_"
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea ebx, [edi + ebp + 0x1c]
    pop dword ptr [ebx + 6]
    pop dword ptr [ebx + 4]
    pop dword ptr [ebx]
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa425_"
    jmp "fn_000f79a8_"
.L000f8ac4:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa41b_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8ad1:
    lea eax, [edi + ebp + 0x1c]
    call "fn_000fa425_"
    notrack jmp dword ptr ds:[ebp + 0x76]
    jmp dword ptr cs:[ebx*4 + .L000f8ae6]
.L000f8ae6:
    .long .L000f84dc
    .long .L000f8b06
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
.L000f8b06:
    mov eax, dword ptr [edi + ebp + 0x1c]
    mov dword ptr ds:[ebp + 0x6c], eax
    mov eax, dword ptr [edi + ebp + 0x20]
    mov dword ptr ds:[ebp + 0x70], eax
    mov ax, word ptr [edi + ebp + 0x24]
    mov word ptr ds:[ebp + 0x74], ax
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    or word ptr ds:[ebp + 8], bx
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    lea eax, [ebp + 0x6c]
    lea edx, [edi + ebp + 0x1c]
    call "fn_000f9292_"
    jmp "fn_000f79a8_"+0x1a2
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f8b6f
.L000f8b6f:
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f8b8f
    .long .L000f8b9b
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
.L000f8b8f:
    and word ptr ds:[ebp + 4], 0x7f00
    jmp "fn_000f79a8_"
.L000f8b9b:
    mov word ptr ds:[ebp], 0x33f
    mov word ptr ds:[ebp + 4], 0
    mov word ptr ds:[ebp + 8], 0xffff
    lea edx, ["fn_000f79a8_"]
    mov dword ptr ds:[ebp + 0x76], edx
    {load} sub edi, edi
    jmp "fn_000f79a8_"
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9e; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9e; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x20
    .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x20
    .byte 0x66; .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x24
    .byte 0x66; .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x24
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9e; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0xe9; .long "fn_000f79a8_"-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1f4-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9292_"-.-4
    .byte 0xe9; .long "fn_000f79a8_"+0x1a2-.-4
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x20
    .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x20
    .byte 0x66; .byte 0x8b; .byte 0x44; .byte 0x2f; .byte 0x24
    .byte 0x66; .byte 0x89; .byte 0x44; .byte 0x2e; .byte 0x24
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9e; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0xe9; .long "fn_000f79a8_"+0x1b9-.-4
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90cc_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f9500_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f8d78
.L000f8d78:
    .long .L000f84dc
    .long .L000f8d98
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
.L000f8d98:
    mov eax, dword ptr [edi + ebp + 0x1c]
    mov dword ptr ds:[ebp + 0x6c], eax
    mov eax, dword ptr [edi + ebp + 0x20]
    mov dword ptr ds:[ebp + 0x70], eax
    mov ax, word ptr [edi + ebp + 0x24]
    mov word ptr ds:[ebp + 0x74], ax
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    or word ptr ds:[ebp + 8], bx
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    lea eax, [ebp + 0x6c]
    lea edx, [edi + ebp + 0x1c]
    call "fn_000f9292_"
    jmp "fn_000f79a8_"+0x1a2
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f90be_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x8d; .byte 0x44; .byte 0x2e; .byte 0x1c
    .byte 0x8d; .byte 0x54; .byte 0x2f; .byte 0x1c
    .byte 0x8d; .byte 0x5c; .byte 0x2e; .byte 0x1c
    .byte 0xe8; .long "fn_000f932b_"-.-4
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x56
    .byte 0x3e; .byte 0x66; .byte 0x21; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0x5e
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x08
    .byte 0x2e; .byte 0x0f; .byte 0xb7; .byte 0xbf; .long "fn_000f75a0_"+0x6
    .byte 0x2e; .byte 0x66; .byte 0x8b; .byte 0x9f; .long "fn_000f75a0_"+0xa
    .byte 0x3e; .byte 0x66; .byte 0x81; .byte 0x65; .byte 0x04; .byte 0xff; .byte 0xc7
    .byte 0x3e; .byte 0x66; .byte 0x09; .byte 0x5d; .byte 0x04
    .byte 0x3e; .byte 0xff; .byte 0x65; .byte 0x76
    .byte 0x2e; .byte 0xff; .byte 0x24; .byte 0x9d; .long .L000f8f2d
.L000f8f2d:
    .long .L000f90ae
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
    .long .L000f84dc
.L000f8f4d:
    lea edx, [edi + ebp + 0x1c]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    or word ptr ds:[ebp + 8], bx
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    lea eax, [edi + ebp + 0x1c]
    {load} mov ebx, eax
    call "fn_000f9b48_"
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f8f96:
    lea esi, [edi + ebp + 0x1c]
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x8]
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0xa]
    and word ptr ds:[ebp + 4], 0xc7ff
    or word ptr ds:[ebp + 4], bx
    mov bx, word ptr cs:[edi + "fn_000f75a0_"+0x56]
    and word ptr ds:[ebp + 8], bx
    lea ebx, [edi + ebp + 0x1c]
    mov eax, dword ptr [esi]
    mov dword ptr [ebx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [ebx + 4], eax
    mov ax, word ptr [esi + 8]
    {load} mov dx, ax
    and dx, 0x8000
    or dx, 0x3fff
    mov word ptr [ebx + 8], dx
    and ax, 0x7fff
    sub ax, 0x3fff
    cwde
    {load} mov edx, esi
    call "fn_000f967b_"
    jmp "fn_000f79a8_"
.L000f8ffb:
    lea eax, [edi + ebp + 0x1c]
    lea edx, [esi + ebp + 0x1c]
    call "fn_000f9d61_"
    and eax, 7
    mov dx, word ptr ds:[ebp + 4]
    and dx, 0xb8ff
    or dh, byte ptr cs:[eax + "fn_000f75a0_"+0xaa]
    mov word ptr ds:[ebp + 4], dx
    notrack jmp dword ptr ds:[ebp + 0x76]
.L000f9025:
    lea eax, [edi + ebp + 0x1c]
    mov dx, word ptr ds:[ebp]
    and dx, 0xc00
    call "fn_000f99bb_"
    jmp "fn_000f79a8_"
.L000f903d:
    {load} mov esi, edi
    movzx edi, word ptr cs:[edi + "fn_000f75a0_"+0x6]
    lea eax, [edi + ebp + 0x1c]
    mov cx, word ptr [eax + 8]
    call "fn_000f967b_"
    {load} mov edi, esi
    lea ebx, [edi + ebp + 0x1c]
    {load} mov edx, eax
    cwde
    {load} cmp eax, edx
    je .L000f907c
    {load} or ch, ch
    jns .L000f906d
    {load} sub dx, dx
    {load} sub cx, cx
    jmp .L000f907a
.L000f906d:
    mov dx, 0x7fff
    mov cx, word ptr [ebx + 8]
    and cx, 0x8000
.L000f907a:
    jmp .L000f90a2
.L000f907c:
    mov dx, word ptr [ebx + 8]
    {load} mov cx, dx
    and cx, 0x8000
    and dx, 0x7fff
    {load} add dx, ax
    jns .L000f90a2
    {load} or ah, ah
    jns .L000f909e
    {load} sub dx, dx
    {load} sub cx, cx
    jmp .L000f90a2
.L000f909e:
    mov dx, 0x7fff
.L000f90a2:
    {load} or dx, cx
    mov word ptr [ebx + 8], dx
    jmp "fn_000f79a8_"
.L000f90ae:
    mov ax, word ptr ds:[ebp + 4]
    {load} mov ebp, esp
    mov word ptr [ebp + 0x28], ax
    jmp "fn_000f79a8_"
.size "fn_000f7bb8_",.-"fn_000f7bb8_"

// fn_000f90be: long double subtract, *ebx = *eax - *edx (negates the second operand's sign, then shares fn_000f90cc).
// FUNCTION: 0x000f90be

.section _TEXT,"ax",@progbits,unique,1020094
.globl "fn_000f90be_"
.type "fn_000f90be_",@function
"fn_000f90be_":
    push esi
    push ecx
    push ebx
    mov si, word ptr [edx + 8]
    xor si, 0x8000
    .byte 0xeb; .byte 0x07  # jmp 0xf90d3
.size "fn_000f90be_",.-"fn_000f90be_"

// fn_000f90cc: long double add, *ebx = *eax + *edx.
// FUNCTION: 0x000f90cc

.section _TEXT,"ax",@progbits,unique,1020108
.globl "fn_000f90cc_"
.type "fn_000f90cc_",@function
"fn_000f90cc_":
    push esi
    push ecx
    push ebx
    mov si, word ptr [edx + 8]
    mov ecx, dword ptr [edx + 4]
    mov ebx, dword ptr [edx]
    shl esi, 0x10
    mov si, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f9126_"
    pop ebx
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    mov word ptr [ebx + 8], si
    pop ecx
    pop esi
    ret
.size "fn_000f90cc_",.-"fn_000f90cc_"

// fn_000f90f6: long double add, *ebx = *eax + the long double passed on the stack.
// FUNCTION: 0x000f90f6

.section _TEXT,"ax",@progbits,unique,1020150
.globl "fn_000f90f6_"
.type "fn_000f90f6_",@function
"fn_000f90f6_":
    push esi
    push ecx
    push ebx
    mov si, word ptr [esp + 0x18]
    mov ecx, dword ptr [esp + 0x14]
    mov ebx, dword ptr [esp + 0x10]
    shl esi, 0x10
    mov si, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f9126_"
    pop ebx
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    mov word ptr [ebx + 8], si
    pop ecx
    pop esi
    ret 0xc
.size "fn_000f90f6_",.-"fn_000f90f6_"

// fn_000f9126: long double add core: edx:eax (exponent si) + ecx:ebx (exponent in esi's high word), result in edx:eax:si.
// FUNCTION: 0x000f9126

.section _TEXT,"ax",@progbits,unique,1020198
.globl "fn_000f9126_"
.type "fn_000f9126_",@function
"fn_000f9126_":
    {load} or eax, eax
    jne .L000f9149
    {load} or edx, edx
    jne .L000f9149
    {load} add si, si
    jne .L000f9146
    shr esi, 0x10
    {load} mov eax, ebx
    {load} mov edx, ecx
    {load} add esi, esi
    {load} or bx, si
    {load} or ebx, edx
    je .L000f9145
    shr esi, 1
.L000f9145:
    ret
.L000f9146:
    rcr si, 1
.L000f9149:
    {load} or ecx, ecx
    jne .L000f915a
    {load} or ebx, ebx
    jne .L000f915a
    test esi, 0x7fff0000
    jne .L000f915a
    ret
.L000f915a:
    push ebp
    push edi
    xchg esi, ecx
    {load} mov edi, ecx
    rol edi, 0x10
    sar edi, 0x10
    sar ecx, 0x10
    and edi, 0x80007fff
    and ecx, 0x80007fff
    {load} mov ebp, ecx
    rol edi, 0x10
    rol ecx, 0x10
    {load} add cx, di
    rol edi, 0x10
    rol ecx, 0x10
    {load} sub cx, di
    je .L000f91a9
    jae .L000f9195
    {load} mov ebp, edi
    neg cx
    xchg ebx, eax
    xchg esi, edx
.L000f9195:
    cmp cx, 0x40
    jbe .L000f91a9
    {load} add ebp, ebp
    rcr bp, 1
    {load} mov eax, ebx
    {load} mov edx, esi
    {load} mov esi, ebp
    pop edi
    pop ebp
    ret
.L000f91a9:
    mov ch, 0
    {load} or ecx, ecx
    jns .L000f91be
    mov ch, 0xff
    neg esi
    neg ebx
    sbb esi, 0
    xor ebp, 0x80000000
.L000f91be:
    {load} sub edi, edi
    cmp cl, 0
    je .L000f91f1
    push ebx
    {load} sub ebx, ebx
    cmp cl, 0x20
    jb .L000f91e3
    {load} or eax, eax
    setne bl
    {load} mov edi, ebx
    {load} sub ebx, ebx
    cmp cl, 0x40
    jne .L000f91df
    {load} or edi, edx
    {load} sub edx, edx
.L000f91df:
    {load} mov eax, edx
    {load} sub edx, edx
.L000f91e3:
    shrd ebx, eax, cl
    {load} or edi, ebx
    {load} sub ebx, ebx
    shrd eax, edx, cl
    shrd edx, ebx, cl
    pop ebx
.L000f91f1:
    {load} add eax, ebx
    {load} adc edx, esi
    adc ch, 0
    jns .L000f9222
    cmp cl, 0x40
    jne .L000f9213
    test edi, 0x7fffffff
    setne bl
    shr ebx, 1
    adc eax, 0
    adc edx, 0
    adc ch, 0
.L000f9213:
    neg edx
    neg eax
    sbb edx, 0
    mov ch, 0
    xor ebp, 0x80000000
.L000f9222:
    {load} mov ebx, eax
    {load} or bl, ch
    {load} or ebx, edx
    je .L000f9279
    {load} or bp, bp
    je .L000f927b
    cmp ch, 0
    jne .L000f9242
    rol edi, 1
    ror edi, 1
.L000f9238:
    dec bp
    je .L000f927b
    {load} adc eax, eax
    {load} adc edx, edx
    jae .L000f9238
.L000f9242:
    inc bp
    cmp bp, 0x7fff
    .byte 0x0f; .byte 0x84; .long .L000f9285-.-4  # je 0xf9285
    stc
    rcr edx, 1
    rcr eax, 1
    jae .L000f9277
    {load} add edi, edi
    jne .L000f925e
    ror eax, 1
    rol eax, 1
.L000f925e:
    adc eax, 0
    adc edx, 0
    jae .L000f9277
    rcr edx, 1
    rcr eax, 1
    inc bp
    cmp bp, 0x7fff
    .byte 0x0f; .byte 0x84; .long .L000f9285-.-4  # je 0xf9285
.L000f9277:
    jmp .L000f927b
.L000f9279:
    {load} mov ebp, ebx
.L000f927b:
    {load} add ebp, ebp
    rcr bp, 1
    {load} mov esi, ebp
    pop edi
    pop ebp
    ret
.L000f9285:
    mov bp, 0x7fff
    {load} sub eax, eax
    mov edx, 0x80000000
    jmp .L000f927b
.size "fn_000f9126_",.-"fn_000f9126_"

// fn_000f9292: compare the long doubles at eax and edx; returns 0 if equal, -1 or 1 for less or greater, 2 if unordered (NaN).
// FUNCTION: 0x000f9292

.section _TEXT,"ax",@progbits,unique,1020562
.globl "fn_000f9292_"
.type "fn_000f9292_",@function
"fn_000f9292_":
    push edi
    push esi
    mov si, word ptr [eax + 8]
    or si, 0x8000
    inc si
    jne .L000f92aa
    mov esi, dword ptr [eax + 4]
    {load} add esi, esi
    or esi, dword ptr [eax]
    jne .L000f92fd
.L000f92aa:
    mov si, word ptr [edx + 8]
    or si, 0x8000
    inc si
    jne .L000f92c0
    mov esi, dword ptr [edx + 4]
    {load} add esi, esi
    or esi, dword ptr [edx]
    jne .L000f92fd
.L000f92c0:
    mov esi, dword ptr [eax + 6]
    mov edi, dword ptr [edx + 6]
    {load} xor edi, esi
    mov edi, 0
    js .L000f9305
    mov si, word ptr [eax + 8]
    cmp si, word ptr [edx + 8]
    jne .L000f92e5
    mov edi, dword ptr [eax + 4]
    cmp edi, dword ptr [edx + 4]
    jne .L000f92e5
    mov edi, dword ptr [eax]
    cmp edi, dword ptr [edx]
.L000f92e5:
    mov edi, 0
    je .L000f92f8
    rcr eax, 1
    {load} xor esi, eax
.L000f92f0:
    {load} add esi, esi
    sbb edi, 0
    {load} add edi, edi
    inc edi
.L000f92f8:
    {load} mov eax, edi
    pop esi
    pop edi
    ret
.L000f92fd:
    mov eax, 2
    pop esi
    pop edi
    ret
.L000f9305:
    or edi, dword ptr [eax]
    or edi, dword ptr [edx]
    or edi, dword ptr [eax + 4]
    or edi, dword ptr [edx + 4]
    mov ax, word ptr [eax + 8]
    shl eax, 0x10
    mov ax, word ptr [edx + 8]
    and eax, 0x7fff7fff
    {load} or eax, edi
    mov edi, 0
    jne .L000f92f0
    pop esi
    pop edi
    ret
.size "fn_000f9292_",.-"fn_000f9292_"

// fn_000f932b: long double divide, *ebx = *eax / *edx.
// FUNCTION: 0x000f932b

.section _TEXT,"ax",@progbits,unique,1020715
.globl "fn_000f932b_"
.type "fn_000f932b_",@function
"fn_000f932b_":
    push esi
    push ecx
    push ebx
    mov si, word ptr [edx + 8]
    mov ecx, dword ptr [edx + 4]
    mov ebx, dword ptr [edx]
    shl esi, 0x10
    mov si, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f9355_"
    pop ebx
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    mov word ptr [ebx + 8], si
    pop ecx
    pop esi
    ret
.size "fn_000f932b_",.-"fn_000f932b_"

// fn_000f9355: long double divide core on registers (as fn_000f9126); x/0 raises zero-divide (infinity), 0/0 invalid (indefinite).
// FUNCTION: 0x000f9355

.section _TEXT,"ax",@progbits,unique,1020757
.globl "fn_000f9355_"
.type "fn_000f9355_",@function
"fn_000f9355_":
    {load} or ecx, ecx
    jne .L000f93a0
    {load} or ebx, ebx
    jne .L000f93a0
    test esi, 0x7fff0000
    jne .L000f93a0
    {load} or eax, eax
    jne .L000f938a
    {load} or edx, edx
    jne .L000f938a
    {load} mov eax, esi
    {load} add ax, ax
    jne .L000f938a
    mov al, 1
    mov ah, 0x81
    call "fn_000f7bb8_"
    mov edx, 0xc0000000
    {load} sub eax, eax
    mov si, 0xffff
    jmp .L000f939f
.L000f938a:
    mov al, 4
    mov ah, 0x83
    call "fn_000f7bb8_"
    mov edx, 0x80000000
    {load} sub eax, eax
    or si, 0x7fff
.L000f939f:
    ret
.L000f93a0:
    {load} or eax, eax
    jne .L000f93b1
    {load} or edx, edx
    jne .L000f93b1
    {load} add si, si
    jne .L000f93ae
    ret
.L000f93ae:
    rcr si, 1
.L000f93b1:
    push ebp
    {load} mov ebp, esp
    push edi
    xchg esi, ecx
    {load} mov edi, ecx
    rol edi, 0x10
    sar edi, 0x10
    sar ecx, 0x10
    and edi, 0x80007fff
    and ecx, 0x80007fff
    rol edi, 0x10
    rol ecx, 0x10
    {load} add di, cx
    rol edi, 0x10
    rol ecx, 0x10
    {load} or di, di
    jne .L000f93ec
.L000f93e2:
    {load} add eax, eax
    {load} adc edx, edx
    dec di
    {load} or edx, edx
    jns .L000f93e2
.L000f93ec:
    {load} or cx, cx
    jne .L000f93fb
.L000f93f1:
    {load} add ebx, ebx
    {load} adc esi, esi
    dec cx
    {load} or esi, esi
    jns .L000f93f1
.L000f93fb:
    {load} sub di, cx
    add di, 0x3fff
    js .L000f941e
    cmp di, 0x7fff
    jb .L000f941e
    {load} mov edi, ecx
    mov di, 0x7fff
    mov edx, 0x80000000
    {load} sub eax, eax
    jmp .L000f94f6
.L000f941e:
    cmp di, -0x40
    jge .L000f942f
    {load} sub eax, eax
    {load} sub edx, edx
    {load} sub edi, edi
    jmp .L000f94f6
.L000f942f:
    push edi
    push esi
    push ebx
    {load} mov ecx, esi
    {load} mov edi, edx
    {load} mov esi, eax
    {load} sub eax, eax
    {load} cmp ecx, edi
    ja .L000f9441
    {load} sub edx, ecx
    inc eax
.L000f9441:
    push eax
    {load} mov eax, esi
    div ecx
    push eax
    xchg ebx, eax
    mul ebx
    xchg ecx, eax
    xchg edx, ebx
    mul edx
    {load} add eax, ebx
    adc edx, 0
    mov ebx, dword ptr [ebp - 0x10]
    test byte ptr [ebp - 0x14], 1
    je .L000f9462
    {load} add eax, ebx
    adc edx, dword ptr [ebp - 0xc]
.L000f9462:
    neg ecx
    {load} sbb esi, eax
    {load} sbb edi, edx
    je .L000f947c
.L000f946a:
    sub dword ptr [ebp - 0x18], 1
    sbb dword ptr [ebp - 0x14], 0
    {load} add ecx, ebx
    adc esi, dword ptr [ebp - 0xc]
    adc edi, 0
    jne .L000f946a
.L000f947c:
    {load} mov edi, esi
    {load} mov esi, ecx
    mov ecx, dword ptr [ebp - 0xc]
    {load} cmp ecx, edi
    ja .L000f9491
    {load} sub edi, ecx
    add dword ptr [ebp - 0x18], 1
    adc dword ptr [ebp - 0x14], 0
.L000f9491:
    {load} mov edx, edi
    {load} mov eax, esi
    div ecx
    push eax
    {load} or eax, eax
    je .L000f94c8
    xchg ebx, eax
    mul ebx
    xchg ecx, eax
    xchg edx, ebx
    mul edx
    {load} add eax, ebx
    adc edx, 0
    neg ecx
    {load} sbb esi, eax
    {load} sbb edi, edx
    je .L000f94c8
.L000f94b1:
    sub dword ptr [ebp - 0x1c], 1
    sbb dword ptr [ebp - 0x18], 0
    sbb dword ptr [ebp - 0x14], 0
    add ecx, dword ptr [ebp - 0x10]
    adc esi, dword ptr [ebp - 0xc]
    adc edi, 0
    jne .L000f94b1
.L000f94c8:
    pop eax
    pop edx
    pop ebx
    add esp, 8
    pop edi
    dec di
    shr ebx, 1
    jae .L000f94da
    rcr edx, 1
    rcr eax, 1
    inc edi
.L000f94da:
    {load} or di, di
    jg .L000f94f6
    jne .L000f94e5
    mov cl, 1
    jmp .L000f94eb
.L000f94e5:
    neg di
    {load} mov cx, di
.L000f94eb:
    {load} sub ebx, ebx
    shrd eax, edx, cl
    shrd edx, ebx, cl
    {load} sub di, di
.L000f94f6:
    {load} add edi, edi
    rcr di, 1
    {load} mov esi, edi
    pop edi
    pop ebp
    ret
.size "fn_000f9355_",.-"fn_000f9355_"

// fn_000f9500: long double multiply, *ebx = *eax * *edx.
// FUNCTION: 0x000f9500

.section _TEXT,"ax",@progbits,unique,1021184
.globl "fn_000f9500_"
.type "fn_000f9500_",@function
"fn_000f9500_":
    push esi
    push ecx
    push ebx
    mov si, word ptr [edx + 8]
    mov ecx, dword ptr [edx + 4]
    mov ebx, dword ptr [edx]
    shl esi, 0x10
    mov si, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f952a_"
    pop ebx
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    mov word ptr [ebx + 8], si
    pop ecx
    pop esi
    ret
.size "fn_000f9500_",.-"fn_000f9500_"

// fn_000f952a: long double multiply core on registers (as fn_000f9126).
// FUNCTION: 0x000f952a

.section _TEXT,"ax",@progbits,unique,1021226
.globl "fn_000f952a_"
.type "fn_000f952a_",@function
"fn_000f952a_":
    {load} or eax, eax
    jne .L000f953b
    {load} or edx, edx
    jne .L000f953b
    {load} add si, si
    jne .L000f9538
    ret
.L000f9538:
    rcr si, 1
.L000f953b:
    {load} or ecx, ecx
    jne .L000f9552
    {load} or ebx, ebx
    jne .L000f9552
    test esi, 0x7fff0000
    jne .L000f9552
    {load} sub eax, eax
    {load} sub edx, edx
    {load} sub esi, esi
    ret
.L000f9552:
    push ebp
    push edi
    xchg esi, ecx
    {load} mov edi, ecx
    rol edi, 0x10
    sar edi, 0x10
    sar ecx, 0x10
    and edi, 0x80007fff
    and ecx, 0x80007fff
    {load} add ecx, edi
    sub cx, 0x3ffe
    jb .L000f9582
    cmp cx, 0x7fff
    jb .L000f9582
    jmp .L000f9619
.L000f9582:
    cmp cx, -0x40
    jge .L000f9593
    {load} sub eax, eax
    {load} sub edx, edx
    {load} sub ecx, ecx
    jmp .L000f960f
    .byte 0x90
    .byte 0x90
    .byte 0x90
.L000f9593:
    push ecx
    {load} sub ebp, ebp
    push esi
    push edx
    push eax
    mul ebx
    xchg esi, eax
    {load} mov ecx, edx
    pop edx
    mul edx
    {load} mov edi, edx
    {load} add ecx, eax
    {load} adc edi, ebp
    {load} adc ebp, ebp
    pop eax
    xchg ebx, eax
    mul ebx
    {load} add ecx, eax
    {load} adc edi, edx
    adc ebp, 0
    {load} mov eax, ebx
    pop edx
    mul edx
    {load} add eax, edi
    {load} adc edx, ebp
    {load} mov ebx, ecx
    pop ecx
    js .L000f95ca
    {load} add ebx, ebx
    {load} adc eax, eax
    {load} adc edx, edx
    dec cx
.L000f95ca:
    {load} add ebx, ebx
    jae .L000f95f6
    jne .L000f95dd
    {load} or esi, esi
    setne bl
    shr ebx, 1
    jb .L000f95dd
    {load} mov esi, eax
    shr esi, 1
.L000f95dd:
    adc eax, 0
    adc edx, 0
    jae .L000f95f6
    rcr edx, 1
    rcr eax, 1
    inc cx
    cmp cx, 0x7fff
    .byte 0x0f; .byte 0x84; .long .L000f9619-.-4  # je 0xf9619
.L000f95f6:
    {load} or cx, cx
    jg .L000f960f
    jne .L000f9601
    mov cl, 1
    jmp .L000f9604
.L000f9601:
    neg cx
.L000f9604:
    {load} sub ebx, ebx
    shrd eax, edx, cl
    shrd edx, ebx, cl
    {load} sub cx, cx
.L000f960f:
    {load} add ecx, ecx
    rcr cx, 1
    {load} mov esi, ecx
    pop edi
    pop ebp
    ret
.L000f9619:
    mov cx, 0x7fff
    mov edx, 0x80000000
    {load} sub eax, eax
    jmp .L000f960f
    .byte 0x53
    .byte 0xb3; .byte 0xa0
    .byte 0xeb; .byte 0x58
    .byte 0x53
    .byte 0x8a; .byte 0x78; .byte 0x09
    .byte 0x51
    .byte 0x66; .byte 0x8b; .byte 0x48; .byte 0x08
    .byte 0x66; .byte 0x81; .byte 0xe1; .byte 0xff; .byte 0x7f
    .byte 0x75; .byte .L000f963f-.-1
    .byte 0x2b; .byte 0xc0
    .byte 0xeb; .byte .L000f9678-.-1
.L000f963f:
    .byte 0x66; .byte 0x81; .byte 0xe9; .byte 0xfe; .byte 0x3f
    .byte 0x7f; .byte .L000f964c-.-1
    .byte 0x02; .byte 0xff
    .byte 0x1b; .byte 0xc0
    .byte 0xeb; .byte .L000f9678-.-1
.L000f964c:
    .byte 0x66; .byte 0x83; .byte 0xf9; .byte 0x1f
    .byte 0x7e; .byte .L000f9659-.-1
    .byte 0xb8; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x80
    .byte 0xeb; .byte .L000f9678-.-1
.L000f9659:
    .byte 0x8a; .byte 0xef
    .byte 0x8b; .byte 0x18
    .byte 0x8b; .byte 0x40; .byte 0x04
    .byte 0x52
    .byte 0x2b; .byte 0xd2
    .byte 0x0f; .byte 0xa5; .byte 0xc2
    .byte 0xd3; .byte 0xe0
    .byte 0x0b; .byte 0xd8
    .byte 0x8b; .byte 0xc2
    .byte 0x5a
    .byte 0x02; .byte 0xed
    .byte 0x73; .byte .L000f9678-.-1
    .byte 0x0b; .byte 0xdb
    .byte 0x74; .byte .L000f9676-.-1
    .byte 0x40
.L000f9676:
    .byte 0xf7; .byte 0xd8
.L000f9678:
    .byte 0x59
    .byte 0x5b
    .byte 0xc3
.size "fn_000f952a_",.-"fn_000f952a_"

// fn_000f967b: convert the long double at eax to an integer of at most 31 magnitude bits in eax (0x80000000 if too large); shares fn_000f9680.
// FUNCTION: 0x000f967b

.section _TEXT,"ax",@progbits,unique,1021563
.globl "fn_000f967b_"
.type "fn_000f967b_",@function
"fn_000f967b_":
    push ebx
    mov bl, 0x1f
    .byte 0xeb; .byte 0x03  # jmp 0xf9683
.size "fn_000f967b_",.-"fn_000f967b_"

// fn_000f9680: convert the long double at eax to an integer of at most 32 magnitude bits in eax (0x80000000 if too large).
// FUNCTION: 0x000f9680

.section _TEXT,"ax",@progbits,unique,1021568
.globl "fn_000f9680_"
.type "fn_000f9680_",@function
"fn_000f9680_":
    push ebx
    mov bl, 0x20
    mov bh, byte ptr [eax + 9]
    shr bh, 1
    {load} or bh, bl
    push ecx
    mov cx, word ptr [eax + 8]
    and cx, 0x7fff
    sub cx, 0x3ffe
    jl .L000f96e5
    cmp cx, 0x20
    jg .L000f96dd
    and bl, 0x3f
    {load} cmp cl, bl
    jg .L000f96dd
    {load} mov ch, bh
    mov ebx, dword ptr [eax]
    mov eax, dword ptr [eax + 4]
    {load} add ebx, ebx
    rcr ch, 1
    cmp cl, 0x20
    jne .L000f96bc
    {load} add ch, ch
    jmp .L000f96c9
.L000f96bc:
    {load} sub ebx, ebx
    shld ebx, eax, cl
    shl eax, cl
    {load} add ch, ch
    {load} add eax, eax
    {load} mov eax, ebx
.L000f96c9:
    mov cl, 0xff
    rcr cl, 1
    {load} and ch, cl
    {load} add ch, ch
    adc eax, 0
    {load} add ch, ch
    jae .L000f96da
    neg eax
.L000f96da:
    pop ecx
    pop ebx
    ret
.L000f96dd:
    mov eax, 0x80000000
    pop ecx
    pop ebx
    ret
.L000f96e5:
    {load} sub eax, eax
    pop ecx
    pop ebx
    ret
.size "fn_000f9680_",.-"fn_000f9680_"

// fn_000f96ea: convert signed int eax to the long double at edx.
// FUNCTION: 0x000f96ea

.section _TEXT,"ax",@progbits,unique,1021674
.globl "fn_000f96ea_"
.type "fn_000f96ea_",@function
"fn_000f96ea_":
    push ebx
    {load} mov ebx, edx
    {load} or eax, eax
    jns .L000f96fa
    neg eax
    mov edx, 0xbfff
    .byte 0xeb; .byte 0x09  # jmp 0xf9703
.L000f96fa:
    pop ebx
.size "fn_000f96ea_",.-"fn_000f96ea_"

// fn_000f96fb: convert unsigned int eax to the long double at edx.
// FUNCTION: 0x000f96fb

.section _TEXT,"ax",@progbits,unique,1021691
.globl "fn_000f96fb_"
.type "fn_000f96fb_",@function
"fn_000f96fb_":
    push ebx
    {load} mov ebx, edx
    mov edx, 0x3fff
    push ecx
    {load} or eax, eax
    je .L000f971e
    bsr ecx, eax
    {load} mov ch, cl
    mov cl, 0x1f
    {load} sub cl, ch
    shl eax, cl
    {load} mov cl, ch
    movzx ecx, ch
    {load} add ecx, edx
    {load} mov edx, eax
    jmp .L000f9722
.L000f971e:
    {load} sub edx, edx
    {load} sub ecx, ecx
.L000f9722:
    {load} sub eax, eax
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    mov word ptr [ebx + 8], cx
    pop ecx
    pop ebx
    ret
.size "fn_000f96fb_",.-"fn_000f96fb_"

// fn_000f9730: convert the double in edx:eax to the long double at ebx (a NaN raises invalid and is made quiet).
// FUNCTION: 0x000f9730

.section _TEXT,"ax",@progbits,unique,1021744
.globl "fn_000f9730_"
.type "fn_000f9730_",@function
"fn_000f9730_":
    push ecx
    {load} mov ecx, edx
    shld edx, eax, 0xb
    shl eax, 0xb
    sar ecx, 0x14
    and cx, 0x7ff
    je .L000f977b
    cmp cx, 0x7ff
    je .L000f9752
    add cx, 0x3c00
    jmp .L000f9773
.L000f9752:
    mov cx, 0x7fff
    test edx, 0x7fffffff
    jne .L000f9760
    {load} or eax, eax
.L000f9760:
    je .L000f9773
    push eax
    mov al, 1
    mov ah, 0x81
    call "fn_000f7bb8_"
    pop eax
    or edx, 0x40000000
.L000f9773:
    or edx, 0x80000000
    jmp .L000f97a0
.L000f977b:
    {load} or edx, edx
    jne .L000f9787
    {load} or eax, eax
    jne .L000f9787
    {load} sub ecx, ecx
    jmp .L000f97a0
.L000f9787:
    mov cx, 0x3c01
    {load} or edx, edx
    jne .L000f9794
    xchg edx, eax
    sub cx, 0x20
.L000f9794:
    {load} or edx, edx
    js .L000f97a0
    {load} add eax, eax
    {load} adc edx, edx
    dec cx
    jmp .L000f9794
.L000f97a0:
    mov dword ptr [ebx], eax
    mov dword ptr [ebx + 4], edx
    {load} add ecx, ecx
    rcr cx, 1
    mov word ptr [ebx + 8], cx
    pop ecx
    ret
.size "fn_000f9730_",.-"fn_000f9730_"

// fn_000f97b0: convert the long double at eax to a float in eax, rounded to nearest even.
// FUNCTION: 0x000f97b0

.section _TEXT,"ax",@progbits,unique,1021872
.globl "fn_000f97b0_"
.type "fn_000f97b0_",@function
"fn_000f97b0_":
    push edx
    push ecx
    push ebx
    {load} mov ebx, eax
    mov edx, 0xffffff00
    mov eax, dword ptr [ebx + 4]
    {load} mov ecx, eax
    shl ecx, 0x19
    mov cx, word ptr [ebx + 8]
    jae .L000f97df
    jne .L000f97d1
    cmp dword ptr [ebx], 0
    jne .L000f97d1
    {load} add edx, edx
.L000f97d1:
    add eax, 0x100
    jae .L000f97df
    mov eax, 0x80000000
    inc cx
.L000f97df:
    {load} and eax, edx
    {load} mov ebx, ecx
    and cx, 0x7fff
    je .L000f982b
    cmp cx, 0x7fff
    jne .L000f9802
    {load} add eax, eax
    shr eax, 8
    or eax, 0xff000000
    {load} add bx, bx
    rcr eax, 1
    jmp .L000f982b
.L000f9802:
    add cx, 0xc080
    jns .L000f980d
    {load} sub eax, eax
    jmp .L000f982b
.L000f980d:
    cmp cx, 0xff
    jl .L000f9820
    mov eax, 0xff000000
    {load} add bx, bx
    rcr eax, 1
    jmp .L000f982b
.L000f9820:
    {load} add eax, eax
    shrd eax, ecx, 8
    {load} add bx, bx
    rcr eax, 1
.L000f982b:
    pop ebx
    pop ecx
    pop edx
    ret
.size "fn_000f97b0_",.-"fn_000f97b0_"

// fn_000f982f: convert the long double at eax to a double in edx:eax, rounded to nearest even.
// FUNCTION: 0x000f982f

.section _TEXT,"ax",@progbits,unique,1021999
.globl "fn_000f982f_"
.type "fn_000f982f_",@function
"fn_000f982f_":
    push ecx
    push ebx
    push esi
    mov cx, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    mov esi, 0xfffff800
    {load} mov ebx, eax
    shl ebx, 0x16
    jae .L000f985c
    jne .L000f984b
    {load} add esi, esi
.L000f984b:
    add eax, 0x800
    adc edx, 0
    jae .L000f985c
    mov edx, 0x80000000
    inc cx
.L000f985c:
    {load} and eax, esi
    {load} mov ebx, ecx
    and cx, 0x7fff
    add cx, 0xc400
    cmp cx, 0x7ff
    jae .L000f9892
    {load} or cx, cx
    jne .L000f9881
    shrd eax, edx, 0xc
    {load} add edx, edx
    shr edx, 0xc
    jmp .L000f988b
.L000f9881:
    shrd eax, edx, 0xb
    {load} add edx, edx
    shrd edx, ecx, 0xb
.L000f988b:
    {load} add bx, bx
    rcr edx, 1
    jmp .L000f98f2
.L000f9892:
    cmp cx, 0xc400
    jb .L000f98cc
    cmp cx, -0x34
    jl .L000f98c6
    sub cx, 0xc
    neg cx
    cmp cl, 0x20
    jb .L000f98b4
    sub cl, 0x20
    {load} mov esi, eax
    {load} mov eax, edx
    {load} sub edx, edx
.L000f98b4:
    shrd esi, eax, cl
    shrd eax, edx, cl
    shr edx, cl
    {load} add esi, esi
    adc eax, 0
    adc edx, 0
    jmp .L000f98ca
.L000f98c6:
    {load} sub eax, eax
    {load} sub edx, edx
.L000f98ca:
    jmp .L000f98f2
.L000f98cc:
    shrd eax, edx, 0xb
    {load} add edx, edx
    shr edx, 0xb
    {load} add bx, bx
    rcr edx, 1
    or edx, 0x7ff00000
    cmp cx, 0x43ff
    je .L000f98f2
    push eax
    mov al, 8
    mov ah, 0x84
    call "fn_000f7bb8_"
    pop eax
.L000f98f2:
    pop esi
    pop ebx
    pop ecx
    ret
.size "fn_000f982f_",.-"fn_000f982f_"

// fn_000f98f6: convert the float in eax to the long double at edx; a table of mantissa masks follows.
// FUNCTION: 0x000f98f6

.section _TEXT,"ax",@progbits,unique,1022198
.globl "fn_000f98f6_"
.type "fn_000f98f6_",@function
"fn_000f98f6_":
    push ecx
    push ebx
    {load} mov ebx, edx
    {load} sub edx, edx
    {load} mov ecx, eax
    sar ecx, 0x17
    and cx, 0xff
    je .L000f992f
    shl eax, 8
    cmp cl, 0xff
    je .L000f9917
    add cx, 0x3f80
    jmp .L000f9925
.L000f9917:
    mov ch, 0xff
    and eax, 0x7fffffff
    je .L000f9925
    or eax, 0x40000000
.L000f9925:
    {load} add ecx, ecx
    rcr cx, 1
    or eax, 0x80000000
.L000f992f:
    mov dword ptr [ebx], edx
    mov dword ptr [ebx + 4], eax
    mov word ptr [ebx + 8], cx
    pop ebx
    pop ecx
    ret
    .byte 0xff
    .byte 0xff
    .byte 0xff
    .byte 0x7f; .byte 0xff
    .byte 0xff
    .byte 0xff
    .byte 0x3f
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x1f
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x0f
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x07
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x03
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x01
    .byte 0xff
    .byte 0xff
    .byte 0xff; .byte 0x00
    .byte 0xff
    .byte 0xff
    .byte 0x7f; .byte .L000f995f-.-1
.L000f995f:
    .byte 0xff
    .byte 0xff
    .byte 0x3f
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x1f
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x0f
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x07
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x03
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x01
    .byte 0x00; .byte 0xff
    .byte 0xff; .byte 0x00
    .byte 0x00; .byte 0xff
    .byte 0x7f; .byte .L000f997e-.-1
.L000f997e:
    .byte 0x00; .byte 0xff
    .byte 0x3f
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x1f
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x0f
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x07
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x03
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x01
    .byte 0x00; .byte 0x00
    .byte 0xff; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x7f; .byte .L000f999d-.-1
.L000f999d:
    .byte 0x00; .byte 0x00
    .byte 0x3f
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x1f
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x0f
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x07
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x03
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x01
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00
.size "fn_000f98f6_",.-"fn_000f98f6_"

// fn_000f99bb: round the long double at eax to an integer in place, using rounding control dx (0 nearest, 400h down, 800h up, C00h chop).
// FUNCTION: 0x000f99bb

.section _TEXT,"ax",@progbits,unique,1022395
.globl "fn_000f99bb_"
.type "fn_000f99bb_",@function
"fn_000f99bb_":
    push edi
    {load} mov edi, eax
    {load} sub ebx, ebx
    mov bx, word ptr [edi + 8]
    cmp dx, 0x400
    je .L000f99d0
    cmp dx, 0x800
.L000f99d0:
    jne .L000f99db
    {load} or bh, bh
    jns .L000f99db
    xor dx, 0xc00
.L000f99db:
    cmp dx, 0xc00
    jne .L000f99e6
    mov dx, 0x400
.L000f99e6:
    and bx, 0x7fff
    sub bx, 0x403f
    jl .L000f99f4
    pop edi
    ret
.L000f99f4:
    mov ecx, dword ptr [edi]
    add bx, 0x20
    jl .L000f9a4a
    mov eax, dword ptr cs:[ebx*4 + "fn_000f98f6_"+0x45]
    {load} and ecx, eax
    xor dword ptr [edi], ecx
    inc eax
    cmp dx, 0x400
    je .L000f9a48
    cmp dx, 0
    jne .L000f9a35
    {load} add ecx, ecx
    {load} cmp eax, ecx
    .byte 0x0f; .byte 0x87; .long .L000f9a48-.-4  # ja 0xf9a48
    jne .L000f9a35
    rol eax, 1
    jae .L000f9a2b
    test dword ptr [edi + 4], eax
    jmp .L000f9a2d
.L000f9a2b:
    test dword ptr [edi], eax
.L000f9a2d:
    .byte 0x0f; .byte 0x84; .long .L000f9a48-.-4  # je 0xf9a48
    ror eax, 1
.L000f9a35:
    {load} or ecx, ecx
    je .L000f9a48
    add dword ptr [edi], eax
    adc dword ptr [edi + 4], 0
    jae .L000f9a48
    rcr dword ptr [edi + 4], 1
    inc word ptr [edi + 8]
.L000f9a48:
    pop edi
    ret
.L000f9a4a:
    push esi
    mov dword ptr [edi], 0
    mov esi, dword ptr [edi + 4]
    add bx, 0x20
    jl .L000f9aad
    mov eax, dword ptr cs:[ebx*4 + "fn_000f98f6_"+0x45]
    {load} and esi, eax
    xor dword ptr [edi + 4], esi
    inc eax
    cmp dx, 0x400
    je .L000f9aaa
    cmp dx, 0
    jne .L000f9a9a
    rol ecx, 1
    {load} adc esi, esi
    {load} cmp eax, esi
    .byte 0x0f; .byte 0x87; .long .L000f9aaa-.-4  # ja 0xf9aaa
    jne .L000f9a9a
    {load} or ecx, ecx
    .byte 0x0f; .byte 0x85; .long .L000f9a9e-.-4  # jne 0xf9a9e
    {load} add eax, eax
    jb .L000f9a92
    test dword ptr [edi + 4], eax
.L000f9a92:
    .byte 0x0f; .byte 0x84; .long .L000f9aaa-.-4  # je 0xf9aaa
    shr eax, 1
.L000f9a9a:
    {load} or ecx, esi
    je .L000f9aaa
.L000f9a9e:
    add dword ptr [edi + 4], eax
    jae .L000f9aaa
    rcr dword ptr [edi + 4], 1
    inc word ptr [edi + 8]
.L000f9aaa:
    pop esi
    pop edi
    ret
.L000f9aad:
    cmp dx, 0x400
    je .L000f9ae2
    cmp dx, 0
    jne .L000f9ad2
    inc bx
    .byte 0x0f; .byte 0x85; .long .L000f9ae2-.-4  # jne 0xf9ae2
    cmp esi, 0x80000000
    jne .L000f9ad2
    {load} or ecx, ecx
    .byte 0x0f; .byte 0x84; .long .L000f9ae2-.-4  # je 0xf9ae2
.L000f9ad2:
    mov dword ptr [edi + 4], 0x80000000
    or word ptr [edi + 8], 0x3fff
    pop esi
    pop edi
    ret
.L000f9ae2:
    {load} sub eax, eax
    mov dword ptr [edi + 4], eax
    mov word ptr [edi + 8], ax
    pop esi
    pop edi
    ret
    .byte 0x4e
    .byte 0x18; .byte 0xe6
    .byte 0xc0; .byte 0xe4; .byte 0xc7
    .byte 0xd1; .byte 0xb5; .byte 0xfa; .byte 0x3f; .byte 0x29; .byte 0x0b
    .byte 0x01; .byte 0x08
    .byte 0x08; .byte 0x9b; .byte 0xc6; .byte 0x84; .byte 0xfb; .byte 0xbf
    .byte 0x1a; .byte 0x26
    .byte 0xc5; .byte 0x6c; .byte 0x2e; .byte 0x02
    .byte 0x46
    .byte 0x9d
    .byte 0xfb
    .byte 0x3f
    .byte 0x2a; .byte 0x98; .byte 0x6b; .byte 0x0a; .byte 0x9d; .byte 0xb9
    .byte 0x2b; .byte 0xba; .byte 0xfb; .byte 0xbf; .byte 0x71; .byte 0x01
    .byte 0x2b; .byte 0x95; .byte 0x27; .byte 0x27; .byte 0x8e; .byte 0xe3
    .byte 0xfb
    .byte 0x3f
    .byte 0xfc
    .byte 0x68; .byte 0x9c; .byte 0xdd; .byte 0x72; .byte 0x24
    .byte 0x49
    .byte 0x92
    .byte 0xfc
    .byte 0xbf; .byte 0xc1; .byte 0xad; .byte 0xeb; .byte 0x94
    .byte 0xcc
    .byte 0xcc
    .byte 0xcc
    .byte 0xcc
    .byte 0xfc
    .byte 0x3f
    .byte 0xf2; .byte 0x81; .byte 0x97; .byte 0xaa; .byte 0xaa; .byte 0xaa; .byte 0xaa; .byte 0xaa; .byte 0xfd; .byte 0xbf; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x80; .byte 0xff; .byte 0x3f
.size "fn_000f99bb_",.-"fn_000f99bb_"

// fn_000f9b48: fpatan: arctangent of *eax / *edx, quadrant from their signs, into *ebx.
// FUNCTION: 0x000f9b48

.section _TEXT,"ax",@progbits,unique,1022792
.globl "fn_000f9b48_"
.type "fn_000f9b48_",@function
"fn_000f9b48_":
    push edi
    push esi
    push ecx
    {load} mov edi, eax
    mov cx, word ptr [edi + 8]
    shl cx, 1
    jne .L000f9b86
    test byte ptr [edx + 9], 0x80
    je .L000f9b71
    mov word ptr [ebx + 8], 0x4000
    mov dword ptr [ebx + 4], 0xc90fdaa2
    mov dword ptr [ebx], 0x2168c235
    jmp .L000f9b84
.L000f9b71:
    mov word ptr [ebx + 8], 0
    mov dword ptr [ebx + 4], 0
    mov dword ptr [ebx], 0
.L000f9b84:
    jmp .L000f9bfa
.L000f9b86:
    mov cx, word ptr [edx + 8]
    shl cx, 1
    jne .L000f9bae
    mov ax, word ptr [edi + 8]
    and ax, 0x8000
    or ax, 0x3fff
    mov word ptr [ebx + 8], ax
    mov dword ptr [ebx + 4], 0xc90fdaa2
    mov dword ptr [ebx], 0x2168c235
    jmp .L000f9bfa
.L000f9bae:
    mov ch, byte ptr [edi + 9]
    call "fn_000f932b_"
    {load} mov eax, ebx
    call "fn_000f9bfe_"
    mov dl, byte ptr [ebx + 9]
    {load} or ch, ch
    js .L000f9be0
    {load} or dl, dl
    jns .L000f9bde
    {load} mov eax, ebx
    push 0x4000
    push 0xc90fdaa2
    push 0x2168c235
    call "fn_000f90f6_"
.L000f9bde:
    jmp .L000f9bfa
.L000f9be0:
    {load} or dl, dl
    jle .L000f9bfa
    {load} mov eax, ebx
    push 0xc000
    push 0xc90fdaa2
    push 0x2168c235
    call "fn_000f90f6_"
.L000f9bfa:
    pop ecx
    pop esi
    pop edi
    ret
.size "fn_000f9b48_",.-"fn_000f9b48_"

// fn_000f9bfe: arctangent of the long double at eax, in place (|x| >= 1 is reduced through 1/x).
// FUNCTION: 0x000f9bfe

.section _TEXT,"ax",@progbits,unique,1022974
.globl "fn_000f9bfe_"
.type "fn_000f9bfe_",@function
"fn_000f9bfe_":
    push edi
    push esi
    push ecx
    {load} mov edi, eax
    mov ch, 0
    mov cl, byte ptr [edi + 9]
    and byte ptr [edi + 9], 0x7f
    cmp word ptr [edi + 8], 0x3fff
    jne .L000f9c40
    cmp dword ptr [edi + 4], 0x80000000
    jne .L000f9c40
    cmp dword ptr [edi], 0
    jne .L000f9c40
    mov dword ptr [edi], 0x2168c235
    mov dword ptr [edi + 4], 0xc90fdaa2
    mov ax, 0x3ffe
    and cl, 0x80
    {load} or ah, cl
    mov word ptr [edi + 8], ax
    pop ecx
    pop esi
    pop edi
    ret
.L000f9c40:
    push edx
    push ebx
    cmp word ptr [edi + 8], 0x3fff
    jl .L000f9c76
    push ecx
    mov si, word ptr [edi + 8]
    mov ecx, dword ptr [edi + 4]
    mov ebx, dword ptr [edi]
    shl esi, 0x10
    mov si, 0x3fff
    mov edx, 0x80000000
    mov eax, 0
    call "fn_000f9355_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop ecx
    mov ch, 2
.L000f9c76:
    mov ax, word ptr [edi + 8]
    cmp ax, 0x3ffd
    jne .L000f9c91
    mov eax, dword ptr [edi + 4]
    cmp eax, 0x8930a2f4
    jne .L000f9c91
    mov eax, dword ptr [edi]
    cmp eax, 0xf66ab09b
.L000f9c91:
    jbe .L000f9d04
    push ecx
    mov ebx, 0xc265539e
    mov ecx, 0xddb3d742
    mov esi, 0x3fff0000
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    call "fn_000f9126_"
    push eax
    push edx
    push esi
    mov ebx, 0xc265539e
    mov ecx, 0xddb3d742
    mov esi, 0x3fff0000
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    call "fn_000f952a_"
    {load} sub ebx, ebx
    mov ecx, 0x80000000
    rol esi, 0x10
    mov si, 0xbfff
    rol esi, 0x10
    call "fn_000f9126_"
    pop ebx
    shl ebx, 0x10
    {load} mov bx, si
    {load} mov esi, ebx
    pop ecx
    pop ebx
    call "fn_000f9355_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop ecx
    or ch, 1
.L000f9d04:
    {load} mov eax, edi
    lea edx, ["fn_000f99bb_"+0x133]
    mov ebx, 8
    call "fn_000fa77a_"
    shr ch, 1
    jae .L000f9d32
    {load} mov eax, edi
    push 0x3ffe
    push 0x860a91c1
    push 0x6b9b2c23
    {load} mov ebx, edi
    call "fn_000f90f6_"
.L000f9d32:
    shr ch, 1
    jae .L000f9d52
    push 0xbfff
    push 0xc90fdaa2
    push 0x2168c235
    {load} mov eax, edi
    {load} mov ebx, edi
    call "fn_000f90f6_"
    xor byte ptr [ebx + 9], 0x80
.L000f9d52:
    cmp cl, 0
    jns .L000f9d5b
    xor byte ptr [edi + 9], 0x80
.L000f9d5b:
    pop ebx
    pop edx
    pop ecx
    pop esi
    pop edi
    ret
.size "fn_000f9bfe_",.-"fn_000f9bfe_"

// fn_000f9d61: partial remainder (fprem): *eax = *eax mod *edx; returns the low quotient bits.
// FUNCTION: 0x000f9d61

.section _TEXT,"ax",@progbits,unique,1023329
.globl "fn_000f9d61_"
.type "fn_000f9d61_",@function
"fn_000f9d61_":
    cmp word ptr [eax + 8], 0
    jne .L000f9d76
    cmp dword ptr [eax + 4], 0
    jne .L000f9d76
    cmp dword ptr [eax], 0
    jne .L000f9d76
    {load} sub eax, eax
    ret
.L000f9d76:
    cmp word ptr [edx + 8], 0
    jne .L000f9d96
    cmp dword ptr [edx + 4], 0
    jne .L000f9d96
    cmp dword ptr [edx], 0
    jne .L000f9d96
    {load} sub edx, edx
    mov dword ptr [eax], edx
    mov dword ptr [eax + 4], edx
    mov word ptr [eax + 8], dx
    {load} sub eax, eax
    ret
.L000f9d96:
    push ebp
    push esi
    push edi
    push edx
    push ecx
    push ebx
    push eax
    mov si, word ptr [eax + 8]
    mov di, word ptr [edx + 8]
    mov ecx, dword ptr [edx + 4]
    mov ebx, dword ptr [edx]
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f9dc7_"
    pop ecx
    mov dword ptr [ecx], eax
    mov dword ptr [ecx + 4], edx
    mov word ptr [ecx + 8], si
    {load} mov eax, edi
    pop ebx
    pop ecx
    pop edx
    pop edi
    pop esi
    pop ebp
    ret
.size "fn_000f9d61_",.-"fn_000f9d61_"

// fn_000f9dc7: remainder core on registers for fn_000f9d61; the quotient bits are returned in edi.
// FUNCTION: 0x000f9dc7

.section _TEXT,"ax",@progbits,unique,1023431
.globl "fn_000f9dc7_"
.type "fn_000f9dc7_",@function
"fn_000f9dc7_":
    push esi
    push edi
    and esi, 0x7fff
    and edi, 0x7fff
    {load} sub esi, edi
    jl .L000f9e26
    {load} sub edi, edi
.L000f9ddb:
    {load} cmp ecx, edx
    jne .L000f9de7
    {load} cmp ebx, eax
    .byte 0x0f; .byte 0x84; .long .L000f9de9-.-4  # je 0xf9de9
.L000f9de7:
    jae .L000f9dee
.L000f9de9:
    {load} sub eax, ebx
    {load} sbb edx, ecx
    stc
.L000f9dee:
    {load} adc edi, edi
    dec esi
    .byte 0x0f; .byte 0x8c; .long .L000f9e03-.-4  # jl 0xf9e03
    {load} add eax, eax
    {load} adc edx, edx
    jb .L000f9de9
    {load} or edx, edx
    js .L000f9ddb
    jmp .L000f9dee
.L000f9e03:
    {load} sub esi, esi
    {load} or eax, eax
    jne .L000f9e0f
    {load} or edx, edx
    jne .L000f9e0f
    jmp .L000f9e24
.L000f9e0f:
    {load} or edx, edx
    js .L000f9e1a
    {load} adc eax, eax
    {load} adc edx, edx
    dec esi
    jmp .L000f9e0f
.L000f9e1a:
    pop ecx
    push ecx
    and ecx, 0x7fff
    {load} add esi, ecx
.L000f9e24:
    jmp .L000f9e2a
.L000f9e26:
    {load} add esi, edi
    {load} sub edi, edi
.L000f9e2a:
    pop ecx
    pop ebx
    and ebx, 0x8000
    {load} or esi, ebx
    {load} or edx, edx
    jne .L000f9e3a
    {load} sub esi, esi
.L000f9e3a:
    {load} xor bx, cx
    jns .L000f9e41
    neg edi
.L000f9e41:
    ret
.size "fn_000f9dc7_",.-"fn_000f9dc7_"

// fn_000f9e42: fxam: classify the long double at eax into condition codes C0-C3 in the status word, and update its tag.
// FUNCTION: 0x000f9e42

.section _TEXT,"ax",@progbits,unique,1023554
.globl "fn_000f9e42_"
.type "fn_000f9e42_",@function
"fn_000f9e42_":
    push ebx
    push edx
    {load} mov ebx, eax
    mov ax, word ptr cs:[edi + "fn_000f75a0_"+0x5e]
    {load} mov edx, eax
    and ax, word ptr ds:[ebp + 8]
    {load} cmp ax, dx
    jne .L000f9e60
    mov al, 0x41
    mov dh, 0
    jmp .L000f9ed3
.L000f9e60:
    not edx
    and word ptr ds:[ebp + 8], dx
    mov ax, word ptr [ebx + 8]
    {load} mov edx, eax
    and dh, 0x80
    rol dh, 1
    shl dh, 1
    and ax, 0x7fff
    jne .L000f9e94
    mov eax, dword ptr [ebx + 4]
    or eax, dword ptr [ebx]
    mov al, 0x44
    jne .L000f9e92
    mov ax, word ptr cs:[edi + "fn_000f75a0_"+0x58]
    or word ptr ds:[ebp + 8], ax
    mov al, 0x40
.L000f9e92:
    jmp .L000f9ed3
.L000f9e94:
    cmp ax, 0x7fff
    jne .L000f9ebc
    mov ax, word ptr cs:[edi + "fn_000f75a0_"+0x5c]
    or word ptr ds:[ebp + 8], ax
    cmp dword ptr [ebx], 0
    jne .L000f9eb4
    mov eax, dword ptr [ebx + 4]
    cmp eax, 0x80000000
.L000f9eb4:
    mov al, 1
    jne .L000f9eba
    mov al, 5
.L000f9eba:
    jmp .L000f9ed3
.L000f9ebc:
    test byte ptr [ebx + 7], 0x80
    mov al, 4
    jne .L000f9ed3
    mov ax, word ptr cs:[edi + "fn_000f75a0_"+0x5c]
    or word ptr ds:[ebp + 8], ax
    mov al, 0x44
.L000f9ed3:
    {load} or al, dh
    mov dx, word ptr ds:[ebp + 4]
    and dx, 0xb8ff
    {load} or dh, al
    mov word ptr ds:[ebp + 4], dx
    pop edx
    pop ebx
    ret
    .byte 0x90
    .byte 0xe9; .byte 0x46; .byte 0xe9; .byte 0xb5; .byte 0x9a
    .byte 0xad
    .byte 0x20; .byte 0xca
    .byte 0xfe
    .byte 0xbf; .byte 0x09; .byte 0x65; .byte 0x7f; .byte 0xb5
    .byte 0x00; .byte 0x51; .byte 0x12
    .byte 0x83; .byte 0x03; .byte 0x40
    .byte 0x28; .byte 0xd2
    .byte 0xac
    .byte 0x9d
    .byte 0x95
    .byte 0xf8
    .byte 0x3f
    .byte 0x80; .byte 0x05; .byte 0xc0; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x80; .byte 0xff; .byte 0x3f; .byte 0xbb; .byte 0x76
    .byte 0x70; .byte 0x3e
    .byte 0x5b
    .byte 0x02; .byte 0xac; .byte 0x8e; .byte 0x04; .byte 0xc0; .byte 0x60; .byte 0xef
    .byte 0x33; .byte 0xa9; .byte 0xd0; .byte 0x1f; .byte 0x04; .byte 0x9c
    .byte 0x07
    .byte 0x40
    .byte 0x96
    .byte 0xbb; .byte 0x83; .byte 0x6c; .byte 0xe0; .byte 0xf4
    .byte 0x5f
    .byte 0xc0; .byte 0x08; .byte 0xc0
.size "fn_000f9e42_",.-"fn_000f9e42_"

// fn_000f9f30: add 1.0 to the long double at eax.
// FUNCTION: 0x000f9f30

.section _TEXT,"ax",@progbits,unique,1023792
.globl "fn_000f9f30_"
.type "fn_000f9f30_",@function
"fn_000f9f30_":
    push ebx
    push edx
    push eax
    push 0x3fff
    push 0x80000000
    push 0
    {load} mov ebx, eax
    call "fn_000f90f6_"
    pop eax
    pop edx
    pop ebx
.size "fn_000f9f30_",.-"fn_000f9f30_"

// fn_000f9f49: fyl2x: *edx = *edx * log2(*eax).
// FUNCTION: 0x000f9f49

.section _TEXT,"ax",@progbits,unique,1023817
.globl "fn_000f9f49_"
.type "fn_000f9f49_",@function
"fn_000f9f49_":
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    call "fn_000f9f8c_"
    pop eax
    {load} mov ebx, edx
    push edx
    call "fn_000f9500_"
    pop edi
    mov si, word ptr [edi + 8]
    mov ecx, dword ptr [edi + 4]
    mov ebx, dword ptr [edi]
    shl esi, 0x10
    mov si, 0x3fff
    mov edx, 0xb8aa3b29
    mov eax, 0x5c17f0bd
    call "fn_000f952a_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    ret
.size "fn_000f9f49_",.-"fn_000f9f49_"

// fn_000f9f8c: natural logarithm of the long double at eax, in place (mantissa reduced about sqrt(1/2), then a series).
// FUNCTION: 0x000f9f8c

.section _TEXT,"ax",@progbits,unique,1023884
.globl "fn_000f9f8c_"
.type "fn_000f9f8c_",@function
"fn_000f9f8c_":
    push edi
    push esi
    push edx
    push ecx
    push ebx
    {load} sub ecx, ecx
    mov cx, word ptr [eax + 8]
    {load} or cx, cx
    js .L000f9f9e
    jmp .L000f9f9e
.L000f9f9e:
    {load} mov edi, eax
    mov edx, 0x3ffe
    sub ecx, 0x3ffe
    push ecx
    mov word ptr [edi + 8], dx
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov esi, 0xbffe3ffe
    mov ecx, 0x80000000
    {load} sub ebx, ebx
    call "fn_000f9126_"
    cmp word ptr [edi + 8], 0x3ffe
    jne .L000f9fdd
    cmp dword ptr [edi + 4], 0xb504f333
    jne .L000f9fdd
    cmp dword ptr [edi], 0xf9de6484
.L000f9fdd:
    jbe .L000f9ff9
    and esi, 0xffff
    or esi, 0xbffe0000
    mov ecx, 0x80000000
    {load} sub ebx, ebx
    call "fn_000f9126_"
    jmp .L000fa005
.L000f9ff9:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    dec dword ptr [esp]
.L000fa005:
    push eax
    push edx
    push esi
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    dec si
    and esi, 0xffff
    or esi, 0x3ffe0000
    mov ecx, 0x80000000
    {load} sub ebx, ebx
    call "fn_000f9126_"
    shl esi, 0x10
    {load} mov ecx, edx
    {load} mov ebx, eax
    pop eax
    {load} mov si, ax
    pop edx
    pop eax
    call "fn_000f9355_"
    push eax
    push edx
    push esi
    {load} mov ecx, esi
    shl esi, 0x10
    {load} mov si, cx
    {load} mov ecx, edx
    {load} mov ebx, eax
    call "fn_000f952a_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    mov ebx, 3
    lea ecx, ["fn_000f9e42_"+0xc6]
    call "fn_000fa728_"
    push eax
    push edx
    push esi
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    lea ecx, ["fn_000f9e42_"+0xa8]
    mov ebx, 2
    call "fn_000fa728_"
    pop ecx
    rol esi, 0x10
    {load} mov si, cx
    rol esi, 0x10
    pop ecx
    pop ebx
    call "fn_000f9355_"
    mov ebx, dword ptr [edi]
    mov ecx, dword ptr [edi + 4]
    rol esi, 0x10
    mov si, word ptr [edi + 8]
    rol esi, 0x10
    call "fn_000f952a_"
    rol esi, 0x10
    mov si, word ptr [esp]
    rol esi, 0x10
    mov ecx, dword ptr [esp + 4]
    mov ebx, dword ptr [esp + 8]
    call "fn_000f952a_"
    rol esi, 0x10
    pop ecx
    {load} mov si, cx
    rol esi, 0x10
    pop ecx
    pop ebx
    call "fn_000f9126_"
    pop ecx
    {load} or ecx, ecx
    je .L000fa14e
    push eax
    push edx
    push esi
    {load} mov edx, edi
    {load} mov eax, ecx
    call "fn_000f96ea_"
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    and esi, 0xffff
    or esi, 0xbff20000
    mov ecx, 0xde8082e3
    mov ebx, 0x865435c
    call "fn_000f952a_"
    pop ecx
    rol esi, 0x10
    {load} mov si, cx
    rol esi, 0x10
    pop ecx
    pop ebx
    call "fn_000f9126_"
    push eax
    push edx
    push esi
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    and esi, 0xffff
    or esi, 0x3ffe0000
    mov ecx, 0xb1800000
    {load} sub ebx, ebx
    call "fn_000f952a_"
    pop ecx
    rol esi, 0x10
    {load} mov si, cx
    rol esi, 0x10
    pop ecx
    pop ebx
    call "fn_000f9126_"
.L000fa14e:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    ret
    .byte 0x90
    .byte 0x59
    .byte 0x53
    .byte 0x6a; .byte 0x85
    .byte 0x81; .byte 0x3b; .byte 0x96; .byte 0xca; .byte 0xce; .byte 0x3f
    .byte 0x8f
    .byte 0xf8
    .byte 0xc0; .byte 0x9d; .byte 0x39; .byte 0x9f; .byte 0x3f; .byte 0xd7; .byte 0xd6
    .byte 0xbf; .byte 0xe5; .byte 0x4b; .byte 0x68; .byte 0x43
    .byte 0x9d
    .byte 0x30; .byte 0x92; .byte 0xb0; .byte 0xde; .byte 0x3f; .byte 0x7f
    .byte 0x1c; .byte 0x27
    .byte 0xaa
    .byte 0x3f
    .byte 0x2b; .byte 0x32
    .byte 0xd7
    .byte 0xe5; .byte 0xbf
    .byte 0x7d; .byte 0x9c
    .byte 0x39; .byte 0xb6; .byte 0x2a; .byte 0x1d; .byte 0xef; .byte 0xb8
    .byte 0xec
    .byte 0x3f
    .byte 0x0c; .byte 0xd0
    .byte 0x00; .byte 0x0d; .byte 0xd0; .byte 0x00; .byte 0x0d; .byte 0xd0
    .byte 0xf2; .byte 0xbf; .byte 0x88; .byte 0x88; .byte 0x88; .byte 0x88
    .byte 0x88; .byte 0x88; .byte 0x88; .byte 0x88; .byte 0xf8; .byte 0x3f
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xfc
    .byte 0xbf; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x80; .byte 0xff; .byte 0x3f; .byte 0x8f; .byte 0xf8
    .byte 0xc0; .byte 0x9d; .byte 0x39; .byte 0x9f; .byte 0x3f; .byte 0xd7; .byte 0xd2
    .byte 0x3f
    .byte 0x05; .byte 0xe9; .byte 0xe4; .byte 0x03; .byte 0x46
    .byte 0xa5
    .byte 0xcb
    .byte 0xc9
    .byte 0xda; .byte 0xbf; .byte 0xaa; .byte 0xbd; .byte 0xc4; .byte 0xc6
    .byte 0x7f; .byte 0xc7
    .byte 0x76; .byte 0x8f
    .byte 0xe2; .byte 0x3f
    .byte 0x97
    .byte 0xe3; .byte 0xfa
    .byte 0xc4; .byte 0xbb; .byte 0x7d; .byte 0xf2; .byte 0x93; .byte 0xe9
    .byte 0xbf; .byte 0x0d; .byte 0xd0; .byte 0x00; .byte 0x0d
    .byte 0xd0; .byte 0x00
    .byte 0x0d; .byte 0xd0; .byte 0xef; .byte 0x3f; .byte 0x0b
    .byte 0xb6; .byte 0x60
    .byte 0x0b; .byte 0xb6; .byte 0x60; .byte 0x0b; .byte 0xb6; .byte 0xf5
    .byte 0xbf; .byte 0xaa; .byte 0xaa; .byte 0xaa; .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xaa
    .byte 0xfa
    .byte 0x3f
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x80; .byte 0xfe; .byte 0xbf; .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x80; .byte 0xff; .byte 0x3f; .byte 0x00; .byte 0x00
    .byte 0x01; .byte 0x00
    .byte 0x02; .byte 0x00
    .byte 0x03; .byte 0x00
    .byte 0x04; .byte 0x00
    .byte 0x04; .byte 0x00
    .byte 0x05; .byte 0x00; .byte 0x05; .byte 0x00; .byte 0x06
    .byte 0x00
.size "fn_000f9f8c_",.-"fn_000f9f8c_"

// fn_000fa224: sine, tangent or cosine (esi 0, 1, 2) of the long double at eax, in place, reducing by octants of pi.
// FUNCTION: 0x000fa224

.section _TEXT,"ax",@progbits,unique,1024548
.globl "fn_000fa224_"
.type "fn_000fa224_",@function
"fn_000fa224_":
    push ebp
    push edi
    push edx
    push ecx
    push ebx
    {load} mov ebp, esp
    sub esp, 0xc
    {load} mov edi, eax
    {load} sub ecx, ecx
    mov cx, word ptr [eax + 8]
    and cx, 0x7fff
    sub cx, 0x3ffe
    cmp cx, 0x20
    jl .L000fa246
.L000fa246:
    push ecx
    push esi
    push edi
    mov si, word ptr [eax + 8]
    mov di, 0x3ffe
    mov ecx, 0xc90fdaa2
    mov ebx, 0x2168c235
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    call "fn_000f9dc7_"
    pop ecx
    mov dword ptr [ecx], eax
    mov dword ptr [ecx + 4], edx
    mov word ptr [ecx + 8], si
    {load} mov eax, edi
    {load} mov edi, ecx
    pop esi
    test byte ptr [edi + 9], 0x80
    je .L000fa295
    push eax
    {load} mov ebx, edi
    {load} mov eax, edi
    push 0x3ffe
    push 0xc90fdaa2
    push 0x2168c235
    call "fn_000f90f6_"
    pop eax
    dec eax
.L000fa295:
    {load} mov ebx, esi
    {load} mov ah, bl
    and bl, 2
    {load} add al, bl
    and al, 7
    push eax
    test al, 1
    je .L000fa2c1
    push 0xbffe
    push 0xc90fdaa2
    push 0x2168c235
    {load} mov eax, edi
    {load} mov ebx, edi
    call "fn_000f90f6_"
    xor byte ptr [ebx + 9], 0x80
.L000fa2c1:
    {load} sub ecx, ecx
    mov cx, word ptr [edi + 8]
    sub cx, 0x3ffe
    cmp cx, -0x20
    jge .L000fa2eb
    mov dword ptr [ebp - 0xc], 0
    mov dword ptr [ebp - 8], 0x80000000
    mov word ptr [ebp - 4], 0x3fff
    jmp .L000fa382
.L000fa2eb:
    {load} or cx, cx
    js .L000fa2f4
    mov cx, 0
.L000fa2f4:
    neg cx
    cmp cx, 8
    jle .L000fa301
    mov cx, 8
.L000fa301:
    mov cx, word ptr cs:[ecx*2 + "fn_000f9f8c_"+0x286]
    push ecx
    mov eax, dword ptr [ebp - 0x14]
    inc al
    and al, 2
    jne .L000fa31b
    cmp ah, 1
    je .L000fa31b
    jmp .L000fa359
.L000fa31b:
    push eax
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    shl esi, 0x10
    mov si, word ptr [edi + 8]
    {load} mov ebx, eax
    {load} mov ecx, edx
    call "fn_000f952a_"
    mov ecx, dword ptr [ebp - 0x18]
    mov ebx, 8
    {load} sub ebx, ecx
    lea ecx, [ecx + ecx*4]
    lea ecx, [ecx*2 + "fn_000f9f8c_"+0x22c]
    call "fn_000fa728_"
    mov dword ptr [ebp - 0xc], eax
    mov dword ptr [ebp - 8], edx
    mov word ptr [ebp - 4], si
    pop eax
.L000fa359:
    cmp al, 0
    je .L000fa364
    cmp ah, 1
    je .L000fa364
    jmp .L000fa382
.L000fa364:
    {load} mov eax, edi
    mov ecx, dword ptr [ebp - 0x18]
    mov ebx, 8
    {load} sub ebx, ecx
    lea ecx, [ecx + ecx*4]
    {load} add ecx, ecx
    lea edx, ["fn_000f9f8c_"+0x1d2]
    {load} add edx, ecx
    call "fn_000fa77a_"
.L000fa382:
    mov eax, dword ptr [ebp - 0x14]
    inc al
    and al, 2
    je .L000fa397
    mov eax, dword ptr [ebp - 0xc]
    mov edx, dword ptr [ebp - 8]
    mov si, word ptr [ebp - 4]
    jmp .L000fa3a0
.L000fa397:
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
.L000fa3a0:
    mov ecx, dword ptr [ebp - 0x14]
    test cl, 4
    je .L000fa3ad
    xor si, 0x8000
.L000fa3ad:
    cmp ch, 1
    jne .L000fa40a
    rol esi, 0x10
    add cl, 2
    {load} mov bl, cl
    inc bl
    and bl, 2
    je .L000fa3d0
    and cl, 4
    mov ebx, dword ptr [ebp - 0xc]
    mov ecx, dword ptr [ebp - 8]
    mov si, word ptr [ebp - 4]
    jmp .L000fa3dc
.L000fa3d0:
    and cl, 4
    mov ebx, dword ptr [edi]
    mov ecx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
.L000fa3dc:
    je .L000fa3e4
    xor esi, 0x8000
.L000fa3e4:
    rol esi, 0x10
    {load} or ebx, ebx
    jne .L000fa405
    {load} or ecx, ecx
    jne .L000fa405
    test esi, 0x7fff0000
    jne .L000fa405
    {load} sub eax, eax
    mov edx, 0x80000000
    or si, 0x7fff
    jmp .L000fa40a
.L000fa405:
    call "fn_000f9355_"
.L000fa40a:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    {load} mov esp, ebp
    pop ebx
    pop ecx
    pop edx
    pop edi
    pop ebp
    ret
.size "fn_000fa224_",.-"fn_000fa224_"

// fn_000fa41b: sine of the long double at eax, in place.
// FUNCTION: 0x000fa41b

.section _TEXT,"ax",@progbits,unique,1025051
.globl "fn_000fa41b_"
.type "fn_000fa41b_",@function
"fn_000fa41b_":
    push esi
    {load} sub esi, esi
    call "fn_000fa224_"
    pop esi
    ret
.size "fn_000fa41b_",.-"fn_000fa41b_"

// fn_000fa425: cosine of the long double at eax, in place.
// FUNCTION: 0x000fa425

.section _TEXT,"ax",@progbits,unique,1025061
.globl "fn_000fa425_"
.type "fn_000fa425_",@function
"fn_000fa425_":
    push esi
    mov esi, 2
    call "fn_000fa224_"
    pop esi
    ret
.size "fn_000fa425_",.-"fn_000fa425_"

// fn_000fa432: tangent of the long double at eax, in place.
// FUNCTION: 0x000fa432

.section _TEXT,"ax",@progbits,unique,1025074
.globl "fn_000fa432_"
.type "fn_000fa432_",@function
"fn_000fa432_":
    push esi
    mov esi, 1
    call "fn_000fa224_"
    pop esi
    ret
.size "fn_000fa432_",.-"fn_000fa432_"

// fn_000fa440: the 2^x routine's table of long double constants, not code.
// FUNCTION: 0x000fa440

.section _TEXT,"ax",@progbits,unique,1025088
.globl "fn_000fa440_"
.type "fn_000fa440_",@function
"fn_000fa440_":
    adc eax, 0x67cc487b
    ret
    .byte 0xaa
    .byte 0x85; .byte 0xff
    .byte 0x3f
    .byte 0xe7; .byte 0xd6
    .byte 0x8b; .byte 0xea
    .byte 0xe3; .byte 0xc1
    .byte 0x95
    .byte 0x8b; .byte 0xff
    .byte 0x3f
    .byte 0x6f
    .byte 0xa9; .byte 0xb8; .byte 0x8d; .byte 0x51; .byte 0xf0
    .byte 0x37
    .byte 0x98
    .byte 0xff
    .byte 0x3f
    .byte 0x84; .byte 0x64; .byte 0xde; .byte 0xf9
    .byte 0x33; .byte 0xf3
    .byte 0x04; .byte 0xb5
    .byte 0xff
    .byte 0x3f
.size "fn_000fa440_",.-"fn_000fa440_"

// fn_000fa468: f2xm1: *eax = 2^(*eax) - 1.
// FUNCTION: 0x000fa468

.section _TEXT,"ax",@progbits,unique,1025128
.globl "fn_000fa468_"
.type "fn_000fa468_",@function
"fn_000fa468_":
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    {load} mov edi, eax
    {load} sub esi, esi
    mov cx, word ptr [edi + 8]
    and byte ptr [edi + 9], 0x7f
    cmp word ptr [edi + 8], 0
    jne .L000fa48a
    cmp dword ptr [edi + 4], 0
    jne .L000fa48a
    cmp dword ptr [edi], 0
.L000fa48a:
    je .L000fa4f2
    {load} or cx, cx
    jns .L000fa4ab
    dec esi
    push 0xbfff
    push 0x80000000
    push 0
    {load} mov eax, edi
    {load} mov ebx, edi
    call "fn_000f90f6_"
    xor byte ptr [ebx + 9], 0x80
.L000fa4ab:
    mov bx, word ptr [edi + 8]
    add bx, 4
    {load} sub ecx, ecx
    mov edx, dword ptr [edi + 4]
    mov eax, dword ptr [edi]
.L000fa4ba:
    cmp bx, 0x3fff
    jb .L000fa4cb
    {load} add eax, eax
    {load} adc edx, edx
    {load} adc ecx, ecx
    dec bx
    jmp .L000fa4ba
.L000fa4cb:
    {load} or edx, edx
    jne .L000fa4d1
    {load} or eax, eax
.L000fa4d1:
    jne .L000fa4d7
    {load} sub ebx, ebx
    jmp .L000fa4e7
.L000fa4d7:
    {load} or edx, edx
    js .L000fa4e3
    {load} add eax, eax
    {load} adc edx, edx
    dec bx
    jmp .L000fa4d7
.L000fa4e3:
    sub bx, 4
.L000fa4e7:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], bx
    jmp .L000fa4f4
.L000fa4f2:
    {load} sub ecx, ecx
.L000fa4f4:
    push esi
    push ecx
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    shl esi, 0x10
    mov si, word ptr [edi + 8]
    {load} mov ebx, eax
    {load} mov ecx, edx
    call "fn_000f952a_"
    push eax
    push edx
    push esi
    mov ecx, 0xa6829a79
    mov ebx, 0x1bf21f8c
    and esi, 0xffff
    or esi, 0x40030000
    call "fn_000f9126_"
    {load} mov ebp, esp
    xchg dword ptr [ebp + 8], eax
    xchg dword ptr [ebp + 4], edx
    xchg dword ptr [ebp], esi
    mov ecx, 0xec96f0d6
    mov ebx, 0x9d7bfdb
    and esi, 0xffff
    or esi, 0x3ffa0000
    call "fn_000f952a_"
    mov ecx, 0xe6d5051a
    mov ebx, 0xe536e187
    and esi, 0xffff
    or esi, 0x40010000
    call "fn_000f9126_"
    mov ebx, dword ptr [edi]
    mov ecx, dword ptr [edi + 4]
    rol esi, 0x10
    mov si, word ptr [edi + 8]
    rol esi, 0x10
    call "fn_000f952a_"
    push eax
    push edx
    push esi
    xor esi, 0x8000
    rol esi, 0x10
    mov si, word ptr [ebp]
    rol esi, 0x10
    mov ebx, dword ptr [ebp + 8]
    mov ecx, dword ptr [ebp + 4]
    call "fn_000f9126_"
    xchg dword ptr [ebp - 4], eax
    xchg dword ptr [ebp - 8], edx
    xchg dword ptr [ebp - 0xc], esi
    rol esi, 0x10
    mov si, word ptr [ebp]
    rol esi, 0x10
    mov ebx, dword ptr [ebp + 8]
    mov ecx, dword ptr [ebp + 4]
    call "fn_000f9126_"
    rol esi, 0x10
    mov si, word ptr [ebp - 0xc]
    rol esi, 0x10
    pop ecx
    pop ecx
    pop ebx
    call "fn_000f9355_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    add esp, 0xc
    pop ecx
    lea esi, ["fn_000fa440_"]
.L000fa5e3:
    {load} or ecx, ecx
    je .L000fa61b
    shr ecx, 1
    jae .L000fa616
    push ecx
    push esi
    mov ax, word ptr [edi + 8]
    mov ecx, dword ptr [edi + 4]
    mov ebx, dword ptr [edi]
    shl eax, 0x10
    mov ax, word ptr cs:[esi + 8]
    mov edx, dword ptr cs:[esi + 4]
    mov esi, dword ptr cs:[esi]
    xchg esi, eax
    call "fn_000f952a_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop esi
    pop ecx
.L000fa616:
    lea esi, [esi + 0xa]
    jmp .L000fa5e3
.L000fa61b:
    pop esi
    add word ptr [edi + 8], si
    {load} mov eax, edi
    push 0xbfff
    push 0x80000000
    push 0
    {load} mov ebx, edi
    call "fn_000f90f6_"
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    ret
.size "fn_000fa468_",.-"fn_000fa468_"

// fn_000fa63c: square root of the long double at eax, in place; negative values give the indefinite NaN.
// FUNCTION: 0x000fa63c

.section _TEXT,"ax",@progbits,unique,1025596
.globl "fn_000fa63c_"
.type "fn_000fa63c_",@function
"fn_000fa63c_":
    push edi
    push esi
    push edx
    push ecx
    push ebx
    mov cx, word ptr [eax + 8]
    mov edx, dword ptr [eax + 4]
    mov ebx, dword ptr [eax]
    {load} or ebx, ebx
    jne .L000fa682
    {load} or edx, edx
    jne .L000fa677
    {load} add cx, cx
    je .L000fa702
.L000fa65b:
    mov word ptr [eax + 8], 0xffff
    mov dword ptr [eax + 4], 0
    mov dword ptr [eax], 0
.L000fa66e:
    or byte ptr [eax + 7], 0xc0
    jmp .L000fa702
.L000fa677:
    cmp cx, 0x7fff
    je .L000fa702
.L000fa682:
    {load} mov edx, ecx
    and dh, 0x7f
    cmp dx, 0x7fff
    je .L000fa66e
    {load} add cx, cx
    jb .L000fa66e
    shr cx, 1
    je .L000fa69f
    mov edx, dword ptr [eax + 4]
    {load} add edx, edx
    jae .L000fa65b
.L000fa69f:
    mov edx, dword ptr [eax + 4]
    {load} sub edi, edi
    push eax
    sub cx, 0x3fff
    sar cx, 1
    jb .L000fa6b5
    shr edx, 1
    rcr ebx, 1
    rcr edi, 1
.L000fa6b5:
    add cx, 0x3fff
    mov word ptr [eax + 8], cx
    {load} mov ecx, edx
    {load} mov eax, ebx
    {load} mov esi, edx
    stc
    rcr esi, 1
    inc edx
    je .L000fa6e8
    dec edx
.L000fa6cb:
    div esi
    dec esi
    {load} cmp esi, eax
    jbe .L000fa6dd
    inc esi
    {load} add esi, eax
    rcr esi, 1
    {load} mov edx, ecx
    {load} mov eax, ebx
    jmp .L000fa6cb
.L000fa6dd:
    inc esi
    {load} mov ecx, eax
    {load} mov eax, edi
    div esi
    {load} add esi, ecx
    jmp .L000fa6f1
.L000fa6e8:
    {load} cmp eax, esi
    je .L000fa6fa
    xchg edx, eax
    {load} mov eax, edi
    div esi
.L000fa6f1:
    {load} sub edx, edx
    stc
    rcr esi, 1
    rcr eax, 1
    {load} adc eax, edx
.L000fa6fa:
    {load} adc edx, esi
    pop esi
    mov dword ptr [esi], eax
    mov dword ptr [esi + 4], edx
.L000fa702:
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    ret
    .byte 0x56
    .byte 0x51
    .byte 0x50
    .byte 0x8b; .byte 0xca
    .byte 0x66; .byte 0x8b; .byte 0x70; .byte 0x08
    .byte 0x8b; .byte 0x50; .byte 0x04
    .byte 0x8b; .byte 0x00
    .byte 0xe8; .long "fn_000fa728_"-.-4
    .byte 0x59
    .byte 0x89; .byte 0x01
    .byte 0x89; .byte 0x51; .byte 0x04
    .byte 0x66; .byte 0x89; .byte 0x71; .byte 0x08
    .byte 0x59
    .byte 0x5e
    .byte 0xc3
.size "fn_000fa63c_",.-"fn_000fa63c_"

// fn_000fa728: evaluate a polynomial by Horner's rule at x (edx:eax, exponent si) with ebx steps over the long double coefficients at cs:ecx; result in edx:eax:si.
// FUNCTION: 0x000fa728

.section _TEXT,"ax",@progbits,unique,1025832
.globl "fn_000fa728_"
.type "fn_000fa728_",@function
"fn_000fa728_":
    push ebp
    push edi
    push ecx
    push ebx
    push esi
    push edx
    push eax
    {load} mov ebp, esp
    {load} mov edi, ecx
    mov eax, dword ptr cs:[edi]
    mov edx, dword ptr cs:[edi + 4]
    mov si, word ptr cs:[edi + 8]
.L000fa73f:
    rol esi, 0x10
    mov si, word ptr [ebp + 8]
    rol esi, 0x10
    mov ebx, dword ptr [ebp]
    mov ecx, dword ptr [ebp + 4]
    call "fn_000f952a_"
    lea edi, [edi + 0xa]
    mov ebx, dword ptr cs:[edi]
    mov ecx, dword ptr cs:[edi + 4]
    rol esi, 0x10
    mov si, word ptr cs:[edi + 8]
    rol esi, 0x10
    call "fn_000f9126_"
    dec dword ptr [ebp + 0xc]
    jne .L000fa73f
    add esp, 0x10
    pop ecx
    pop edi
    pop ebp
    ret
.size "fn_000fa728_",.-"fn_000fa728_"

// fn_000fa77a: *eax = x * P(x^2), with P's coefficients at cs:edx and ebx steps (fn_000fa728).
// FUNCTION: 0x000fa77a

.section _TEXT,"ax",@progbits,unique,1025914
.globl "fn_000fa77a_"
.type "fn_000fa77a_",@function
"fn_000fa77a_":
    push edi
    push esi
    push ecx
    push ebx
    push edx
    {load} mov edi, eax
    mov eax, dword ptr [edi]
    mov edx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    shl esi, 0x10
    mov si, word ptr [edi + 8]
    {load} mov ebx, eax
    {load} mov ecx, edx
    call "fn_000f952a_"
    pop ecx
    pop ebx
    call "fn_000fa728_"
    rol esi, 0x10
    mov ebx, dword ptr [edi]
    mov ecx, dword ptr [edi + 4]
    mov si, word ptr [edi + 8]
    rol esi, 0x10
    call "fn_000f952a_"
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], edx
    mov word ptr [edi + 8], si
    pop ecx
    pop esi
    pop edi
    ret
.size "fn_000fa77a_",.-"fn_000fa77a_"
