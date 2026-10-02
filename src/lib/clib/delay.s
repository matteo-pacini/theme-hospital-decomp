.intel_syntax noprefix

// __delay_init: calibrate delay(); count DOS get-time (2Ch) calls during one change of the seconds value.
// FUNCTION: 0x000eb701

.section _TEXT,"ax",@progbits,unique,964353
.globl "__delay_init_"
.type "__delay_init_",@function
"__delay_init_":
    push ebx
    push ecx
    push edx
    push esi
    mov ah, 0x2c
    int 0x21
    xor eax, eax
    mov al, dh
    mov ebx, eax
.L000eb70f:
    mov ah, 0x2c
    int 0x21
    xor eax, eax
    mov al, dh
    cmp ebx, eax
    je .L000eb70f
    xor esi, esi
    mov bl, al
.L000eb71f:
    mov ah, 0x2c
    int 0x21
    inc esi
    cmp bl, dh
    je .L000eb71f
    mov dword ptr [_D_0018cfa4], esi
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret
.size "__delay_init_",.-"__delay_init_"

// __delay: wait about ms milliseconds by repeating DOS get-time calls (calibrated count per second).
// FUNCTION: 0x000eb733

.section _TEXT,"ax",@progbits,unique,964403
.globl "__delay_"
.type "__delay_",@function
"__delay_":
    push ebx
    push ecx
    push edx
    push esi
    test eax, eax
    je .L000eb764
    imul eax, dword ptr [_D_0018cfa4]
    mov ebx, 0x3e8
    add eax, 0x1f4
    xor edx, edx
    div ebx
    test eax, eax
    jne .L000eb755
    inc eax
.L000eb755:
    mov esi, eax
    xor eax, eax
    mov bl, al
.L000eb75b:
    mov ah, 0x2c
    int 0x21
    cmp bl, dh
    dec esi
    jne .L000eb75b
.L000eb764:
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret
.size "__delay_",.-"__delay_"
