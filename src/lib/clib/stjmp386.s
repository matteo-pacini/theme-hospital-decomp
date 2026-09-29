.intel_syntax noprefix

// FUNCTION: 0x000fb880

.section _TEXT,"ax",@progbits,unique,1030272
.globl "__wcpp_2_call_unexpected__"
.type "__wcpp_2_call_unexpected__",@function
"__wcpp_2_call_unexpected__":
    jmp "fn_000fc770_"
.size "__wcpp_2_call_unexpected__",.-"__wcpp_2_call_unexpected__"

// FUNCTION: 0x000fb886

.section _TEXT,"ax",@progbits,unique,1030278
.globl "_setjmp_"
.type "_setjmp_",@function
"_setjmp_":
    mov dword ptr [eax], ebx
    mov dword ptr [eax + 4], ecx
    mov dword ptr [eax + 8], edx
    mov dword ptr [eax + 0xc], esi
    mov dword ptr [eax + 0x10], edi
    mov dword ptr [eax + 0x14], ebp
    pop dword ptr [eax + 0x18]
    mov dword ptr [eax + 0x1c], esp
    push dword ptr [eax + 0x18]
    .byte 0x66; .byte 0x8c; .byte 0x40; .byte 0x20  # mov word ptr [eax + 0x20], es
    .byte 0x66; .byte 0x8c; .byte 0x58; .byte 0x22  # mov word ptr [eax + 0x22], ds
    .byte 0x66; .byte 0x8c; .byte 0x48; .byte 0x24  # mov word ptr [eax + 0x24], cs
    .byte 0x66; .byte 0x8c; .byte 0x60; .byte 0x26  # mov word ptr [eax + 0x26], fs
    .byte 0x66; .byte 0x8c; .byte 0x68; .byte 0x28  # mov word ptr [eax + 0x28], gs
    .byte 0x66; .byte 0x8c; .byte 0x50; .byte 0x2a  # mov word ptr [eax + 0x2a], ss
    sub eax, eax
    ret
.size "_setjmp_",.-"_setjmp_"

// FUNCTION: 0x000fb8bb

.section _TEXT,"ax",@progbits,unique,1030331
.globl "longjmp_"
.type "longjmp_",@function
"longjmp_":
    push edx
    push eax
    mov ebp, esp
    push eax
    push edx
    mov dx, word ptr [eax + 0x2a]
    mov eax, dword ptr [eax + 0x1c]
    call dword ptr [___longjmp_handler]
    pop edx
    pop eax
    mov ss, word ptr [eax + 0x2a]
    mov esp, dword ptr [eax + 0x1c]
    push dword ptr [eax + 0x18]
    or edx, edx
    jne .L000fb8de
    inc edx
.L000fb8de:
    push edx
    mov ebx, dword ptr [eax]
    mov ecx, dword ptr [eax + 4]
    mov esi, dword ptr [eax + 0xc]
    mov edi, dword ptr [eax + 0x10]
    mov ebp, dword ptr [eax + 0x14]
    mov dx, word ptr [eax + 0x20]
    verr dx
    je .L000fb8f8
    sub edx, edx
.L000fb8f8:
    mov es, edx
    mov dx, word ptr [eax + 0x26]
    verr dx
    je .L000fb905
    sub edx, edx
.L000fb905:
    mov fs, edx
    mov dx, word ptr [eax + 0x28]
    verr dx
    je .L000fb912
    sub edx, edx
.L000fb912:
    mov gs, edx
    mov edx, dword ptr [eax + 8]
    mov ds, word ptr [eax + 0x22]
    pop eax
    ret
.size "longjmp_",.-"longjmp_"
