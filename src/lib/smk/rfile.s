.intel_syntax noprefix

// FUNCTION: 0x000f2fe0

.section _TEXT,"ax",@progbits,unique,995296
.globl "_radopen"
.type "_radopen",@function
"_radopen":
    push ebp
    {load} mov ebp, esp
    push ds
    push ebx
    push edx
    mov ds, word ptr [ebp + 0xc]
    mov ah, 0x3d
    mov al, byte ptr [ebp + 0x10]
    mov edx, dword ptr [ebp + 8]
    int 0x21
    jae .L000f2ff9
    mov ax, 0xffff
.L000f2ff9:
    movsx eax, ax
    pop edx
    pop ebx
    pop ds
    pop ebp
    ret
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x1e
    .byte 0x53
    .byte 0x52
    .byte 0x8e; .byte 0x5d; .byte 0x0c
    .byte 0xb4; .byte 0x41
    .byte 0x8a; .byte 0x45; .byte 0x10
    .byte 0x8b; .byte 0x55; .byte 0x08
    .byte 0xcd; .byte 0x21
    .byte 0x5a
    .byte 0x5b
    .byte 0x1f
    .byte 0x5d
    .byte 0xc3
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x1e
    .byte 0x53
    .byte 0x52
    .byte 0x8e; .byte 0x5d; .byte 0x0c
    .byte 0xb4; .byte 0x3c
    .byte 0x8b; .byte 0x55; .byte 0x08
    .byte 0x33; .byte 0xc9
    .byte 0xcd; .byte 0x21
    .byte 0x73; .byte .L000f3031-.-1
    .byte 0x66; .byte 0xb8; .byte 0xff; .byte 0xff
.L000f3031:
    .byte 0x0f; .byte 0xbf; .byte 0xc0
    .byte 0x5a
    .byte 0x5b
    .byte 0x1f
    .byte 0x5d
    .byte 0xc3
.size "_radopen",.-"_radopen"

// FUNCTION: 0x000f3039

.section _TEXT,"ax",@progbits,unique,995385
.globl "_radread"
.type "_radread",@function
"_radread":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    push ds
    mov ds, word ptr [ebp + 0x10]
    mov edx, dword ptr [ebp + 0xc]
.L000f3046:
    mov ah, 0x3f
    mov ecx, dword ptr [ebp + 0x14]
    cmp ecx, 0xffff
    jbe .L000f3058
    mov ecx, 0xf000
.L000f3058:
    mov ebx, dword ptr [ebp + 8]
    push ecx
    push edx
    int 0x21
    pop edx
    pop ecx
    jb .L000f3075
    and eax, 0xffff
    je .L000f3075
    {load} add edx, eax
    {load} cmp eax, ecx
    jb .L000f3075
    sub dword ptr [ebp + 0x14], eax
    jne .L000f3046
.L000f3075:
    sub edx, dword ptr [ebp + 0xc]
    {load} mov eax, edx
    mov edx, dword ptr [ebp + 0x18]
    pop ds
    mov dword ptr [edx], eax
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x53
    .byte 0x51
    .byte 0x52
    .byte 0x1e
    .byte 0x8e; .byte 0x5d; .byte 0x10
    .byte 0x8b; .byte 0x55; .byte 0x0c
.L000f3092:
    .byte 0xb4; .byte 0x40
    .byte 0x8b; .byte 0x4d; .byte 0x14
    .byte 0x81; .byte 0xf9; .byte 0xff; .byte 0xff; .byte 0x00; .byte 0x00
    .byte 0x76; .byte .L000f30a4-.-1
    .byte 0xb9; .byte 0x00; .byte 0xf0; .byte 0x00; .byte 0x00
.L000f30a4:
    .byte 0x8b; .byte 0x5d; .byte 0x08
    .byte 0x51
    .byte 0x52
    .byte 0xcd; .byte 0x21
    .byte 0x5a
    .byte 0x59
    .byte 0x72; .byte .L000f30c1-.-1
    .byte 0x25; .byte 0xff; .byte 0xff; .byte 0x00; .byte 0x00
    .byte 0x74; .byte .L000f30c1-.-1
    .byte 0x03; .byte 0xd0
    .byte 0x3b; .byte 0xc1
    .byte 0x72; .byte .L000f30c1-.-1
    .byte 0x29; .byte 0x45; .byte 0x14
    .byte 0x75; .byte .L000f3092-.-1
.L000f30c1:
    .byte 0x2b; .byte 0x55; .byte 0x0c
    .byte 0x8b; .byte 0xc2
    .byte 0x8b; .byte 0x55; .byte 0x18
    .byte 0x1f
    .byte 0x89; .byte 0x02
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0x5d
    .byte 0xc3
.size "_radread",.-"_radread"

// FUNCTION: 0x000f30d1

.section _TEXT,"ax",@progbits,unique,995537
.globl "_radseekbegin"
.type "_radseekbegin",@function
"_radseekbegin":
    mov ax, 0x4200
.size "_radseekbegin",.-"_radseekbegin"

// FUNCTION: 0x000f30d5

.section _TEXT,"ax",@progbits,unique,995541
.globl "_radseekcur"
.type "_radseekcur",@function
"_radseekcur":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    mov ebx, dword ptr [ebp + 8]
    movzx edx, word ptr [ebp + 0xc]
    movzx ecx, word ptr [ebp + 0xe]
    int 0x21
    shl edx, 0x10
    {load} mov dx, ax
    {load} mov eax, edx
    pop ecx
    pop edx
    pop ebx
    pop ebp
    ret
.size "_radseekcur",.-"_radseekcur"

// FUNCTION: 0x000f30f5

.section _TEXT,"ax",@progbits,unique,995573
.globl "_radseekend"
.type "_radseekend",@function
"_radseekend":
    mov ax, 0x4201
    .byte 0xeb; .byte 0xda  # jmp 0xf30d5
    .byte 0x66; .byte 0xb8; .byte 0x02; .byte 0x42
    .byte 0xeb; .byte 0xd4
.size "_radseekend",.-"_radseekend"

// FUNCTION: 0x000f3101

.section _TEXT,"ax",@progbits,unique,995585
.globl "_radclose"
.type "_radclose",@function
"_radclose":
    push ebp
    {load} mov ebp, esp
    push ebx
    mov ah, 0x3e
    mov ebx, dword ptr [ebp + 8]
    int 0x21
    pop ebx
    pop ebp
    ret
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x53
    .byte 0x52
    .byte 0x66; .byte 0xb8; .byte 0x0a; .byte 0x44
    .byte 0x8b; .byte 0x5d; .byte 0x08
    .byte 0x66; .byte 0x33; .byte 0xd2
    .byte 0xcd; .byte 0x21
    .byte 0x66; .byte 0x8b; .byte 0xc2
    .byte 0x66; .byte 0xc1; .byte 0xe8; .byte 0x0f
    .byte 0x24; .byte 0x01
    .byte 0x5a
    .byte 0x5b
    .byte 0x5d
    .byte 0xc3
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x53
    .byte 0x51
    .byte 0x66; .byte 0xb8; .byte 0x00; .byte 0x57
    .byte 0x8b; .byte 0x5d; .byte 0x08
    .byte 0xcd; .byte 0x21
    .byte 0x66; .byte 0x8b; .byte 0xc1
    .byte 0xc1; .byte 0xe0; .byte 0x10
    .byte 0x66; .byte 0x8b; .byte 0xc2
    .byte 0x59
    .byte 0x5b
    .byte 0x5d
    .byte 0xc3
    .byte 0x55
    .byte 0x8b; .byte 0xec
    .byte 0x53
    .byte 0x51
    .byte 0x52
    .byte 0x8b; .byte 0x55; .byte 0x0c
    .byte 0x8b; .byte 0xca
    .byte 0xc1; .byte 0xe9; .byte 0x10
    .byte 0x81; .byte 0xe2; .byte 0xff; .byte 0xff; .byte 0x00; .byte 0x00
    .byte 0x66; .byte 0xb8; .byte 0x01; .byte 0x57
    .byte 0x8b; .byte 0x5d; .byte 0x08
    .byte 0xcd; .byte 0x21
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0x5d
    .byte 0xc3
.size "_radclose",.-"_radclose"
