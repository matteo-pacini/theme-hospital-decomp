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
.size "_radopen",.-"_radopen"

// FUNCTION: 0x000f3001

.section _TEXT,"ax",@progbits,unique,995329
.globl "_raderase"
.type "_raderase",@function
"_raderase":
    push ebp
    {load} mov ebp, esp
    push ds
    push ebx
    push edx
    mov ds, word ptr [ebp + 0xc]
    mov ah, 0x41
    mov al, byte ptr [ebp + 0x10]
    mov edx, dword ptr [ebp + 8]
    int 0x21
    pop edx
    pop ebx
    pop ds
    pop ebp
    ret
.size "_raderase",.-"_raderase"

// FUNCTION: 0x000f3019

.section _TEXT,"ax",@progbits,unique,995353
.globl "_radcreate"
.type "_radcreate",@function
"_radcreate":
    push ebp
    {load} mov ebp, esp
    push ds
    push ebx
    push edx
    mov ds, word ptr [ebp + 0xc]
    mov ah, 0x3c
    mov edx, dword ptr [ebp + 8]
    {load} xor ecx, ecx
    int 0x21
    jae .L000f3031
    mov ax, 0xffff
.L000f3031:
    movsx eax, ax
    pop edx
    pop ebx
    pop ds
    pop ebp
    ret
.size "_radcreate",.-"_radcreate"

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
.size "_radread",.-"_radread"

// FUNCTION: 0x000f3085

.section _TEXT,"ax",@progbits,unique,995461
.globl "_radwrite"
.type "_radwrite",@function
"_radwrite":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    push ds
    mov ds, word ptr [ebp + 0x10]
    mov edx, dword ptr [ebp + 0xc]
.L000f3092:
    mov ah, 0x40
    mov ecx, dword ptr [ebp + 0x14]
    cmp ecx, 0xffff
    jbe .L000f30a4
    mov ecx, 0xf000
.L000f30a4:
    mov ebx, dword ptr [ebp + 8]
    push ecx
    push edx
    int 0x21
    pop edx
    pop ecx
    jb .L000f30c1
    and eax, 0xffff
    je .L000f30c1
    {load} add edx, eax
    {load} cmp eax, ecx
    jb .L000f30c1
    sub dword ptr [ebp + 0x14], eax
    jne .L000f3092
.L000f30c1:
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
.size "_radwrite",.-"_radwrite"

// FUNCTION: 0x000f30d1

.section _TEXT,"ax",@progbits,unique,995537
.globl "_radseekbegin"
.type "_radseekbegin",@function
"_radseekbegin":
    mov ax, 0x4200
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
.size "_radseekbegin",.-"_radseekbegin"

// FUNCTION: 0x000f30f5

.section _TEXT,"ax",@progbits,unique,995573
.globl "_radseekcur"
.type "_radseekcur",@function
"_radseekcur":
    mov ax, 0x4201
    .byte 0xeb; .byte 0xda  # jmp 0xf30d5
.size "_radseekcur",.-"_radseekcur"

// FUNCTION: 0x000f30fb

.section _TEXT,"ax",@progbits,unique,995579
.globl "_radseekend"
.type "_radseekend",@function
"_radseekend":
    mov ax, 0x4202
    .byte 0xeb; .byte 0xd4  # jmp 0xf30d5
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
.size "_radclose",.-"_radclose"

// FUNCTION: 0x000f310f

.section _TEXT,"ax",@progbits,unique,995599
.globl "_IsRemote"
.type "_IsRemote",@function
"_IsRemote":
    push ebp
    {load} mov ebp, esp
    push ebx
    push edx
    mov ax, 0x440a
    mov ebx, dword ptr [ebp + 8]
    {load} xor dx, dx
    int 0x21
    {load} mov ax, dx
    shr ax, 0xf
    and al, 1
    pop edx
    pop ebx
    pop ebp
    ret
.size "_IsRemote",.-"_IsRemote"

// FUNCTION: 0x000f312d

.section _TEXT,"ax",@progbits,unique,995629
.globl "_radgetfiletime"
.type "_radgetfiletime",@function
"_radgetfiletime":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    mov ax, 0x5700
    mov ebx, dword ptr [ebp + 8]
    int 0x21
    {load} mov ax, cx
    shl eax, 0x10
    {load} mov ax, dx
    pop ecx
    pop ebx
    pop ebp
    ret
.size "_radgetfiletime",.-"_radgetfiletime"

// FUNCTION: 0x000f3148

.section _TEXT,"ax",@progbits,unique,995656
.globl "_radsetfiletime"
.type "_radsetfiletime",@function
"_radsetfiletime":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    mov edx, dword ptr [ebp + 0xc]
    {load} mov ecx, edx
    shr ecx, 0x10
    and edx, 0xffff
    mov ax, 0x5701
    mov ebx, dword ptr [ebp + 8]
    int 0x21
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret
.size "_radsetfiletime",.-"_radsetfiletime"
