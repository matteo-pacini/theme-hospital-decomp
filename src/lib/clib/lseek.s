.intel_syntax noprefix

// FUNCTION: 0x000d0ed8

.section _TEXT,"ax",@progbits,unique,855768
.globl "lseek_"
.type "lseek_",@function
"lseek_":
    push ecx
    push edi
    sub esp, 4
    mov ecx, eax
    mov edi, esp
    mov al, bl
    mov ebx, ecx
    mov ah, 0x42
    mov ecx, edx
    shr ecx, 0x10
    int 0x21
    .byte 0x66; .byte 0x36; .byte 0x89; .byte 0x07  # mov word ptr ss:[edi], ax
    .byte 0x66; .byte 0x36; .byte 0x89; .byte 0x57; .byte 0x02  # mov word ptr ss:[edi + 2], dx
    rcl eax, 1
    ror eax, 1
    mov edx, eax
    test eax, eax
    jge .L000d0f12
    xor eax, eax
    mov ax, dx
    call "__set_errno_dos_"
    mov eax, 0xffffffff
    jmp .L000d0f15
.L000d0f12:
    mov eax, dword ptr [esp]
.L000d0f15:
    add esp, 4
    pop edi
    pop ecx
    ret
.size "lseek_",.-"lseek_"
