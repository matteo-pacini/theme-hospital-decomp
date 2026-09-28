.intel_syntax noprefix

// FUNCTION: 0x000cd192

.section _TEXT,"ax",@progbits,unique,840082
.globl "_dos_setvect_"
.type "_dos_setvect_",@function
"_dos_setvect_":
    push edx
    mov edx, ebx
    xor ebx, ebx
    mov bl, byte ptr ["__Extender"]
    cmp ebx, 2
    jl .L000cd1b5
    cmp ebx, 8
    jg .L000cd1b5
    push ds
    mov ds, ecx
    mov cl, al
    mov al, 4
    mov ah, 0x25
    int 0x21
    pop ds
    pop edx
    ret
.L000cd1b5:
    push ds
    mov ds, ecx
    mov ah, 0x25
    int 0x21
    pop ds
    pop edx
    ret
.size "_dos_setvect_",.-"_dos_setvect_"
