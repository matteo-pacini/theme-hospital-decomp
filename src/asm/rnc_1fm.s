.intel_syntax noprefix

// FUNCTION: 0x000d0108

.section _TEXT,"ax",@progbits,unique,852232
.globl "UnpackM1_"
.type "UnpackM1_",@function
"UnpackM1_":
    push ebp
    {load} mov ebp, esp
    add esp, -8
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ds
    push es
    mov dword ptr [ebp - 4], eax
    mov dword ptr [ebp - 8], edx
    cld
    push ds
    pop es
    mov esi, dword ptr [ebp - 4]
    lodsw ax, word ptr [esi]
    cmp ax, 0x4e52
    jne .L000d0288
    lodsw ax, word ptr [esi]
    cmp ax, 0x143
    jne .L000d0288
    call "read_long"
    mov dword ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x18c], eax
    call "read_long"
    mov dword ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x190], eax
    mov bl, byte ptr [esi + 5]
    mov byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19c], bl
    add esi, 6
    add eax, 0x12
    mov edx, dword ptr [ebp - 4]
    mov ebx, dword ptr [ebp - 8]
    {load} add edx, eax
    {load} cmp edx, ebx
    jbe .L000d01af
    mov edi, dword ptr [ebp - 4]
    {load} xor eax, eax
    mov al, byte ptr [edi + 0x10]
    add eax, dword ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x18c]
    {load} add ebx, eax
    {load} cmp ebx, edx
    jbe .L000d01af
    {load} mov esi, edx
    {load} mov edi, ebx
    sub esi, 4
    sub edi, 4
    mov ecx, dword ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x190]
    shr ecx, 2
    std
    rep movsd dword ptr es:[edi], dword ptr [esi]
    add esi, 4
    add edi, 4
    mov cx, word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x190]
    and cx, 3
    .byte 0x67; .byte 0xe3; .byte 0x06  # jcxz 0xd01ac
    dec esi
    dec edi
    rep movsb byte ptr es:[edi], byte ptr [esi]
    inc esi
    inc edi
    cld
    {load} mov esi, edi
.L000d01af:
    mov edi, dword ptr [ebp - 8]
    mov byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19d], 0
    mov ax, word ptr [esi]
    mov word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198], ax
    mov al, 2
    call "input_bits"
.L000d01c9:
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0xc  # mov edx, 0x305f8
    call "make_huftable"
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x8c  # mov edx, 0x30678
    call "make_huftable"
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x10c  # mov edx, 0x306f8
    call "make_huftable"
    mov al, 0x10
    call "input_bits"
    mov word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x196], ax
    jmp .L000d0220
.L000d01f6:
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x8c  # mov edx, 0x30678
    call "input_value"
    push cx
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x10c  # mov edx, 0x306f8
    call "input_value"
    add cx, 2
    {load} xor eax, eax
    pop ax
    inc ax
    {load} mov edx, esi
    {load} mov esi, edi
    {load} sub esi, eax
    rep movsb byte ptr es:[edi], byte ptr [esi]
    {load} mov esi, edx
.L000d0220:
    .byte 0xba; .long "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0xc  # mov edx, 0x305f8
    call "input_value"
    .byte 0x67; .byte 0xe3; .byte 0x3e  # jcxz 0xd026b
    rep movsb byte ptr es:[edi], byte ptr [esi]
    mov cl, byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19d]
    mov ax, word ptr [esi]
    {load} mov bx, ax
    rol ax, cl
    mov dx, 1
    shl dx, cl
    dec dx
    and word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198], dx
    {load} and dx, ax
    mov ax, word ptr [esi + 2]
    shl bx, cl
    shl ax, cl
    {load} or ax, dx
    or word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198], bx
    mov word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19a], ax
    dec word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x196]
    jne .L000d01f6
    dec byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19c]
    jne .L000d01c9
    mov eax, dword ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x18c]
    clc
    jmp .L000d029c
.L000d0288:
    mov eax, 0
    jmp .L000d029b
    .byte 0xb8; .byte 0xff; .byte 0xff; .byte 0xff; .byte 0xff
    .byte 0xeb; .byte .L000d029b-.-1
    .byte 0xb8; .byte 0xfe; .byte 0xff; .byte 0xff; .byte 0xff
.L000d029b:
    stc
.L000d029c:
    pop es
    pop ds
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret
.size "UnpackM1_",.-"UnpackM1_"

// FUNCTION: 0x000d02a5

.section _TEXT,"ax",@progbits,unique,852645
.globl "read_long"
.type "read_long",@function
"read_long":
    lodsd eax, dword ptr [esi]
    xchg al, ah
    rol eax, 0x10
    xchg al, ah
    ret
.size "read_long",.-"read_long"

// FUNCTION: 0x000d02ae

.section _TEXT,"ax",@progbits,unique,852654
.globl "input_value"
.type "input_value",@function
"input_value":
    xchg esi, edx
    mov cx, word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198]
.L000d02b7:
    lodsw ax, word ptr [esi]
    {load} mov bx, ax
    {load} and bx, cx
    lodsw ax, word ptr [esi]
    {load} cmp ax, bx
    jne .L000d02b7
    mov cx, word ptr [esi + 0x3c]
    xchg esi, edx
    {load} mov al, ch
    call "input_bits"
    {load} xor ch, ch
    cmp cl, 2
    jb .L000d02f0
    dec cl
    {load} mov al, cl
    call "input_bits"
    mov bx, 1
    shl bx, cl
    {load} or ax, bx
    {load} mov cx, ax
.L000d02f0:
    ret
.size "input_value",.-"input_value"

// FUNCTION: 0x000d02f1

.section _TEXT,"ax",@progbits,unique,852721
.globl "input_bits"
.type "input_bits",@function
"input_bits":
    push cx
    {load} mov cl, al
    mov ax, word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19a]
    mov bx, word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198]
    mov ch, byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19d]
    mov dx, 1
    shl dx, cl
    dec dx
    {load} and dx, bx
    push dx
    {load} sub ch, cl
    jae .L000d0344
    {load} add ch, cl
    xchg ch, cl
    mov dx, 1
    shl dx, cl
    dec dx
    {load} and dx, ax
    ror dx, cl
    shr ax, cl
    shr bx, cl
    {load} or bx, dx
    add esi, 2
    mov ax, word ptr [esi]
    xchg ch, cl
    {load} sub cl, ch
    mov ch, 0x10
    {load} sub ch, cl
.L000d0344:
    mov dx, 1
    shl dx, cl
    dec dx
    {load} and dx, ax
    ror dx, cl
    shr ax, cl
    shr bx, cl
    {load} or bx, dx
    mov word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19a], ax
    mov word ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x198], bx
    mov byte ptr ["W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"+0x19d], ch
    pop ax
    pop cx
    ret
.size "input_bits",.-"input_bits"

// FUNCTION: 0x000d0374

.section _TEXT,"ax",@progbits,unique,852852
.globl "make_huftable"
.type "make_huftable",@function
"make_huftable":
    push edi
    push edx
    sub esp, 0x10
    {load} mov edi, esp
    mov al, 5
    call "input_bits"
    {load} xor ecx, ecx
    {load} mov cx, ax
    .byte 0x67; .byte 0xe3; .byte 0x7d  # jcxz 0xd0407
    push ecx
    mov al, 4
    call "input_bits"
    mov byte ptr ss:[edi], al
    inc edi
    .byte 0xe2; .byte 0xf3  # loop 0xd038b
    pop ecx
    push esi
    {load} mov esi, esp
    add esi, 4
    mov edi, dword ptr ss:[esi + 0x10]
    mov al, 1
    {load} xor bx, bx
    mov dx, 0x8000
.L000d03ac:
    push cx
    push esi
    cmp al, byte ptr ss:[esi]
    jne .L000d03f7
    push ax
    push bx
    push cx
    {load} mov cl, al
    mov ax, 1
    shl ax, cl
    dec ax
    stosw word ptr es:[edi], ax
    {load} mov al, cl
    mov cl, 0x10
    {load} sub cl, al
    shr bx, cl
    {load} mov cl, al
    {load} xor ax, ax
    rcr bx, 1
    rcl ax, 1
    .byte 0xe2; .byte 0xf8  # loop 0xd03d5
    stosw word ptr es:[edi], ax
    {load} mov eax, esi
    {load} sub eax, esp
    sub ax, 0x10
    mov ah, byte ptr ss:[esi]
    mov word ptr [edi + 0x3c], ax
    pop cx
    pop bx
    pop ax
    {load} add bx, dx
.L000d03f7:
    inc esi
    .byte 0xe2; .byte 0xb5  # loop 0xd03af
    pop esi
    pop cx
    shr dx, 1
    inc al
    cmp al, 0x11
    jne .L000d03ac
    pop esi
    add esp, 0x10
    pop edx
    pop edi
    ret
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x55; .byte 0x89
    .byte 0xe5; .byte 0x83
    .byte 0xec
    .byte 0x04; .byte 0x89
    .byte 0x45
    .byte 0xfc
    .byte 0x8b; .byte 0x45; .byte 0xfc
    .byte 0xe8; .long "chdir_"-.-4
    .byte 0x85; .byte 0xc0
    .byte 0x75; .byte .L000d042e-.-1
    .byte 0xb8; .byte 0x01; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x89; .byte 0xec
    .byte 0x5d
    .byte 0xc3
.L000d042e:
    .byte 0xb8; .byte 0xff; .byte 0xff; .byte 0xff; .byte 0xff
    .byte 0x89; .byte 0xec
    .byte 0x5d
    .byte 0xc3
    .byte 0x90
    .byte 0x55
    .byte 0x89; .byte 0xe5
    .byte 0x83; .byte 0xec; .byte 0x04
    .byte 0x89; .byte 0x45; .byte 0xfc
    .byte 0x8b; .byte 0x45; .byte 0xfc
    .byte 0xe8; .long "mkdir_"-.-4
    .byte 0x85; .byte 0xc0
    .byte 0x75; .byte .L000d0456-.-1
    .byte 0xb8; .byte 0x01; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x89; .byte 0xec
    .byte 0x5d
    .byte 0xc3
.L000d0456:
    .byte 0xb8; .byte 0xff; .byte 0xff; .byte 0xff; .byte 0xff
    .byte 0x89; .byte 0xec
    .byte 0x5d
    .byte 0xc3
.size "make_huftable",.-"make_huftable"
    // The original object ends with a nop that pads it to a dword.
    .byte 0x90
