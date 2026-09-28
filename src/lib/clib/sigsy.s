.intel_syntax noprefix

// FUNCTION: 0x000f4bca

.section _TEXT,"ax",@progbits,unique,1002442
.globl "fn_000f4bca_"
.type "fn_000f4bca_",@function
"fn_000f4bca_":
    .byte 0x60  # pushal
    push ds
    push es
    push fs
    push gs
    mov ebp, esp
    cld
    call "__GETDS"
    .byte 0xb8; .long "W?com_dev$n[]$ComHandlerInfo$$"+0xbf0  # mov eax, 0x6e690
    mov edx, ds
    mov bx, ss
    mov ecx, esp
    mov ss, edx
    mov esp, eax
    mov dx, bx
    mov eax, ecx
    mov esi, dword ptr ["__STACKLOW"]
    mov edi, dword ptr [_D_001507b8]
    mov ecx, edx
    .byte 0xba; .long _D_0018d6a0  # mov edx, 0x6d6a0
    mov ebx, eax
    mov dword ptr ["__STACKLOW"], edx
    test edi, edi
    jne .L000f4c2e
    mov eax, dword ptr ["___int23_exit"]
    .byte 0x3d; .long "__null_int23_exit_"  # cmp eax, 0xb2110
    je .L000f4c2e
    .byte 0x3d; .long "__restore_int_ctrl_break_"  # cmp eax, 0xe4f21
    je .L000f4c2e
    mov eax, 4
    sti
    cld
    call "fn_000fbbef_"
    jmp .L000f4c3f
.L000f4c2e:
    mov dx, word ptr ["___old_int23"+0x4]
    mov eax, dword ptr ["___old_int23"]
    call "fn_000fbcb2_"
.L000f4c3f:
    mov edx, ecx
    mov eax, ebx
    mov bx, ss
    mov ecx, esp
    mov ss, edx
    mov esp, eax
    mov dx, bx
    mov eax, ecx
    mov dword ptr ["__STACKLOW"], esi
    pop gs
    pop fs
    pop es
    pop ds
    .byte 0x61  # popal
    iretd
.size "fn_000f4bca_",.-"fn_000f4bca_"

// FUNCTION: 0x000f4c5f

.section _TEXT,"ax",@progbits,unique,1002591
.globl "fn_000f4c5f_"
.type "fn_000f4c5f_",@function
"fn_000f4c5f_":
    .byte 0x60  # pushal
    push ds
    push es
    push fs
    push gs
    mov ebp, esp
    cld
    call "__GETDS"
    .byte 0xb8; .long "W?com_dev$n[]$ComHandlerInfo$$"+0xbf0  # mov eax, 0x6e690
    mov edx, ds
    mov bx, ss
    mov ecx, esp
    mov ss, edx
    mov esp, eax
    mov dx, bx
    mov eax, ecx
    mov esi, dword ptr ["__STACKLOW"]
    mov edi, dword ptr [_D_001507b8]
    mov ecx, edx
    .byte 0xba; .long _D_0018d6a0  # mov edx, 0x6d6a0
    mov ebx, eax
    mov dword ptr ["__STACKLOW"], edx
    test edi, edi
    jne .L000f4cc6
    mov eax, dword ptr ["___int23_exit"]
    .byte 0x3d; .long "__null_int23_exit_"  # cmp eax, 0xb2110
    je .L000f4cc6
    .byte 0x3d; .long "__restore_int23_"  # cmp eax, 0xe4d6b
    je .L000f4cc6
    mov eax, 7
    sti
    cld
    call "fn_000fbbef_"
    jmp "fn_000f4bca_"+0x75
.L000f4cc6:
    mov dx, word ptr ["___old_int_ctrl_break"+0x4]
    mov eax, dword ptr ["___old_int_ctrl_break"]
    jmp "fn_000f4bca_"+0x70
.size "fn_000f4c5f_",.-"fn_000f4c5f_"

// FUNCTION: 0x000f4cd7

.section _TEXT,"ax",@progbits,unique,1002711
.globl "__DPMI_hosted_"
.type "__DPMI_hosted_",@function
"__DPMI_hosted_":
    push ebx
    push ecx
    push edx
    push esi
    sub esp, 8
    cmp dword ptr ["___old_pm_int_ctrl_break"+0x6], 0
    .byte 0x0f; .byte 0x85; .long .L000f4d5e-.-4  # jne 0xf4d5e
    xor eax, eax
    mov al, byte ptr ["__Extender"]
    cmp eax, 1
    je .L000f4d02
    mov ax, 0x1686
    int 0x2f
    test ax, ax
    jne .L000f4d54
.L000f4d02:
    mov esi, esp
    xor ah, ah
    xor dl, dl
    mov byte ptr [esp], ah
    xor dh, dh
    mov byte ptr [esp + 1], ah
    mov word ptr [esp + 2], dx
    mov edx, ds
    mov byte ptr [esp + 4], ah
    push ds
    mov ds, edx
    mov ax, 0x400
    int 0x31
    mov byte ptr [esi], ah
    mov byte ptr [esi + 1], al
    mov word ptr [esi + 2], bx
    mov byte ptr [esi + 4], cl
    mov byte ptr [esi + 5], dh
    mov byte ptr [esi + 6], dl
    pop ds
    xor eax, eax
    movzx esi, byte ptr [esp + 1]
    mov al, byte ptr [esp]
    add eax, esi
    test eax, eax
    jle .L000f4d54
    mov dword ptr ["___old_pm_int_ctrl_break"+0x6], 1
    jmp .L000f4d5e
.L000f4d54:
    mov dword ptr ["___old_pm_int_ctrl_break"+0x6], 0xffffffff
.L000f4d5e:
    mov eax, dword ptr ["___old_pm_int_ctrl_break"+0x6]
    add esp, 8
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret
.size "__DPMI_hosted_",.-"__DPMI_hosted_"

// FUNCTION: 0x000f4d6b

.section _TEXT,"ax",@progbits,unique,1002859
.globl "__restore_int23_"
.type "__restore_int23_",@function
"__restore_int23_":
    push ebx
    push ecx
    push edx
    push ebp
    cmp dword ptr ["___old_int23"], 0
    jne .L000f4d86
    cmp word ptr ["___old_int23"+0x4], 0
    je .L000f4e59
.L000f4d86:
    mov ebx, dword ptr ["___int23_exit"]
    .byte 0x81; .byte 0xfb; .long "__restore_int_"  # cmp ebx, 0xe4f1c
    jne .L000f4da0
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int_ctrl_break_"  # mov dword ptr [0x2db68], 0xe4f21
    jmp .L000f4db2
.L000f4da0:
    .byte 0x81; .byte 0xfb; .long "__restore_int23_"  # cmp ebx, 0xe4d6b
    jne .L000f4db2
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__null_int23_exit_"  # mov dword ptr [0x2db68], 0xb2110
.L000f4db2:
    xor eax, eax
    mov al, byte ptr ["__Extender"]
    cmp eax, 2
    jl .L000f4dfb
    cmp eax, 8
    jg .L000f4dfb
    mov cl, 0x23
    mov dx, word ptr ["___old_int23"+0x4]
    mov eax, dword ptr ["___old_int23"]
    mov ebx, eax
    shl ebx, 0x10
    mov bx, dx
    mov ax, 0x2505
    int 0x21
    mov cl, 0x23
    mov dx, word ptr ["___old_pm_int23"+0x4]
    mov eax, dword ptr ["___old_pm_int23"]
    push ds
    mov ds, edx
    mov edx, eax
    mov ax, 0x2504
    int 0x21
    pop ds
    jmp .L000f4e48
.L000f4dfb:
    call "__DPMI_hosted_"
    cmp eax, 1
    jne .L000f4e31
    mov bl, 0x23
    mov cx, word ptr ["___old_int23"+0x4]
    mov edx, dword ptr ["___old_int23"]
    mov ax, 0x201
    int 0x31
    mov bl, 0x23
    mov cx, word ptr ["___old_pm_int23"+0x4]
    mov edx, dword ptr ["___old_pm_int23"]
    mov ax, 0x205
    int 0x31
    jmp .L000f4e48
.L000f4e31:
    mov eax, 0x23
    mov cx, word ptr ["___old_int23"+0x4]
    mov ebx, dword ptr ["___old_int23"]
    call "_dos_setvect_"
.L000f4e48:
    xor ebx, ebx
    xor ebp, ebp
    mov word ptr ["___old_int23"+0x4], bx
    mov dword ptr ["___old_int23"], ebp
.L000f4e59:
    pop ebp
    pop edx
    pop ecx
    pop ebx
    ret
    .byte 0x81; .byte 0xfb; .long "__restore_int_ctrl_break_"
    .byte 0x75; .byte .L000f4e70-.-1
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__null_int23_exit_"
.L000f4e70:
    .byte 0x31; .byte 0xc0
    .byte 0xa0; .long "__Extender"
    .byte 0x83; .byte 0xf8; .byte 0x02
    .byte 0x7c; .byte .L000f4eb9-.-1
    .byte 0x83; .byte 0xf8; .byte 0x08
    .byte 0x7f; .byte .L000f4eb9-.-1
    .byte 0xb1; .byte 0x1b
    .byte 0x66; .byte 0x8b; .byte 0x15; .long "___old_int_ctrl_break"+0x4
    .byte 0xa1; .long "___old_int_ctrl_break"
    .byte 0x89; .byte 0xc3
    .byte 0xc1; .byte 0xe3; .byte 0x10
    .byte 0x66; .byte 0x89; .byte 0xd3
    .byte 0x66; .byte 0xb8; .byte 0x05; .byte 0x25
    .byte 0xcd; .byte 0x21
    .byte 0xb1; .byte 0x1b
    .byte 0x66; .byte 0x8b; .byte 0x15; .long "___old_pm_int_ctrl_break"+0x4
    .byte 0xa1; .long "___old_pm_int_ctrl_break"
    .byte 0x1e
    .byte 0x8e; .byte 0xda
    .byte 0x89; .byte 0xc2
    .byte 0x66; .byte 0xb8; .byte 0x04; .byte 0x25
    .byte 0xcd; .byte 0x21
    .byte 0x1f
    .byte 0xeb; .byte .L000f4f06-.-1
.L000f4eb9:
    .byte 0xe8; .long "__DPMI_hosted_"-.-4
    .byte 0x83; .byte 0xf8; .byte 0x01
    .byte 0x75; .byte .L000f4eef-.-1
    .byte 0xb3; .byte 0x1b
    .byte 0x66; .byte 0x8b; .byte 0x0d; .long "___old_int_ctrl_break"+0x4
    .byte 0x8b; .byte 0x15; .long "___old_int_ctrl_break"
    .byte 0x66; .byte 0xb8; .byte 0x01; .byte 0x02
    .byte 0xcd; .byte 0x31
    .byte 0xb3; .byte 0x1b
    .byte 0x66; .byte 0x8b; .byte 0x0d; .long "___old_pm_int_ctrl_break"+0x4
    .byte 0x8b; .byte 0x15; .long "___old_pm_int_ctrl_break"
    .byte 0x66; .byte 0xb8; .byte 0x05; .byte 0x02
    .byte 0xcd; .byte 0x31
    .byte 0xeb; .byte .L000f4f06-.-1
.L000f4eef:
    .byte 0xb8; .byte 0x1b; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x66; .byte 0x8b; .byte 0x0d; .long "___old_int_ctrl_break"+0x4
    .byte 0x8b; .byte 0x1d; .long "___old_int_ctrl_break"
    .byte 0xe8; .long "_dos_setvect_"-.-4
.L000f4f06:
    .byte 0x31; .byte 0xdb
    .byte 0x31; .byte 0xed
    .byte 0x66; .byte 0x89; .byte 0x1d; .long "___old_int_ctrl_break"+0x4
    .byte 0x89; .byte 0x2d; .long "___old_int_ctrl_break"
    .byte 0x5d
    .byte 0x5a
    .byte 0x59
    .byte 0x5b
    .byte 0xc3
.size "__restore_int23_",.-"__restore_int23_"

// FUNCTION: 0x000f4f1c

.section _TEXT,"ax",@progbits,unique,1003292
.globl "__restore_int_"
.type "__restore_int_",@function
"__restore_int_":
    call "__restore_int23_"
.size "__restore_int_",.-"__restore_int_"

// FUNCTION: 0x000f4f21

.section _TEXT,"ax",@progbits,unique,1003297
.globl "__restore_int_ctrl_break_"
.type "__restore_int_ctrl_break_",@function
"__restore_int_ctrl_break_":
    push ebx
    push ecx
    push edx
    push ebp
    cmp dword ptr ["___old_int_ctrl_break"], 0
    jne .L000f4f38
    cmp word ptr ["___old_int_ctrl_break"+0x4], 0
    .byte 0x74; .byte 0xdf  # je 0xf4f17
.L000f4f38:
    mov ebx, dword ptr ["___int23_exit"]
    .byte 0x81; .byte 0xfb; .long "__restore_int_"  # cmp ebx, 0xe4f1c
    jne "__restore_int23_"+0xf3
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int23_"  # mov dword ptr [0x2db68], 0xe4d6b
    jmp "__restore_int23_"+0x105
.size "__restore_int_ctrl_break_",.-"__restore_int_ctrl_break_"

// FUNCTION: 0x000f4f59

.section _TEXT,"ax",@progbits,unique,1003353
.globl "__grab_int23_"
.type "__grab_int23_",@function
"__grab_int23_":
    push ebx
    push ecx
    push edx
    cmp dword ptr ["___old_int23"], 0
    jne .L000f5070
    cmp word ptr ["___old_int23"+0x4], 0
    jne .L000f5070
    xor eax, eax
    mov al, byte ptr ["__Extender"]
    cmp eax, 2
    jl .L000f4fd6
    cmp eax, 8
    jg .L000f4fd6
    mov cl, 0x23
    mov ax, 0x2503
    int 0x21
    mov cx, bx
    shr ebx, 0x10
    mov edx, ebx
    mov word ptr ["___old_int23"+0x4], cx
    mov dword ptr ["___old_int23"], edx
    mov cl, 0x23
    .byte 0xba; .long "fn_000f4bca_"  # mov edx, 0xe4bca
    push es
    mov ax, 0x2502
    int 0x21
    mov cx, es
    pop es
    mov al, 0x23
    mov word ptr ["___old_pm_int23"+0x4], cx
    mov ecx, cs
    mov dword ptr ["___old_pm_int23"], ebx
    push ds
    mov ds, ecx
    mov cl, al
    mov al, 6
    mov ah, 0x25
    int 0x21
    pop ds
    jmp .L000f5042
.L000f4fd6:
    call "__DPMI_hosted_"
    cmp eax, 1
    jne .L000f501b
    mov bl, 0x23
    mov ax, 0x200
    int 0x31
    mov bl, 0x23
    mov word ptr ["___old_int23"+0x4], cx
    mov dword ptr ["___old_int23"], edx
    mov ax, 0x204
    int 0x31
    mov bl, 0x23
    mov word ptr ["___old_pm_int23"+0x4], cx
    mov dword ptr ["___old_pm_int23"], edx
    .byte 0xba; .long "fn_000f4bca_"  # mov edx, 0xe4bca
    mov ecx, cs
    mov ax, 0x205
    int 0x31
    jmp .L000f5042
.L000f501b:
    mov eax, 0x23
    .byte 0xbb; .long "fn_000f4bca_"  # mov ebx, 0xe4bca
    call "_dos_getvect_"
    mov ecx, cs
    mov dword ptr ["___old_int23"], eax
    mov eax, 0x23
    mov word ptr ["___old_int23"+0x4], dx
    call "_dos_setvect_"
.L000f5042:
    mov ebx, dword ptr ["___int23_exit"]
    .byte 0x81; .byte 0xfb; .long "__null_int23_exit_"  # cmp ebx, 0xb2110
    jne .L000f505e
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int23_"  # mov dword ptr [0x2db68], 0xe4d6b
    pop edx
    pop ecx
    pop ebx
    ret
.L000f505e:
    .byte 0x81; .byte 0xfb; .long "__restore_int_ctrl_break_"  # cmp ebx, 0xe4f21
    jne .L000f5070
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int_"  # mov dword ptr [0x2db68], 0xe4f1c
.L000f5070:
    pop edx
    pop ecx
    pop ebx
    ret
.size "__grab_int23_",.-"__grab_int23_"

// FUNCTION: 0x000f5074

.section _TEXT,"ax",@progbits,unique,1003636
.globl "__grab_int_ctrl_break_"
.type "__grab_int_ctrl_break_",@function
"__grab_int_ctrl_break_":
    push ebx
    push ecx
    push edx
    cmp dword ptr ["___old_int_ctrl_break"], 0
    jne .L000f518b
    cmp word ptr ["___old_int_ctrl_break"+0x4], 0
    jne .L000f518b
    xor eax, eax
    mov al, byte ptr ["__Extender"]
    cmp eax, 2
    jl .L000f50f1
    cmp eax, 8
    jg .L000f50f1
    mov cl, 0x1b
    mov ax, 0x2503
    int 0x21
    mov cx, bx
    shr ebx, 0x10
    mov edx, ebx
    mov word ptr ["___old_int_ctrl_break"+0x4], cx
    mov dword ptr ["___old_int_ctrl_break"], edx
    mov cl, 0x1b
    .byte 0xba; .long "fn_000f4c5f_"  # mov edx, 0xe4c5f
    push es
    mov ax, 0x2502
    int 0x21
    mov cx, es
    pop es
    mov al, 0x1b
    mov word ptr ["___old_pm_int_ctrl_break"+0x4], cx
    mov ecx, cs
    mov dword ptr ["___old_pm_int_ctrl_break"], ebx
    push ds
    mov ds, ecx
    mov cl, al
    mov al, 6
    mov ah, 0x25
    int 0x21
    pop ds
    jmp .L000f515d
.L000f50f1:
    call "__DPMI_hosted_"
    cmp eax, 1
    jne .L000f5136
    mov bl, 0x1b
    mov ax, 0x200
    int 0x31
    mov bl, 0x1b
    mov word ptr ["___old_int_ctrl_break"+0x4], cx
    mov dword ptr ["___old_int_ctrl_break"], edx
    mov ax, 0x204
    int 0x31
    mov bl, 0x1b
    mov word ptr ["___old_pm_int_ctrl_break"+0x4], cx
    mov dword ptr ["___old_pm_int_ctrl_break"], edx
    .byte 0xba; .long "fn_000f4c5f_"  # mov edx, 0xe4c5f
    mov ecx, cs
    mov ax, 0x205
    int 0x31
    jmp .L000f515d
.L000f5136:
    mov eax, 0x1b
    .byte 0xbb; .long "fn_000f4c5f_"  # mov ebx, 0xe4c5f
    call "_dos_getvect_"
    mov ecx, cs
    mov dword ptr ["___old_int_ctrl_break"], eax
    mov eax, 0x1b
    mov word ptr ["___old_int_ctrl_break"+0x4], dx
    call "_dos_setvect_"
.L000f515d:
    mov ebx, dword ptr ["___int23_exit"]
    .byte 0x81; .byte 0xfb; .long "__null_int23_exit_"  # cmp ebx, 0xb2110
    jne .L000f5179
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int_ctrl_break_"  # mov dword ptr [0x2db68], 0xe4f21
    pop edx
    pop ecx
    pop ebx
    ret
.L000f5179:
    .byte 0x81; .byte 0xfb; .long "__restore_int23_"  # cmp ebx, 0xe4d6b
    jne .L000f518b
    .byte 0xc7; .byte 0x05; .long "___int23_exit"; .long "__restore_int_"  # mov dword ptr [0x2db68], 0xe4f1c
.L000f518b:
    pop edx
    pop ecx
    pop ebx
    ret
.size "__grab_int_ctrl_break_",.-"__grab_int_ctrl_break_"
