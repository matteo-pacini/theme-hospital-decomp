.intel_syntax noprefix

// FUNCTION: 0x001093f0

.section _TEXT,"ax",@progbits,unique,1086448
.globl "LbSpriteDrawUsingScalingData_"
.type "LbSpriteDrawUsingScalingData_",@function
"LbSpriteDrawUsingScalingData_":
    .byte 0x60  # pushal
    sub esp, 0x280
    {load} xor ecx, ecx
    mov cl, byte ptr [ebx + 4]
    mov dword ptr [esp + 0x24], ecx
    mov cl, byte ptr [ebx + 5]
    mov dword ptr [esp + 0x28], ecx
    mov esi, dword ptr ["_lbDisplay"+0x20]
    mov edi, 8
    mov cx, word ptr ["_lbDisplay"+0x60]
    and cx, 1
    je .L00109424
    add eax, dword ptr [esp + 0x24]
    dec eax
.L00109424:
    mov cx, word ptr ["_lbDisplay"+0x60]
    and cx, 2
    je .L0010943a
    add edx, dword ptr [esp + 0x28]
    dec edx
    neg esi
    neg edi
.L0010943a:
    mov dword ptr [esp + 0x2c], esi
    mov dword ptr [esp + 0x30], edi
    lea ebp, [eax*8 + "_xsteps_array"]
    mov dword ptr [esp + 4], ebp
    lea edi, [edx*8 + "_ysteps_array"]
    mov eax, dword ptr ["_lbSpriteReMapPtr"]
    mov dword ptr [esp + 0x20], eax
    mov ecx, dword ptr [esp + 0x28]
    mov edx, dword ptr [ebx]
    mov esi, dword ptr [edi]
    mov ax, word ptr ["_lbDisplay"+0x60]
    and ax, 2
    je .L00109475
    add esi, dword ptr [edi + 4]
    dec esi
.L00109475:
    mov eax, dword ptr ["_lbDisplay"+0x20]
    imul esi, eax
    add esi, dword ptr ["_lbDisplay"+0x10]
    add esi, dword ptr [ebp]
    mov ax, word ptr ["_lbDisplay"+0x60]
    and ax, 1
    je .L00109496
    add esi, dword ptr [ebp + 4]
    dec esi
.L00109496:
    mov eax, dword ptr ["_scale_up"]
    {load} or eax, eax
    jne .L0010a308
    mov ax, word ptr ["_lbDisplay"+0x60]
    test ax, 0x800
    jne .L00109f98
    test ax, 4
    jne .L001097f8
    test ax, 8
    jne .L001099e8
    test ax, 1
    jne .L00109668
    jmp .L0010952b
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L001094e0:
    {load} xor eax, eax
    {load} mov eax, eax
.L001094e4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109520
    js .L001094e4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109520
    js .L001094e4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109520
    js .L001094e4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109520
    js .L001094e4
    {load} add edx, eax
    jmp .L001094e4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109520:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L00109660
.L0010952b:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L001094e0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L00109542:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L00109640
    js .L00109620
    jmp .L00109567
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109560:
    inc edx
    add ebp, 8
    dec ecx
    je .L00109542
.L00109567:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109560
    mov al, byte ptr [edx]
    inc edx
    add ebp, 8
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109542
    jmp .L00109567
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L00109620:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L00109542
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109640:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109520
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109660:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L00109668:
    jmp .L001096bb
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109670:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109674:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L001096b0
    js .L00109674
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L001096b0
    js .L00109674
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L001096b0
    js .L00109674
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L001096b0
    js .L00109674
    {load} add edx, eax
    jmp .L00109674
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L001096b0:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L001097f0
.L001096bb:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109670
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L001096d2:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L001097d0
    js .L001097b0
    jmp .L001096f7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001096f0:
    inc edx
    sub ebp, 8
    dec ecx
    je .L001096d2
.L001096f7:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L001096f0
    mov al, byte ptr [edx]
    inc edx
    sub ebp, 8
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L001096d2
    jmp .L001096f7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L001097b0:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L001096d2
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L001097d0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L001096b0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L001097f0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L001097f8:
    and ax, 1
    jne .L00109bd8
    jmp .L0010985b
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109810:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109814:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109850
    js .L00109814
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109850
    js .L00109814
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109850
    js .L00109814
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109850
    js .L00109814
    {load} add edx, eax
    jmp .L00109814
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109850:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L001099e0
.L0010985b:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109810
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L00109872:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L001099c0
    js .L001099a0
    jmp .L00109897
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109890:
    inc edx
    add ebp, 8
    dec ecx
    je .L00109872
.L00109897:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109890
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109872
    jmp .L00109897
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001099a0:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L00109872
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001099c0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109850
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L001099e0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L001099e8:
    and ax, 1
    jne .L00109db8
    jmp .L00109a4b
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109a00:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109a04:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109a40
    js .L00109a04
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109a40
    js .L00109a04
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109a40
    js .L00109a04
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109a40
    js .L00109a04
    {load} add edx, eax
    jmp .L00109a04
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109a40:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L00109bd0
.L00109a4b:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109a00
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L00109a62:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L00109bb0
    js .L00109b90
    jmp .L00109a87
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109a80:
    inc edx
    add ebp, 8
    dec ecx
    je .L00109a62
.L00109a87:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109a80
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L00109a62
    jmp .L00109a87
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109b90:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L00109a62
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109bb0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109a40
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109bd0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L00109bd8:
    jmp .L00109c2b
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109be0:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109be4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109c20
    js .L00109be4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109c20
    js .L00109be4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109c20
    js .L00109be4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109c20
    js .L00109be4
    {load} add edx, eax
    jmp .L00109be4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109c20:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L00109db0
.L00109c2b:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109be0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L00109c42:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L00109d90
    js .L00109d70
    jmp .L00109c67
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109c60:
    inc edx
    sub ebp, 8
    dec ecx
    je .L00109c42
.L00109c67:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109c60
    mov al, byte ptr [esi]
    mov ah, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109c42
    jmp .L00109c67
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109d70:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L00109c42
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109d90:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109c20
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109db0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L00109db8:
    jmp .L00109e0b
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109dc0:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109dc4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109e00
    js .L00109dc4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109e00
    js .L00109dc4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109e00
    js .L00109dc4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109e00
    js .L00109dc4
    {load} add edx, eax
    jmp .L00109dc4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109e00:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L00109f90
.L00109e0b:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109dc0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
.L00109e22:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L00109f70
    js .L00109f50
    jmp .L00109e47
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109e40:
    inc edx
    sub ebp, 8
    dec ecx
    je .L00109e22
.L00109e47:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L00109e40
    mov ah, byte ptr [esi]
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [eax + _ghost_table]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L00109e22
    jmp .L00109e47
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109f50:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L00109e22
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109f70:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109e00
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109f90:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L00109f98:
    test ax, 1
    jne .L0010a158
    jmp .L00109ffb
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00109fb0:
    {load} xor eax, eax
    {load} mov eax, eax
.L00109fb4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109ff0
    js .L00109fb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109ff0
    js .L00109fb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109ff0
    js .L00109fb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L00109ff0
    js .L00109fb4
    {load} add edx, eax
    jmp .L00109fb4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L00109ff0:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010a150
.L00109ffb:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L00109fb0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    mov edi, dword ptr [esp + 0x20]
.L0010a016:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010a130
    js .L0010a110
    jmp .L0010a037
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a030:
    inc edx
    add ebp, 8
    dec ecx
    je .L0010a016
.L0010a037:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a030
    mov al, byte ptr [edx]
    add ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a016
    jmp .L0010a037
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a110:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L0010a016
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a130:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L00109ff0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a150:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010a158:
    jmp .L0010a1ab
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a160:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010a164:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a1a0
    js .L0010a164
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a1a0
    js .L0010a164
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a1a0
    js .L0010a164
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a1a0
    js .L0010a164
    {load} add edx, eax
    jmp .L0010a164
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a1a0:
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010a300
.L0010a1ab:
    mov eax, dword ptr [edi + 4]
    {load} or eax, eax
    je .L0010a160
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x10], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    mov edi, dword ptr [esp + 0x20]
.L0010a1c6:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010a2e0
    js .L0010a2c0
    jmp .L0010a1e7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a1e0:
    inc edx
    sub ebp, 8
    dec ecx
    je .L0010a1c6
.L0010a1e7:
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    mov eax, dword ptr [ebp + 4]
    {load} or eax, eax
    je .L0010a1e0
    mov al, byte ptr [edx]
    sub ebp, 8
    mov al, byte ptr [edi + eax]
    inc edx
    mov byte ptr [esi], al
    dec esi
    dec ecx
    je .L0010a1c6
    jmp .L0010a1e7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a2c0:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L0010a1c6
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a2e0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x10]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    jmp .L0010a1a0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a300:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010a308:
    mov ax, word ptr ["_lbDisplay"+0x60]
    test ax, 0x800
    jne .L0010ae18
    test ax, 4
    jne .L0010a838
    test ax, 8
    jne .L0010a9b8
    test ax, 1
    jne .L0010a5b8
    jmp .L0010a38f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a340:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010a344:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a380
    js .L0010a344
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a380
    js .L0010a344
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a380
    js .L0010a344
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a380
    js .L0010a344
    {load} add edx, eax
    jmp .L0010a344
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a380:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010a5b0
.L0010a38f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    mov dword ptr [esp + 0x3c], ecx
    {load} or ecx, ecx
    je .L0010a340
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    jmp .L0010a4dc
    .byte 0x90
.L0010a3b4:
    mov ebx, dword ptr [esp + 0x3c]
    dec ebx
    je .L0010a4dc
    js .L0010a4dc
    mov edi, dword ptr [esp + 0x34]
    mov dword ptr [esp + 0x38], esi
    {load} mov ecx, esi
    {load} sub ecx, edi
    {load} mov esi, edi
    add esi, dword ptr [esp + 0x2c]
    mov dword ptr [esp + 0x40], ecx
    nop
.L0010a3dc:
    mov dword ptr [esp + 0x44], esi
    {load} or ecx, ecx
    je .L0010a4bf
    test esi, 3
    je .L0010a428
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a4bf
    test esi, 3
    je .L0010a428
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a4bf
    test esi, 3
    je .L0010a428
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a4bf
    nop
.L0010a428:
    mov dword ptr [esp + 0x48], ecx
    sar ecx, 2
    je .L0010a4a0
.L0010a431:
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a4a0
    jmp .L0010a431
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a4a0:
    mov ecx, dword ptr [esp + 0x48]
    and ecx, 3
    je .L0010a4bf
    mov al, byte ptr [edi]
    mov byte ptr [esi], al
    dec ecx
    je .L0010a4bf
    mov al, byte ptr [edi + 1]
    mov byte ptr [esi + 1], al
    dec ecx
    je .L0010a4bf
    mov al, byte ptr [edi + 2]
    mov byte ptr [esi + 2], al
.L0010a4bf:
    mov edi, dword ptr [esp + 0x34]
    mov ecx, dword ptr [esp + 0x40]
    mov esi, dword ptr [esp + 0x44]
    add esi, dword ptr [esp + 0x2c]
    dec ebx
    jne .L0010a3dc
    mov esi, dword ptr [esp + 0x38]
    {load} mov eax, eax
.L0010a4dc:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010a580
    js .L0010a560
    mov dword ptr [esp + 0x34], esi
    jmp .L0010a50e
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a500:
    add esi, dword ptr [ebp + 4]
.L0010a503:
    inc edx
    add ebp, 8
    dec ecx
    je .L0010a3b4
.L0010a50e:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010a503
    mov al, byte ptr [edx]
    mov byte ptr [esi], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 1], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 2], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 3], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 4], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 5], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 6], al
    dec ebx
    je .L0010a500
    mov byte ptr [esi + 7], al
    dec ebx
    je .L0010a500
    add esi, 8
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L0010a54c:
    mov byte ptr [esi], al
    inc esi
    dec ebx
    jne .L0010a54c
    jmp .L0010a503
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a560:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L0010a4dc
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a580:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    je .L0010a380
.L0010a597:
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010a597
    jmp .L0010a380
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010a5b0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010a5b8:
    jmp .L0010a60f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a5c0:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010a5c4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a600
    js .L0010a5c4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a600
    js .L0010a5c4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a600
    js .L0010a5c4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a600
    js .L0010a5c4
    {load} add edx, eax
    jmp .L0010a5c4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a600:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010a830
.L0010a60f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    mov dword ptr [esp + 0x3c], ecx
    {load} or ecx, ecx
    je .L0010a5c0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    jmp .L0010a75c
    .byte 0x90
.L0010a634:
    mov ebx, dword ptr [esp + 0x3c]
    dec ebx
    je .L0010a75c
    js .L0010a75c
    mov edi, dword ptr [esp + 0x34]
    mov dword ptr [esp + 0x38], esi
    {load} mov ecx, edi
    {load} sub ecx, esi
    {load} mov edi, esi
    mov dword ptr [esp + 0x34], edi
    inc ecx
    add esi, dword ptr [esp + 0x2c]
    mov dword ptr [esp + 0x40], ecx
.L0010a660:
    mov dword ptr [esp + 0x44], esi
    {load} or ecx, ecx
    je .L0010a73f
    test esi, 3
    je .L0010a6ac
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a73f
    test esi, 3
    je .L0010a6ac
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a73f
    test esi, 3
    je .L0010a6ac
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010a73f
    nop
.L0010a6ac:
    mov dword ptr [esp + 0x48], ecx
    sar ecx, 2
    je .L0010a720
.L0010a6b5:
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010a720
    jmp .L0010a6b5
    .byte 0x90
.L0010a720:
    mov ecx, dword ptr [esp + 0x48]
    and ecx, 3
    je .L0010a73f
    mov al, byte ptr [edi]
    mov byte ptr [esi], al
    dec ecx
    je .L0010a73f
    mov al, byte ptr [edi + 1]
    mov byte ptr [esi + 1], al
    dec ecx
    je .L0010a73f
    mov al, byte ptr [edi + 2]
    mov byte ptr [esi + 2], al
.L0010a73f:
    mov edi, dword ptr [esp + 0x34]
    mov ecx, dword ptr [esp + 0x40]
    mov esi, dword ptr [esp + 0x44]
    add esi, dword ptr [esp + 0x2c]
    dec ebx
    jne .L0010a660
    mov esi, dword ptr [esp + 0x38]
    {load} mov eax, eax
.L0010a75c:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010a800
    js .L0010a7e0
    mov dword ptr [esp + 0x34], esi
    jmp .L0010a78e
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a780:
    sub esi, dword ptr [ebp + 4]
.L0010a783:
    inc edx
    sub ebp, 8
    dec ecx
    je .L0010a634
.L0010a78e:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010a783
    mov al, byte ptr [edx]
    mov byte ptr [esi], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 1], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 2], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 3], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 4], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 5], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 6], al
    dec ebx
    je .L0010a780
    mov byte ptr [esi - 7], al
    dec ebx
    je .L0010a780
    sub esi, 8
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L0010a7cc:
    mov byte ptr [esi], al
    dec esi
    dec ebx
    jne .L0010a7cc
    jmp .L0010a783
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a7e0:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L0010a75c
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a800:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    je .L0010a600
.L0010a817:
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010a817
    jmp .L0010a600
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010a830:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010a838:
    test ax, 1
    jne .L0010ab38
    jmp .L0010a89f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a850:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010a854:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a890
    js .L0010a854
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a890
    js .L0010a854
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a890
    js .L0010a854
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010a890
    js .L0010a854
    {load} add edx, eax
    jmp .L0010a854
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a890:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010a9b0
.L0010a89f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    {load} or ecx, ecx
    mov dword ptr [esp + 0x18], edx
    je .L0010a850
.L0010a8ae:
    mov edx, dword ptr [esp + 0x18]
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    {load} mov eax, eax
.L0010a8c4:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010a990
    js .L0010a970
    jmp .L0010a8e7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010a8e0:
    inc edx
    add ebp, 8
    dec ecx
    je .L0010a8c4
.L0010a8e7:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010a8e0
    mov ah, byte ptr [edx]
.L0010a8f0:
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    inc esi
    dec ebx
    je .L0010a8e0
    jmp .L0010a8f0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010a970:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L0010a8c4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a990:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010a8ae
    jmp .L0010a890
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010a9b0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010a9b8:
    test ax, 1
    jne .L0010aca8
    jmp .L0010aa1f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010a9d0:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010a9d4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010aa10
    js .L0010a9d4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010aa10
    js .L0010a9d4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010aa10
    js .L0010a9d4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010aa10
    js .L0010a9d4
    {load} add edx, eax
    jmp .L0010a9d4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010aa10:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010ab30
.L0010aa1f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    {load} or ecx, ecx
    mov dword ptr [esp + 0x18], edx
    je .L0010a9d0
.L0010aa2e:
    mov edx, dword ptr [esp + 0x18]
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    {load} mov eax, eax
.L0010aa44:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010ab10
    js .L0010aaf0
    jmp .L0010aa67
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010aa60:
    inc edx
    add ebp, 8
    dec ecx
    je .L0010aa44
.L0010aa67:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010aa60
    mov al, byte ptr [edx]
.L0010aa70:
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    inc esi
    dec ebx
    je .L0010aa60
    jmp .L0010aa70
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010aaf0:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L0010aa44
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010ab10:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010aa2e
    jmp .L0010aa10
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010ab30:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010ab38:
    jmp .L0010ab8f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010ab40:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010ab44:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ab80
    js .L0010ab44
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ab80
    js .L0010ab44
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ab80
    js .L0010ab44
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ab80
    js .L0010ab44
    {load} add edx, eax
    jmp .L0010ab44
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010ab80:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010aca0
.L0010ab8f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    {load} or ecx, ecx
    mov dword ptr [esp + 0x18], edx
    je .L0010ab40
.L0010ab9e:
    mov edx, dword ptr [esp + 0x18]
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    {load} mov eax, eax
.L0010abb4:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010ac80
    js .L0010ac60
    jmp .L0010abd7
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010abd0:
    inc edx
    sub ebp, 8
    dec ecx
    je .L0010abb4
.L0010abd7:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010abd0
    mov ah, byte ptr [edx]
.L0010abe0:
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    mov al, byte ptr [esi]
    mov al, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], al
    dec esi
    dec ebx
    je .L0010abd0
    jmp .L0010abe0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010ac60:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L0010abb4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010ac80:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010ab9e
    jmp .L0010ab80
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010aca0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010aca8:
    jmp .L0010acff
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010acb0:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010acb4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010acf0
    js .L0010acb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010acf0
    js .L0010acb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010acf0
    js .L0010acb4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010acf0
    js .L0010acb4
    {load} add edx, eax
    jmp .L0010acb4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010acf0:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010ae10
.L0010acff:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    {load} or ecx, ecx
    mov dword ptr [esp + 0x18], edx
    je .L0010acb0
.L0010ad0e:
    mov edx, dword ptr [esp + 0x18]
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    {load} mov eax, eax
.L0010ad24:
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010adf0
    js .L0010add0
    jmp .L0010ad47
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010ad40:
    inc edx
    sub ebp, 8
    dec ecx
    je .L0010ad24
.L0010ad47:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010ad40
    mov al, byte ptr [edx]
.L0010ad50:
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    mov ah, byte ptr [esi]
    mov ah, byte ptr [eax + _ghost_table]
    mov byte ptr [esi], ah
    dec esi
    dec ebx
    je .L0010ad40
    jmp .L0010ad50
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010add0:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L0010ad24
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010adf0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010ad0e
    jmp .L0010acf0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010ae10:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010ae18:
    test ax, 1
    jne .L0010b0a8
    jmp .L0010ae7f
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010ae30:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010ae34:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ae70
    js .L0010ae34
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ae70
    js .L0010ae34
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ae70
    js .L0010ae34
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010ae70
    js .L0010ae34
    {load} add edx, eax
    jmp .L0010ae34
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010ae70:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010b0a0
.L0010ae7f:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    mov dword ptr [esp + 0x3c], ecx
    {load} or ecx, ecx
    je .L0010ae30
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    jmp .L0010afcc
    .byte 0x90
.L0010aea4:
    mov ebx, dword ptr [esp + 0x3c]
    dec ebx
    je .L0010afcc
    js .L0010afcc
    mov edi, dword ptr [esp + 0x34]
    mov dword ptr [esp + 0x38], esi
    {load} mov ecx, esi
    {load} sub ecx, edi
    {load} mov esi, edi
    add esi, dword ptr [esp + 0x2c]
    mov dword ptr [esp + 0x40], ecx
    nop
.L0010aecc:
    mov dword ptr [esp + 0x44], esi
    {load} or ecx, ecx
    je .L0010afaf
    test esi, 3
    je .L0010af18
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010afaf
    test esi, 3
    je .L0010af18
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010afaf
    test esi, 3
    je .L0010af18
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010afaf
    nop
.L0010af18:
    mov dword ptr [esp + 0x48], ecx
    sar ecx, 2
    je .L0010af90
.L0010af21:
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010af90
    jmp .L0010af21
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010af90:
    mov ecx, dword ptr [esp + 0x48]
    and ecx, 3
    je .L0010afaf
    mov al, byte ptr [edi]
    mov byte ptr [esi], al
    dec ecx
    je .L0010afaf
    mov al, byte ptr [edi + 1]
    mov byte ptr [esi + 1], al
    dec ecx
    je .L0010afaf
    mov al, byte ptr [edi + 2]
    mov byte ptr [esi + 2], al
.L0010afaf:
    mov edi, dword ptr [esp + 0x34]
    mov ecx, dword ptr [esp + 0x40]
    mov esi, dword ptr [esp + 0x44]
    add esi, dword ptr [esp + 0x2c]
    dec ebx
    jne .L0010aecc
    mov esi, dword ptr [esp + 0x38]
    {load} mov eax, eax
.L0010afcc:
    mov edi, dword ptr [esp + 0x20]
    {load} xor eax, eax
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010b070
    js .L0010b050
    mov dword ptr [esp + 0x34], esi
    jmp .L0010affe
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010aff0:
    add esi, dword ptr [ebp + 4]
.L0010aff3:
    inc edx
    add ebp, 8
    dec ecx
    je .L0010aea4
.L0010affe:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010aff3
    mov al, byte ptr [edx]
    mov al, byte ptr [edi + eax]
    mov byte ptr [esi], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 1], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 2], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 3], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 4], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 5], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 6], al
    dec ebx
    je .L0010aff0
    mov byte ptr [esi + 7], al
    dec ebx
    je .L0010aff0
    add esi, 8
.L0010b03c:
    mov byte ptr [esi], al
    inc esi
    dec ebx
    jne .L0010b03c
    jmp .L0010aff3
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010b050:
    neg cl
    sub esi, dword ptr [ebp]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp]
    jmp .L0010afcc
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010b070:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    je .L0010ae70
.L0010b087:
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010b087
    jmp .L0010ae70
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010b0a0:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.L0010b0a8:
    jmp .L0010b0ff
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010b0b0:
    {load} xor eax, eax
    {load} mov eax, eax
.L0010b0b4:
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010b0f0
    js .L0010b0b4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010b0f0
    js .L0010b0b4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010b0f0
    js .L0010b0b4
    {load} add edx, eax
    mov al, byte ptr [edx]
    inc edx
    {load} or al, al
    je .L0010b0f0
    js .L0010b0b4
    {load} add edx, eax
    jmp .L0010b0b4
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010b0f0:
    mov ecx, dword ptr [esp + 0x10]
    add edi, dword ptr [esp + 0x30]
    dec ecx
    je .L0010b320
.L0010b0ff:
    mov dword ptr [esp + 0x10], ecx
    mov ecx, dword ptr [edi + 4]
    mov dword ptr [esp + 0x3c], ecx
    {load} or ecx, ecx
    je .L0010b0b0
    mov dword ptr [esp + 0xc], esi
    mov dword ptr [esp + 0x1c], ecx
    mov dword ptr [esp + 8], edi
    mov ebp, dword ptr [esp + 4]
    jmp .L0010b24c
    .byte 0x90
.L0010b124:
    mov ebx, dword ptr [esp + 0x3c]
    dec ebx
    je .L0010b24c
    js .L0010b24c
    mov edi, dword ptr [esp + 0x34]
    mov dword ptr [esp + 0x38], esi
    {load} mov ecx, edi
    {load} sub ecx, esi
    {load} mov edi, esi
    mov dword ptr [esp + 0x34], edi
    inc ecx
    add esi, dword ptr [esp + 0x2c]
    mov dword ptr [esp + 0x40], ecx
.L0010b150:
    mov dword ptr [esp + 0x44], esi
    {load} or ecx, ecx
    je .L0010b22f
    test esi, 3
    je .L0010b19c
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010b22f
    test esi, 3
    je .L0010b19c
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010b22f
    test esi, 3
    je .L0010b19c
    mov al, byte ptr [edi]
    inc edi
    mov byte ptr [esi], al
    inc esi
    dec ecx
    je .L0010b22f
    nop
.L0010b19c:
    mov dword ptr [esp + 0x48], ecx
    sar ecx, 2
    je .L0010b210
.L0010b1a5:
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    mov eax, dword ptr [edi]
    add edi, 4
    mov dword ptr [esi], eax
    add esi, 4
    dec ecx
    je .L0010b210
    jmp .L0010b1a5
    .byte 0x90
.L0010b210:
    mov ecx, dword ptr [esp + 0x48]
    and ecx, 3
    je .L0010b22f
    mov al, byte ptr [edi]
    mov byte ptr [esi], al
    dec ecx
    je .L0010b22f
    mov al, byte ptr [edi + 1]
    mov byte ptr [esi + 1], al
    dec ecx
    je .L0010b22f
    mov al, byte ptr [edi + 2]
    mov byte ptr [esi + 2], al
.L0010b22f:
    mov edi, dword ptr [esp + 0x34]
    mov ecx, dword ptr [esp + 0x40]
    mov esi, dword ptr [esp + 0x44]
    add esi, dword ptr [esp + 0x2c]
    dec ebx
    jne .L0010b150
    mov esi, dword ptr [esp + 0x38]
    {load} mov eax, eax
.L0010b24c:
    mov edi, dword ptr [esp + 0x20]
    {load} xor eax, eax
    {load} xor ecx, ecx
    mov cl, byte ptr [edx]
    inc edx
    {load} or cl, cl
    je .L0010b2f0
    js .L0010b2d0
    mov dword ptr [esp + 0x34], esi
    jmp .L0010b27e
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010b270:
    sub esi, dword ptr [ebp + 4]
.L0010b273:
    inc edx
    sub ebp, 8
    dec ecx
    je .L0010b124
.L0010b27e:
    mov ebx, dword ptr [ebp + 4]
    {load} or ebx, ebx
    je .L0010b273
    mov al, byte ptr [edx]
    mov al, byte ptr [edi + eax]
    mov byte ptr [esi], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 1], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 2], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 3], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 4], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 5], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 6], al
    dec ebx
    je .L0010b270
    mov byte ptr [esi - 7], al
    dec ebx
    je .L0010b270
    sub esi, 8
.L0010b2bc:
    mov byte ptr [esi], al
    dec esi
    dec ebx
    jne .L0010b2bc
    jmp .L0010b273
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010b2d0:
    neg cl
    sub esi, dword ptr [ebp]
    neg ecx
    sub esi, dword ptr [ebp + 4]
    lea ebp, [ebp + ecx*8]
    add esi, dword ptr [ebp + 4]
    neg ecx
    add esi, dword ptr [ebp]
    jmp .L0010b24c
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x8b; .byte 0xc0
.L0010b2f0:
    mov esi, dword ptr [esp + 0xc]
    mov ecx, dword ptr [esp + 0x1c]
    mov edi, dword ptr [esp + 8]
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    je .L0010b0f0
.L0010b307:
    add esi, dword ptr [esp + 0x2c]
    dec ecx
    jne .L0010b307
    jmp .L0010b0f0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x2e; .byte 0x8b; .byte 0xc0
    .byte 0x90
.L0010b320:
    add esp, 0x280
    .byte 0x61  # popal
    ret
.size "LbSpriteDrawUsingScalingData_",.-"LbSpriteDrawUsingScalingData_"

// FUNCTION: 0x0010b328

.section _TEXT,"ax",@progbits,unique,1094440
.globl "LbSpriteSetScalingData_"
.type "LbSpriteSetScalingData_",@function
"LbSpriteSetScalingData_":
    .byte 0x60  # pushal
    mov esi, dword ptr [esp + 0x24]
    mov edi, dword ptr [esp + 0x28]
    sub esp, 0x20
    mov ebp, 1
    {load} cmp esi, ebx
    jg .L0010b343
    {load} cmp edi, ecx
    jg .L0010b343
    {load} xor ebp, ebp
.L0010b343:
    mov dword ptr ["_scale_up"], ebp
    mov ebp, dword ptr ["_lbDisplay"+0x30]
    mov dword ptr [esp + 0x18], ebp
    mov ebp, dword ptr ["_lbDisplay"+0x34]
    mov dword ptr [esp + 0x1c], ebp
    mov dword ptr [esp], eax
    mov dword ptr [esp + 4], edx
    mov dword ptr [esp + 8], ebx
    mov dword ptr [esp + 0xc], ecx
    mov dword ptr [esp + 0x10], esi
    mov dword ptr [esp + 0x14], edi
    {load} or eax, eax
    js .L0010b472
    {load} mov ebp, eax
    {load} add ebp, esi
    cmp ebp, dword ptr [esp + 0x18]
    jge .L0010b472
    mov esi, dword ptr [esp]
    shl esi, 0x10
    mov eax, dword ptr [esp + 0x10]
    mov ebx, dword ptr [esp + 8]
    shl eax, 0x10
    {load} xor edx, edx
    idiv ebx
    lea edx, ["_xsteps_array"]
    mov ecx, dword ptr [esp + 8]
    {load} mov ebx, eax
    sar ebx, 1
    {load} add esi, ebx
    {load} mov edi, esi
    sar edi, 0x10
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L0010b3c0:
    mov dword ptr [edx], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 4], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 8], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0xc], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x10], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x14], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x18], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x1c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x20], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x24], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x28], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x2c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x30], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x34], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    mov dword ptr [edx + 0x38], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x3c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b470
    add edx, 0x40
    jmp .L0010b3c0
    .byte 0x90
.L0010b470:
    jmp .L0010b4d5
.L0010b472:
    mov esi, dword ptr [esp]
    shl esi, 0x10
    mov eax, dword ptr [esp + 0x10]
    mov ebx, dword ptr [esp + 8]
    shl eax, 0x10
    {load} xor edx, edx
    idiv ebx
    lea edx, ["_xsteps_array"]
    mov ecx, dword ptr [esp + 8]
    {load} mov ebx, eax
    sar ebx, 1
    {load} add esi, ebx
    {load} mov edi, esi
    sar edi, 0x10
    jns .L0010b4a3
    mov edi, 0
.L0010b4a3:
    cmp edi, dword ptr [esp + 0x18]
    jl .L0010b4ad
    mov edi, dword ptr [esp + 0x18]
.L0010b4ad:
    mov dword ptr [edx], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    jns .L0010b4bc
    {load} xor ebx, ebx
    {load} mov eax, eax
.L0010b4bc:
    cmp ebx, dword ptr [esp + 0x18]
    jl .L0010b4c8
    mov ebx, dword ptr [esp + 0x18]
    {load} mov eax, eax
.L0010b4c8:
    {load} sub ebx, edi
    mov dword ptr [edx + 4], ebx
    {load} add edi, ebx
    add edx, 8
    dec ecx
    jne .L0010b4ad
.L0010b4d5:
    {load} or edx, edx
    js .L0010b5d2
    {load} mov ebx, edx
    {load} add ebx, edi
    cmp ebx, dword ptr [esp + 0x1c]
    jge .L0010b5d2
    mov esi, dword ptr [esp + 4]
    shl esi, 0x10
    mov eax, dword ptr [esp + 0x14]
    mov ebx, dword ptr [esp + 0xc]
    shl eax, 0x10
    {load} xor edx, edx
    idiv ebx
    lea edx, ["_ysteps_array"]
    mov ecx, dword ptr [esp + 0xc]
    {load} mov ebx, eax
    sar ebx, 1
    {load} add esi, ebx
    {load} mov edi, esi
    sar edi, 0x10
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
    nop
.L0010b520:
    mov dword ptr [edx], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 4], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 8], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0xc], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x10], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x14], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x18], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x1c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x20], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x24], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x28], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x2c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x30], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x34], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    mov dword ptr [edx + 0x38], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    {load} sub ebx, edi
    mov dword ptr [edx + 0x3c], ebx
    {load} add edi, ebx
    dec ecx
    je .L0010b5d0
    add edx, 0x40
    jmp .L0010b520
    .byte 0x90
.L0010b5d0:
    jmp .L0010b635
.L0010b5d2:
    mov esi, dword ptr [esp + 4]
    shl esi, 0x10
    mov eax, dword ptr [esp + 0x14]
    mov ebx, dword ptr [esp + 0xc]
    shl eax, 0x10
    {load} xor edx, edx
    idiv ebx
    lea edx, ["_ysteps_array"]
    mov ecx, dword ptr [esp + 0xc]
    {load} mov ebx, eax
    sar ebx, 1
    {load} add esi, ebx
    {load} mov edi, esi
    sar edi, 0x10
    jns .L0010b604
    mov edi, 0
.L0010b604:
    cmp edi, dword ptr [esp + 0x1c]
    jl .L0010b60e
    mov edi, dword ptr [esp + 0x1c]
.L0010b60e:
    mov dword ptr [edx], edi
    {load} add esi, eax
    {load} mov ebx, esi
    sar ebx, 0x10
    jns .L0010b61c
    {load} xor ebx, ebx
    nop
.L0010b61c:
    cmp ebx, dword ptr [esp + 0x1c]
    jl .L0010b628
    mov ebx, dword ptr [esp + 0x1c]
    {load} mov eax, eax
.L0010b628:
    {load} sub ebx, edi
    mov dword ptr [edx + 4], ebx
    {load} add edi, ebx
    add edx, 8
    dec ecx
    jne .L0010b60e
.L0010b635:
    add esp, 0x20
    .byte 0x61  # popal
    ret 8
.size "LbSpriteSetScalingData_",.-"LbSpriteSetScalingData_"
