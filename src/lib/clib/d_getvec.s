.intel_syntax noprefix

// FUNCTION: 0x000cd160

.section _TEXT,"ax",@progbits,unique,840032
.globl "_dos_getvect_"
.type "_dos_getvect_",@function
"_dos_getvect_":
    push ebx
    push ecx
    xor edx, edx
    mov dl, byte ptr ["__Extender"]
    cmp edx, 2
    jl .L000cd17d
    cmp edx, 8
    jg .L000cd17d
    mov cl, al
    mov eax, 0x2502
    jmp .L000cd187
.L000cd17d:
    and eax, 0xff
    xor cl, cl
    or ah, 0x35
.L000cd187:
    push es
    int 0x21
    mov edx, es
    pop es
    mov eax, ebx
    pop ecx
    pop ebx
    ret
.size "_dos_getvect_",.-"_dos_getvect_"
