.intel_syntax noprefix

// FUNCTION: 0x000fa7c2

.section _TEXT,"ax",@progbits,unique,1025986
.globl "__hook387"
.type "__hook387",@function
"__hook387":
    push ebx
    push ecx
    push esi
    push edi
    push es
    push ebp
    push edx
    mov ebp, eax
    mov ax, 0x1600
    int 0x2f
    test al, al
    je .L000fa832
    and eax, 0xff
    cmp eax, 0x80
    je .L000fa832
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    xor al, al
    mov ah, 0xfa
    int 0x2f
    cwde
    cmp eax, 0x666
    jne .L000fa832
    mov al, 0x22
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    mov ah, 0xfa
    int 0x2f
    test ax, ax
    jne .L000fa832
    mov dh, 1
    mov al, 0x1e
    xor ebx, ebx
    xor ecx, ecx
    mov byte ptr ["_has_wgod_emu"], dh
    xor edx, edx
    mov ah, 0xfa
    int 0x2f
    mov eax, cs
    .byte 0xbb; .long _D_00195e6c  # mov ebx, 0x75e6c
    mov edx, eax
    mov ecx, ebx
    mov al, 0x20
    shr ecx, 0x10
    mov ah, 0xfa
    int 0x2f
    mov al, 1
    jmp .L000fa88c
.L000fa832:
    mov edx, dword ptr [esp]
    test ebp, ebp
    jne .L000fa83e
    test dx, dx
    je .L000fa88a
.L000fa83e:
    mov eax, 0xa00
    .byte 0xbe; .long .Lconst  # mov esi, 0x87f4
    push es
    int 0x31
    sbb eax, eax
    pop es
    test eax, eax
    jne .L000fa88a
    .byte 0xbb; .long "__int7_X32VM"+0x330  # mov ebx, 0xe7984
    mov eax, 7
    mov ecx, cs
    call "_dos_setvect_"
    mov es, word ptr [esp]
    mov ah, byte ptr es:[ebp + 0x40]
    mov byte ptr es:[ebp + 0x3e], 1
    or ah, 4
    mov byte ptr es:[ebp + 0x40], ah
    mov eax, cr0
    or al, 4
    mov dl, 1
    mov cr0, eax
    mov ah, 0x30
    int 0x21
    mov byte ptr ["_hooked"], dl
.L000fa88a:
    xor al, al
.L000fa88c:
    add esp, 4
    pop ebp
    pop es
    pop edi
    pop esi
    pop ecx
    pop ebx
    ret
.size "__hook387",.-"__hook387"

// FUNCTION: 0x000fa896

.section _TEXT,"ax",@progbits,unique,1026198
.globl "__unhook387"
.type "__unhook387",@function
"__unhook387":
    push ebx
    push ecx
    push es
    mov es, edx
    mov dl, byte ptr ["_has_wgod_emu"]
    test dl, dl
    je .L000fa8c5
    mov eax, cs
    mov edx, eax
    xor ebx, ebx
    xor ecx, ecx
    mov al, 0x21
    mov ah, 0xfa
    int 0x2f
    mov al, 0x1f
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    mov ah, 0xfa
    int 0x2f
    mov al, 1
    pop es
    pop ecx
    pop ebx
    ret
.L000fa8c5:
    cmp byte ptr ["_hooked"], 0
    je .L000fa8e9
    mov bl, byte ptr es:[eax + 0x40]
    mov byte ptr es:[eax + 0x3e], dl
    and bl, 0xfb
    mov byte ptr es:[eax + 0x40], bl
    mov eax, cr0
    and al, 0xfb
    mov cr0, eax
    mov ah, 0x30
    int 0x21
.L000fa8e9:
    xor al, al
    pop es
    pop ecx
    pop ebx
    ret
.L000fa8ef:
    .byte 0xc3
.size "__unhook387",.-"__unhook387"

// FUNCTION: 0x000fa8f0

.section _TEXT,"ax",@progbits,unique,1026288
.globl "fn_000fa8f0_"
.type "fn_000fa8f0_",@function
"fn_000fa8f0_":
    push ax
    wait
    fninit
    fld1
    fldz
    fdivp st(1)
    fld st(0)
    fchs
    fcompp
    fnstsw ax
    mov al, 2
    sahf
    je .L000fa90a
    mov al, 3
.L000fa90a:
    wait
    fninit
    fldcw word ptr [esp]
    xchg word ptr [esp], ax
    pop ax
    ret
.size "fn_000fa8f0_",.-"fn_000fa8f0_"

.section CONST,"aw",@progbits
.Lconst:
    .asciz "RATIONAL DOS/4G"

// save8087's hooks for saving and restoring the 80x87 state: its
// do-nothing routine (0xfa8ef) until fn_000ef30c installs the real ones
// when a coprocessor is present.
.section _DATA,"aw",@progbits
.globl "___Save8087"
"___Save8087":
    .long .L000fa8ef
.globl "___Rest8087"
"___Rest8087":
    .long .L000fa8ef
