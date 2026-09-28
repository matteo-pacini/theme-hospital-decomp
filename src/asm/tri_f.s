.intel_syntax noprefix

// FUNCTION: 0x00108c78

.section _TEXT,"ax",@progbits,unique,1084536
.globl "LbDrawTriangleFilled_"
.type "LbDrawTriangleFilled_",@function
"LbDrawTriangleFilled_":
    push ebp
    {load} mov ebp, esp
    add esp, -0x4c
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    mov dword ptr [ebp - 4], eax
    mov dword ptr [ebp - 8], edx
    mov dword ptr [ebp - 0xc], ebx
    mov dword ptr [ebp - 0x10], ecx
    lea esi, [ebp - 0x14]
    lea edi, [ebp - 0x18]
    lea ecx, [ebp - 0x1c]
    mov eax, dword ptr [ebp - 4]
    mov word ptr [esi], ax
    mov eax, dword ptr [ebp - 0xc]
    mov word ptr [edi], ax
    mov eax, dword ptr [ebp + 8]
    mov word ptr [ecx], ax
    mov eax, dword ptr [ebp - 8]
    mov word ptr [esi + 2], ax
    mov ebx, dword ptr [ebp - 0x10]
    mov word ptr [edi + 2], bx
    mov edx, dword ptr [ebp + 0xc]
    mov word ptr [ecx + 2], dx
    mov ax, word ptr [esi + 2]
    mov bx, word ptr [edi + 2]
    mov dx, word ptr [ecx + 2]
    {load} cmp ax, bx
    je .L00108d25
    jg .L00108ce7
    {load} cmp ax, dx
    je .L00108d10
    js .L00108d67
    xchg ecx, esi
    xchg ecx, edi
    jmp .L00108d72
.L00108ce7:
    {load} cmp ax, dx
    je .L0010911d
    js .L00108f45
    {load} cmp bx, dx
    je .L00109222
    js .L00108d0a
    xchg ecx, esi
    xchg ecx, edi
    jmp .L00108f49
.L00108d0a:
    xchg edi, esi
    xchg ecx, edi
    jmp .L00108d72
.L00108d10:
    mov ax, word ptr [esi]
    cmp ax, word ptr [ecx]
    jle .L001093dc
    xchg ecx, esi
    xchg ecx, edi
    jmp .L00109232
.L00108d25:
    {load} cmp ax, dx
    je .L001093dc
    js .L00108d45
    mov ax, word ptr [esi]
    cmp ax, word ptr [edi]
    jle .L001093dc
    xchg ecx, esi
    xchg ecx, edi
    jmp .L0010912d
.L00108d45:
    mov ax, word ptr [edi]
    cmp ax, word ptr [esi]
    jle .L001093dc
    jmp .L00109232
.L00108d56:
    mov ax, word ptr [edi]
    cmp ax, word ptr [ecx]
    jle .L001093dc
    jmp .L0010912d
.L00108d67:
    {load} cmp bx, dx
    je .L00108d56
    jg .L00108f49
.L00108d72:
    mov ax, word ptr [esi + 2]
    mov word ptr [ebp - 0x46], ax
    {load} or ax, ax
    jns .L00108d8e
    mov ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 1
    jmp .L00108db2
.L00108d8e:
    cmp ax, word ptr [_D_00188c48]
    jge .L001093dc
    movzx ebx, ax
    imul ebx, dword ptr ["W?research_names$npnpna"]
    add ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 0
.L00108db2:
    movzx ebx, word ptr [ecx + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x49]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x30], ebx
    mov dword ptr [ebp - 0x40], ebx
    mov bx, word ptr [edi + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x48]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x34], ebx
    mov ax, word ptr [ecx]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv dword ptr [ebp - 0x30]
    mov dword ptr [ebp - 0x24], eax
    mov ax, word ptr [edi]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv dword ptr [ebp - 0x34]
    cmp eax, dword ptr [ebp - 0x24]
    jle .L001093dc
    mov dword ptr [ebp - 0x28], eax
    movzx ebx, word ptr [ecx + 2]
    sub bx, word ptr [edi + 2]
    mov ax, word ptr [ecx]
    sub ax, word ptr [edi]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x2c], eax
    mov dword ptr [ebp - 0x38], ebx
    mov ax, word ptr [edi]
    shl eax, 0x10
    mov dword ptr [ebp - 0x3c], eax
    mov ax, word ptr [esi]
    shl eax, 0x10
    {load} mov ebx, eax
    cmp byte ptr [ebp - 0x47], 0
    je .L00108ed9
    movsx edi, word ptr [ebp - 0x46]
    neg edi
    sub dword ptr [ebp - 0x40], edi
    jle .L001093dc
    mov dword ptr [ebp - 0x44], edi
    cmp di, word ptr [ebp - 0x34]
    js .L00108e9a
    mov edi, dword ptr [ebp - 0x24]
    imul edi, dword ptr [ebp - 0x34]
    {load} add eax, edi
    mov ebx, dword ptr [ebp - 0x3c]
    mov edi, dword ptr [ebp - 0x44]
    sub edi, dword ptr [ebp - 0x34]
    sub word ptr [ebp - 0x38], di
    mov dword ptr [ebp - 0x44], edi
    imul edi, dword ptr [ebp - 0x24]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x44]
    imul edi, dword ptr [ebp - 0x2c]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x49], 0
    je .L00108e8f
    mov edi, dword ptr [_D_00188c48]
    mov dword ptr [ebp - 0x38], edi
    mov dword ptr [ebp - 0x40], edi
.L00108e8f:
    lea edi, ["_lbVesaGran"+0x2]
    jmp .L00108f21
.L00108e9a:
    mov edi, dword ptr [ebp - 0x44]
    sub word ptr [ebp - 0x34], di
    imul edi, dword ptr [ebp - 0x24]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x44]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x49], 0
    je .L00108f04
    mov edi, dword ptr [_D_00188c48]
    mov dword ptr [ebp - 0x40], edi
    cmp byte ptr [ebp - 0x48], 0
    je .L00108ecb
    mov word ptr [ebp - 0x34], di
    jmp .L00108ed7
.L00108ecb:
    sub di, word ptr [ebp - 0x34]
    setle byte ptr [ebp - 0x48]
    mov word ptr [ebp - 0x38], di
.L00108ed7:
    jmp .L00108f04
.L00108ed9:
    cmp byte ptr [ebp - 0x49], 0
    je .L00108f04
    mov edi, dword ptr [_D_00188c48]
    sub di, word ptr [ebp - 0x46]
    mov dword ptr [ebp - 0x40], edi
    cmp byte ptr [ebp - 0x48], 0
    je .L00108ef8
    mov word ptr [ebp - 0x34], di
    jmp .L00108f04
.L00108ef8:
    sub di, word ptr [ebp - 0x34]
    setle byte ptr [ebp - 0x48]
    mov word ptr [ebp - 0x38], di
.L00108f04:
    lea edi, ["_lbVesaGran"+0x2]
.L00108f0a:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x24]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x28]
    add edi, 8
    dec word ptr [ebp - 0x34]
    jne .L00108f0a
    mov ebx, dword ptr [ebp - 0x3c]
.L00108f21:
    cmp byte ptr [ebp - 0x48], 0
    je .L00108f2c
    jmp .L00109326
.L00108f2c:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x24]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x2c]
    add edi, 8
    dec word ptr [ebp - 0x38]
    jne .L00108f2c
    jmp .L00109326
.L00108f45:
    xchg edi, esi
    xchg ecx, edi
.L00108f49:
    mov ax, word ptr [esi + 2]
    mov word ptr [ebp - 0x46], ax
    {load} or ax, ax
    jns .L00108f65
    mov ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 1
    jmp .L00108f89
.L00108f65:
    cmp ax, word ptr [_D_00188c48]
    jge .L001093dc
    movzx ebx, ax
    imul ebx, dword ptr ["W?research_names$npnpna"]
    add ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 0
.L00108f89:
    movzx ebx, word ptr [ecx + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x48]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x30], ebx
    mov bx, word ptr [edi + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x49]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x34], ebx
    mov dword ptr [ebp - 0x40], ebx
    mov ax, word ptr [ecx]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv dword ptr [ebp - 0x30]
    mov dword ptr [ebp - 0x24], eax
    mov ax, word ptr [edi]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv dword ptr [ebp - 0x34]
    cmp eax, dword ptr [ebp - 0x24]
    jle .L001093dc
    mov dword ptr [ebp - 0x28], eax
    movzx ebx, word ptr [edi + 2]
    sub bx, word ptr [ecx + 2]
    mov ax, word ptr [edi]
    sub ax, word ptr [ecx]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x2c], eax
    mov dword ptr [ebp - 0x38], ebx
    mov ax, word ptr [ecx]
    shl eax, 0x10
    mov dword ptr [ebp - 0x3c], eax
    mov ax, word ptr [esi]
    shl eax, 0x10
    {load} mov ebx, eax
    cmp byte ptr [ebp - 0x47], 0
    je .L001090b1
    movsx edi, word ptr [ebp - 0x46]
    neg edi
    sub dword ptr [ebp - 0x40], edi
    jle .L001093dc
    mov dword ptr [ebp - 0x44], edi
    cmp di, word ptr [ebp - 0x30]
    js .L00109072
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x30]
    {load} add ebx, edi
    mov eax, dword ptr [ebp - 0x3c]
    mov edi, dword ptr [ebp - 0x44]
    sub edi, dword ptr [ebp - 0x30]
    mov dword ptr [ebp - 0x44], edi
    sub word ptr [ebp - 0x38], di
    imul edi, dword ptr [ebp - 0x2c]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x44]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x49], 0
    je .L00109067
    mov edi, dword ptr [_D_00188c48]
    mov word ptr [ebp - 0x38], di
    mov dword ptr [ebp - 0x40], edi
.L00109067:
    lea edi, ["_lbVesaGran"+0x2]
    jmp .L001090f9
.L00109072:
    mov edi, dword ptr [ebp - 0x44]
    sub word ptr [ebp - 0x30], di
    imul edi, dword ptr [ebp - 0x24]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x44]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x49], 0
    je .L001090dc
    mov edi, dword ptr [_D_00188c48]
    mov dword ptr [ebp - 0x40], edi
    cmp byte ptr [ebp - 0x48], 0
    je .L001090a3
    mov word ptr [ebp - 0x30], di
    jmp .L001090af
.L001090a3:
    sub di, word ptr [ebp - 0x30]
    setle byte ptr [ebp - 0x48]
    mov word ptr [ebp - 0x38], di
.L001090af:
    jmp .L001090dc
.L001090b1:
    cmp byte ptr [ebp - 0x49], 0
    je .L001090dc
    mov edi, dword ptr [_D_00188c48]
    sub di, word ptr [ebp - 0x46]
    mov dword ptr [ebp - 0x40], edi
    cmp byte ptr [ebp - 0x48], 0
    je .L001090d0
    mov word ptr [ebp - 0x30], di
    jmp .L001090dc
.L001090d0:
    sub di, word ptr [ebp - 0x30]
    setle byte ptr [ebp - 0x48]
    mov word ptr [ebp - 0x38], di
.L001090dc:
    lea edi, ["_lbVesaGran"+0x2]
.L001090e2:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x24]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x28]
    add edi, 8
    dec word ptr [ebp - 0x30]
    jne .L001090e2
    mov eax, dword ptr [ebp - 0x3c]
.L001090f9:
    cmp byte ptr [ebp - 0x48], 0
    je .L00109104
    jmp .L00109326
.L00109104:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x2c]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x28]
    add edi, 8
    dec word ptr [ebp - 0x38]
    jne .L00109104
    jmp .L00109326
.L0010911d:
    mov ax, word ptr [ecx]
    cmp ax, word ptr [esi]
    jle .L001093dc
    xchg edi, esi
    xchg ecx, edi
.L0010912d:
    mov ax, word ptr [esi + 2]
    mov word ptr [ebp - 0x46], ax
    {load} or ax, ax
    jns .L00109149
    mov ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 1
    jmp .L0010916d
.L00109149:
    cmp ax, word ptr [_D_00188c48]
    jge .L001093dc
    movzx ebx, ax
    imul ebx, dword ptr ["W?research_names$npnpna"]
    add ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 0
.L0010916d:
    movzx ebx, word ptr [ecx + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x48]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x30], ebx
    mov dword ptr [ebp - 0x40], ebx
    mov ax, word ptr [ecx]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x24], eax
    mov ax, word ptr [edi]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x28], eax
    mov ax, word ptr [esi]
    shl eax, 0x10
    {load} mov ebx, eax
    cmp byte ptr [ebp - 0x47], 0
    je .L001091ec
    movsx edi, word ptr [ebp - 0x46]
    neg edi
    sub word ptr [ebp - 0x30], di
    sub word ptr [ebp - 0x40], di
    jle .L001093dc
    mov dword ptr [ebp - 0x44], edi
    imul edi, dword ptr [ebp - 0x24]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x44]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x48], 0
    je .L00109203
    mov edi, dword ptr [_D_00188c48]
    mov dword ptr [ebp - 0x40], edi
    mov word ptr [ebp - 0x30], di
    jmp .L00109203
.L001091ec:
    cmp byte ptr [ebp - 0x48], 0
    je .L00109203
    mov edi, dword ptr [_D_00188c48]
    sub di, word ptr [ebp - 0x46]
    mov dword ptr [ebp - 0x40], edi
    mov word ptr [ebp - 0x30], di
.L00109203:
    lea edi, ["_lbVesaGran"+0x2]
.L00109209:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x24]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x28]
    add edi, 8
    dec word ptr [ebp - 0x30]
    jne .L00109209
    jmp .L00109326
.L00109222:
    mov ax, word ptr [ecx]
    cmp ax, word ptr [edi]
    jle .L001093dc
    xchg edi, esi
    xchg ecx, edi
.L00109232:
    mov ax, word ptr [esi + 2]
    mov word ptr [ebp - 0x46], ax
    {load} or ax, ax
    jns .L0010924e
    mov ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 1
    jmp .L00109272
.L0010924e:
    cmp ax, word ptr [_D_00188c48]
    jge .L001093dc
    movzx ebx, ax
    imul ebx, dword ptr ["W?research_names$npnpna"]
    add ebx, dword ptr ["_lbDisplay"+0x4]
    mov dword ptr [ebp - 0x20], ebx
    mov byte ptr [ebp - 0x47], 0
.L00109272:
    movzx ebx, word ptr [ecx + 2]
    cmp bx, word ptr [_D_00188c48]
    setg byte ptr [ebp - 0x48]
    {load} sub bx, ax
    mov dword ptr [ebp - 0x30], ebx
    mov dword ptr [ebp - 0x40], ebx
    mov ax, word ptr [ecx]
    sub ax, word ptr [esi]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x24], eax
    mov ax, word ptr [ecx]
    sub ax, word ptr [edi]
    shl eax, 0x10
    cdq
    idiv ebx
    mov dword ptr [ebp - 0x28], eax
    mov ax, word ptr [esi]
    shl eax, 0x10
    mov bx, word ptr [edi]
    shl ebx, 0x10
    cmp byte ptr [ebp - 0x47], 0
    je .L001092f5
    movsx edi, word ptr [ebp - 0x46]
    neg edi
    sub word ptr [ebp - 0x30], di
    sub word ptr [ebp - 0x40], di
    jle .L001093dc
    mov dword ptr [ebp - 0x44], edi
    imul edi, dword ptr [ebp - 0x24]
    {load} add eax, edi
    mov edi, dword ptr [ebp - 0x28]
    imul edi, dword ptr [ebp - 0x44]
    {load} add ebx, edi
    cmp byte ptr [ebp - 0x48], 0
    je .L0010930c
    mov edi, dword ptr [_D_00188c48]
    mov dword ptr [ebp - 0x40], edi
    mov word ptr [ebp - 0x30], di
    jmp .L0010930c
.L001092f5:
    cmp byte ptr [ebp - 0x48], 0
    je .L0010930c
    mov edi, dword ptr [_D_00188c48]
    sub di, word ptr [ebp - 0x46]
    mov dword ptr [ebp - 0x40], edi
    mov word ptr [ebp - 0x30], di
.L0010930c:
    lea edi, ["_lbVesaGran"+0x2]
.L00109312:
    mov dword ptr [edi], eax
    add eax, dword ptr [ebp - 0x24]
    mov dword ptr [edi + 4], ebx
    add ebx, dword ptr [ebp - 0x28]
    add edi, 8
    dec word ptr [ebp - 0x30]
    jne .L00109312
.L00109326:
    lea esi, ["_lbVesaGran"+0x2]
    mov edx, dword ptr [ebp - 0x20]
    mov eax, dword ptr [_D_00188c40]
    dec eax
    imul eax, dword ptr ["W?research_names$npnpna"]
    add eax, dword ptr [_D_00188c3c]
    {load} add edx, eax
    {load} xor eax, eax
    mov al, byte ptr [ebp + 0x10]
    {load} xor ecx, ecx
.L0010934b:
    {load} xor ebx, ebx
    mov bx, word ptr [esi + 2]
    mov cx, word ptr [esi + 6]
    add edx, dword ptr ["W?research_names$npnpna"]
    {load} or bx, bx
    jns .L0010937f
    {load} or cx, cx
    jle .L001093d0
    cmp cx, word ptr [_D_00188c44]
    jle .L00109375
    mov cx, word ptr [_D_00188c44]
.L00109375:
    {load} mov edi, edx
    mov ebx, dword ptr ["_lbDisplay"+0x8]
    jmp .L0010939d
.L0010937f:
    cmp cx, word ptr [_D_00188c44]
    jle .L0010938f
    mov cx, word ptr [_D_00188c44]
.L0010938f:
    {load} sub cx, bx
    jle .L001093d0
    lea edi, [ebx + edx]
    mov ebx, dword ptr ["_lbDisplay"+0x8]
.L0010939d:
    test word ptr [_D_00188c74], 4
    je .L001093b6
    mov ah, byte ptr [ebp + 0x10]
.L001093ab:
    mov al, byte ptr [edi]
    mov al, byte ptr [eax + ebx]
    stosb byte ptr es:[edi], al
    dec ecx
    jne .L001093ab
    jmp .L001093d0
.L001093b6:
    test word ptr [_D_00188c74], 8
    je .L001093ce
.L001093c1:
    mov ah, byte ptr [edi]
    mov ah, byte ptr [eax + ebx]
    mov byte ptr [edi], ah
    inc edi
    dec ecx
    jne .L001093c1
    jmp .L001093d0
.L001093ce:
    rep stosb byte ptr es:[edi], al
.L001093d0:
    add esi, 8
    dec dword ptr [ebp - 0x40]
    jne .L0010934b
.L001093dc:
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    leave
    ret 0xc
.size "LbDrawTriangleFilled_",.-"LbDrawTriangleFilled_"
