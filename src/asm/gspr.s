.intel_syntax noprefix

// FUNCTION: 0x00102c80

.section _TEXT,"ax",@progbits,unique,1059968
.globl "LbSpriteDraw_"
.type "LbSpriteDraw_",@function
"LbSpriteDraw_":
    .byte 0x60  # pushal
    add esp, -0xc
    mov cl, byte ptr [ebx + 4]
    {load} or cl, cl
    je .L00107871
    mov cl, byte ptr [ebx + 5]
    {load} or cl, cl
    je .L00107871
    mov esi, dword ptr [ebx]
    {load} xor ecx, ecx
    mov cx, word ptr [_D_00188c74]
    test ecx, 2
    jne .L00102d0b
    push eax
    mov eax, dword ptr ["W?research_names$npnpna"]
    mov dword ptr [esp + 0xc], eax
    {load} xor eax, eax
    mov al, byte ptr [ebx + 5]
    {load} mov ebp, eax
    {load} add eax, edx
    mov ecx, dword ptr [_D_00188c48]
    {load} cmp eax, ecx
    jle .L00102cd4
    {load} sub ebp, eax
    {load} add ebp, ecx
    jle .L0010786c
.L00102cd4:
    {load} or edx, edx
    jns .L00102d08
    {load} add ebp, edx
    jle .L0010786c
    push ebp
    {load} mov ebp, edx
    neg ebp
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00102ce8:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00102d00
    js .L00102cf8
    inc esi
    {load} add esi, ecx
    jmp .L00102ce8
    .byte 0x90
.L00102cf8:
    inc esi
    neg cl
    jmp .L00102ce8
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00102d00:
    inc esi
    dec ebp
    jne .L00102ce8
    {load} xor edx, edx
    pop ebp
    nop
.L00102d08:
    pop eax
    jmp .L00102d74
.L00102d0b:
    push eax
    mov eax, dword ptr ["W?research_names$npnpna"]
    neg eax
    mov dword ptr [esp + 0xc], eax
    {load} xor eax, eax
    mov al, byte ptr [ebx + 5]
    {load} mov ebp, eax
    {load} mov eax, edx
    {load} add edx, ebp
    dec edx
    {load} or eax, eax
    je .L00102d34
    jns .L00102d34
    {load} add ebp, eax
    jle .L0010786c
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00102d34:
    mov ecx, dword ptr [_D_00188c48]
    dec ecx
    {load} cmp edx, ecx
    jle .L00102d70
    {load} sub ebp, edx
    {load} add ebp, ecx
    jle .L0010786c
    push ebp
    {load} mov ebp, edx
    {load} sub ebp, ecx
    push ecx
    nop
.L00102d50:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00102d68
    js .L00102d60
    inc esi
    {load} add esi, ecx
    jmp .L00102d50
    .byte 0x90
.L00102d60:
    inc esi
    neg cl
    jmp .L00102d50
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00102d68:
    inc esi
    dec ebp
    jne .L00102d50
    pop ecx
    {load} mov edx, ecx
    pop ebp
.L00102d70:
    pop eax
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00102d74:
    {load} xor ecx, ecx
    mov cx, word ptr [_D_00188c74]
    test ecx, 1
    jne .L001050c0
    push edx
    {load} xor edx, edx
    mov dword ptr [esp + 8], edx
    mov ecx, dword ptr [_D_00188c44]
    {load} cmp eax, ecx
    jge .L0010786c
    {load} xor edx, edx
    mov dl, byte ptr [ebx + 4]
    mov dword ptr [esp + 4], edx
    {load} add edx, eax
    jle .L0010786c
    {load} cmp edx, ecx
    jg .L00104d74
    {load} or eax, eax
    js .L00104d74
    pop edx
    add eax, dword ptr [_D_00188c3c]
    add edx, dword ptr [_D_00188c40]
    mov ecx, dword ptr ["W?research_names$npnpna"]
    imul edx, ecx
    {load} add edx, eax
    add edx, dword ptr ["_lbDisplay"+0x4]
    {load} mov edi, edx
    {load} xor eax, eax
    {load} xor ebx, ebx
    mov bx, word ptr [_D_00188c74]
    test ebx, 4
    jne .L00104cf4
    test ebx, 8
    jne .L00104d34
    {load} mov ebx, ebp
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L00102e10
    .byte 0x8b; .byte 0xc0
.L00102e0c:
    neg cl
    {load} add edx, ecx
.L00102e10:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L00102e0c
    je .L00104ce0
    {load} mov eax, ecx
    shl eax, 4
    {load} mov ebp, edx
    and ebp, 3
    jmp dword ptr [eax + ebp*4 + .L00107878]
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xeb; .byte .L00102e10-.-1
.L00102e34:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102e3e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102e4e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xeb; .byte .L00102e10-.-1
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xeb; .byte .L00102e10-.-1
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L00102e97:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L00102ee2:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102f01:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L00102f3f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102f5e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102f83:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102fae:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102fbb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102fce:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00102fe7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103006:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103019:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103032:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103051:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103076:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010308f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001030ae:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001030d3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001030fe:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010311d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103142:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx + 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010316d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx + 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010319e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001031b1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001031ca:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001031e9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010320e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103227:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103246:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010326b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103296:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001032b5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001032da:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103305:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103336:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx + 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010335b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx + 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103386:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx + 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001033b7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx + 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001033ee:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103407:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103426:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010344b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103476:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103495:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001034ba:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001034e5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103516:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010353b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103566:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103597:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001035ce:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx + 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001035f9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx + 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010362a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx + 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103661:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx + 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010369e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001036bd:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001036e2:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010370d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010373e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103763:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010378e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001037bf:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001037f6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103821:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103852:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103889:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001038c6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx + 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001038f7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx + 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010392e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx + 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010396b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx + 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001039ae:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001039d3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001039fe:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103a2f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103a66:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103a91:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103ac2:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103af9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103b36:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103b67:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103b9e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103bdb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103c1e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx + 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103c55:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx + 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103c92:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx + 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103cd5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx + 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103d1e:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103d49:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103d7a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103db1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov eax, dword ptr [esi + 0x17]
    mov dword ptr [edx + 0x17], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103dee:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103e1f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103e56:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103e93:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov eax, dword ptr [esi + 0x17]
    mov dword ptr [edx + 0x17], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103ed6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103f0d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103f4a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103f8d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov eax, dword ptr [esi + 0x17]
    mov dword ptr [edx + 0x17], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00103fd6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx + 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00104013:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx + 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00104056:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx + 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010409f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov eax, dword ptr [esi + 0x17]
    mov dword ptr [edx + 0x17], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx + 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx + 0x1d], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001040ee:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov eax, dword ptr [esi + 0x18]
    mov dword ptr [edx + 0x18], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010411f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov eax, dword ptr [esi + 0x19]
    mov dword ptr [edx + 0x19], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00104156:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    mov eax, dword ptr [esi + 0x1a]
    mov dword ptr [edx + 0x1a], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00104193:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov eax, dword ptr [esi + 3]
    mov dword ptr [edx + 3], eax
    mov eax, dword ptr [esi + 7]
    mov dword ptr [edx + 7], eax
    mov eax, dword ptr [esi + 0xb]
    mov dword ptr [edx + 0xb], eax
    mov eax, dword ptr [esi + 0xf]
    mov dword ptr [edx + 0xf], eax
    mov eax, dword ptr [esi + 0x13]
    mov dword ptr [edx + 0x13], eax
    mov eax, dword ptr [esi + 0x17]
    mov dword ptr [edx + 0x17], eax
    mov eax, dword ptr [esi + 0x1b]
    mov dword ptr [edx + 0x1b], eax
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L001041d6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov eax, dword ptr [esi + 0x18]
    mov dword ptr [edx + 0x18], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010420d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov eax, dword ptr [esi + 0x19]
    mov dword ptr [edx + 0x19], eax
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx + 0x1d], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L0010424a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov eax, dword ptr [esi + 2]
    mov dword ptr [edx + 2], eax
    mov eax, dword ptr [esi + 6]
    mov dword ptr [edx + 6], eax
    mov eax, dword ptr [esi + 0xa]
    mov dword ptr [edx + 0xa], eax
    mov eax, dword ptr [esi + 0xe]
    mov dword ptr [edx + 0xe], eax
    mov eax, dword ptr [esi + 0x12]
    mov dword ptr [edx + 0x12], eax
    mov eax, dword ptr [esi + 0x16]
    mov dword ptr [edx + 0x16], eax
    mov eax, dword ptr [esi + 0x1a]
    mov dword ptr [edx + 0x1a], eax
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx + 0x1e], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x89; .byte 0x42; .byte 0x13
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x89; .byte 0x42; .byte 0x17
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x89; .byte 0x42; .byte 0x1b
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L001042d6:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov eax, dword ptr [esi + 0x18]
    mov dword ptr [edx + 0x18], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx + 0x1d], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
.L00104313:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    mov dword ptr [edx + 1], eax
    mov eax, dword ptr [esi + 5]
    mov dword ptr [edx + 5], eax
    mov eax, dword ptr [esi + 9]
    mov dword ptr [edx + 9], eax
    mov eax, dword ptr [esi + 0xd]
    mov dword ptr [edx + 0xd], eax
    mov eax, dword ptr [esi + 0x11]
    mov dword ptr [edx + 0x11], eax
    mov eax, dword ptr [esi + 0x15]
    mov dword ptr [edx + 0x15], eax
    mov eax, dword ptr [esi + 0x19]
    mov dword ptr [edx + 0x19], eax
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx + 0x1d], al
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx + 0x1e], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x8b; .byte 0x46; .byte 0x0e
    .byte 0x89; .byte 0x42; .byte 0x0e
    .byte 0x8b; .byte 0x46; .byte 0x12
    .byte 0x89; .byte 0x42; .byte 0x12
    .byte 0x8b; .byte 0x46; .byte 0x16
    .byte 0x89; .byte 0x42; .byte 0x16
    .byte 0x8b; .byte 0x46; .byte 0x1a
    .byte 0x89; .byte 0x42; .byte 0x1a
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0x1e
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x89; .byte 0x42; .byte 0x13
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x89; .byte 0x42; .byte 0x17
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x89; .byte 0x42; .byte 0x1b
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0x20
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L001043ee:
    mov eax, dword ptr [esi]
    mov dword ptr [edx], eax
    mov eax, dword ptr [esi + 4]
    mov dword ptr [edx + 4], eax
    mov eax, dword ptr [esi + 8]
    mov dword ptr [edx + 8], eax
    mov eax, dword ptr [esi + 0xc]
    mov dword ptr [edx + 0xc], eax
    mov eax, dword ptr [esi + 0x10]
    mov dword ptr [edx + 0x10], eax
    mov eax, dword ptr [esi + 0x14]
    mov dword ptr [edx + 0x14], eax
    mov eax, dword ptr [esi + 0x18]
    mov dword ptr [edx + 0x18], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx + 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx + 0x1d], al
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx + 0x1e], al
    {load} add esi, ecx
    {load} add edx, ecx
    jmp .L00102e10
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x01
    .byte 0x89; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x05
    .byte 0x89; .byte 0x42; .byte 0x05
    .byte 0x8b; .byte 0x46; .byte 0x09
    .byte 0x89; .byte 0x42; .byte 0x09
    .byte 0x8b; .byte 0x46; .byte 0x0d
    .byte 0x89; .byte 0x42; .byte 0x0d
    .byte 0x8b; .byte 0x46; .byte 0x11
    .byte 0x89; .byte 0x42; .byte 0x11
    .byte 0x8b; .byte 0x46; .byte 0x15
    .byte 0x89; .byte 0x42; .byte 0x15
    .byte 0x8b; .byte 0x46; .byte 0x19
    .byte 0x89; .byte 0x42; .byte 0x19
    .byte 0x8a; .byte 0x46; .byte 0x1d
    .byte 0x88; .byte 0x42; .byte 0x1d
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0x1e
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x89; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x89; .byte 0x42; .byte 0x06
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x89; .byte 0x42; .byte 0x0a
    .byte 0x8b; .byte 0x46; .byte 0x0e
    .byte 0x89; .byte 0x42; .byte 0x0e
    .byte 0x8b; .byte 0x46; .byte 0x12
    .byte 0x89; .byte 0x42; .byte 0x12
    .byte 0x8b; .byte 0x46; .byte 0x16
    .byte 0x89; .byte 0x42; .byte 0x16
    .byte 0x8b; .byte 0x46; .byte 0x1a
    .byte 0x89; .byte 0x42; .byte 0x1a
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0x1e
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0x20
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0x01
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x89; .byte 0x42; .byte 0x03
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x89; .byte 0x42; .byte 0x07
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x89; .byte 0x42; .byte 0x0b
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x89; .byte 0x42; .byte 0x0f
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x89; .byte 0x42; .byte 0x13
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x89; .byte 0x42; .byte 0x17
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x89; .byte 0x42; .byte 0x1b
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0x1f
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0x20
    .byte 0x8a; .byte 0x46; .byte 0x21
    .byte 0x88; .byte 0x42; .byte 0x21
    .byte 0x03; .byte 0xf1
    .byte 0x03; .byte 0xd1
    .byte 0xe9; .long .L00102e10-.-4
.L0010451e:
    sub ecx, 0
    shr ecx, 2
    je .L00104576
.L00104526:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104576
    jmp .L00104526
.L00104576:
    jmp .L00102e10
.L0010457b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L001045e3
.L00104587:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001045e3
    jmp .L00104587
.L001045e3:
    add esi, 1
    add edx, 1
    jmp .L00102e10
.L001045ee:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L0010465c
.L00104600:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L0010465c
    jmp .L00104600
.L0010465c:
    add esi, 2
    add edx, 2
    jmp .L00102e10
.L00104667:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L001046db
.L0010467f:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001046db
    jmp .L0010467f
.L001046db:
    add esi, 3
    add edx, 3
    jmp .L00102e10
.L001046e6:
    sub ecx, 0
    shr ecx, 2
    je .L0010473e
.L001046ee:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L0010473e
    jmp .L001046ee
.L0010473e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    add esi, 1
    add edx, 1
    jmp .L00102e10
.L0010474d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L001047b5
.L00104759:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L001047b5
    jmp .L00104759
.L001047b5:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    add esi, 2
    add edx, 2
    jmp .L00102e10
.L001047c6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00104834
.L001047d8:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104834
    jmp .L001047d8
.L00104834:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L00102e10
.L00104845:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L001048b9
.L0010485d:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L001048b9
    jmp .L0010485d
.L001048b9:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L00102e10
.L001048ca:
    sub ecx, 0
    shr ecx, 2
    je .L00104922
.L001048d2:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104922
    jmp .L001048d2
.L00104922:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    add esi, 2
    add edx, 2
    jmp .L00102e10
.L00104937:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L0010499f
.L00104943:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L0010499f
    jmp .L00104943
.L0010499f:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L00102e10
.L001049b6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00104a24
.L001049c8:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104a24
    jmp .L001049c8
.L00104a24:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L00102e10
.L00104a3b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00104aaf
.L00104a53:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104aaf
    jmp .L00104a53
.L00104aaf:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    add esi, 5
    add edx, 5
    jmp .L00102e10
.L00104ac6:
    sub ecx, 0
    shr ecx, 2
    je .L00104b1e
.L00104ace:
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    mov eax, dword ptr [esi]
    add esi, 4
    mov dword ptr [edx], eax
    add edx, 4
    dec ecx
    je .L00104b1e
    jmp .L00104ace
.L00104b1e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    add esi, 3
    add edx, 3
    jmp .L00102e10
.L00104b39:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00104ba1
.L00104b45:
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    mov eax, dword ptr [esi + 1]
    add esi, 4
    mov dword ptr [edx + 1], eax
    add edx, 4
    dec ecx
    je .L00104ba1
    jmp .L00104b45
.L00104ba1:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    add esi, 4
    add edx, 4
    jmp .L00102e10
.L00104bbe:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00104c2c
.L00104bd0:
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    mov eax, dword ptr [esi + 2]
    add esi, 4
    mov dword ptr [edx + 2], eax
    add edx, 4
    dec ecx
    je .L00104c2c
    jmp .L00104bd0
.L00104c2c:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    add esi, 5
    add edx, 5
    jmp .L00102e10
.L00104c49:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx + 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx + 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00104cbd
.L00104c61:
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    mov eax, dword ptr [esi + 3]
    add esi, 4
    mov dword ptr [edx + 3], eax
    add edx, 4
    dec ecx
    je .L00104cbd
    jmp .L00104c61
.L00104cbd:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx + 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx + 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx + 5], al
    add esi, 6
    add edx, 6
    jmp .L00102e10
    .byte 0xe9; .long .L00102e10-.-4
    .byte 0x90
.L00104ce0:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L00102e10
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00104cf4:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L00104d08
    .byte 0x8b; .byte 0xc0
.L00104d04:
    neg cl
    {load} add edx, ecx
.L00104d08:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L00104d04
    je .L00104d24
    {load} xor eax, eax
    nop
.L00104d14:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L00104d14
    jmp .L00104d08
.L00104d24:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L00104d08
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00104d34:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L00104d48
    .byte 0x8b; .byte 0xc0
.L00104d44:
    neg cl
    {load} add edx, ecx
.L00104d48:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L00104d44
    je .L00104d64
    {load} xor eax, eax
    nop
.L00104d54:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L00104d54
    jmp .L00104d48
.L00104d64:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L00104d48
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00104d74:
    {load} cmp edx, ecx
    jle .L00104d80
    {load} sub ecx, eax
    mov dword ptr [esp + 4], ecx
    {load} mov eax, eax
.L00104d80:
    {load} or eax, eax
    jns .L00104d90
    neg eax
    mov dword ptr [esp + 8], eax
    sub dword ptr [esp + 4], eax
    {load} xor eax, eax
.L00104d90:
    pop edx
    add eax, dword ptr [_D_00188c3c]
    add edx, dword ptr [_D_00188c40]
    mov ecx, dword ptr ["W?research_names$npnpna"]
    imul edx, ecx
    {load} add edx, eax
    add edx, dword ptr ["_lbDisplay"+0x4]
    {load} mov edi, edx
    {load} xor eax, eax
    {load} xor ebx, ebx
    mov bx, word ptr [_D_00188c74]
    test ebx, 4
    jne .L00104f08
    test ebx, 8
    jne .L00104fe4
    {load} mov ebx, ebp
.L00104dd5:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L00104e2c
    {load} mov eax, eax
.L00104de4:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104ef4
    js .L00104e10
    inc esi
    {load} cmp ecx, ebp
    jle .L00104e08
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    je .L00104ed4
    jmp .L00104e40
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00104e08:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L00104de4
    .byte 0x8b; .byte 0xc0
.L00104e10:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L00104e28
    {load} sub ecx, ebp
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00104ed4
    jmp .L00104e2c
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00104e28:
    {load} sub ebp, ecx
    jmp .L00104de4
.L00104e2c:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104ef4
    js .L00104ee4
    inc esi
    nop
.L00104e40:
    {load} cmp ecx, edi
    jge .L00104e88
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L00104e4c:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104e84
    jmp .L00104e4c
.L00104e84:
    jmp .L00104e2c
    .byte 0x8b; .byte 0xc0
.L00104e88:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00104e98:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    inc edx
    dec ecx
    je .L00104ed0
    jmp .L00104e98
.L00104ed0:
    pop ebp
    {load} add esi, ebp
    nop
.L00104ed4:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104ef4
    js .L00104ee0
    {load} add esi, ecx
.L00104ee0:
    inc esi
    jmp .L00104ed4
    .byte 0x90
.L00104ee4:
    inc esi
    neg cl
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00104ed4
    jmp .L00104e2c
    .byte 0x8b; .byte 0xc0
.L00104ef4:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L00104dd5
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00104f08:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L00104f10:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L00104f64
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00104f20:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104fd0
    js .L00104f4c
    inc esi
    {load} cmp ecx, ebp
    jle .L00104f44
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    .byte 0x0f; .byte 0x84; .long .L00104fb4-.-4  # je 0x104fb4
    jmp .L00104f70
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00104f44:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L00104f20
    .byte 0x8b; .byte 0xc0
.L00104f4c:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L00104f60
    {load} sub ecx, ebp
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00104fb4
    jmp .L00104f64
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00104f60:
    {load} sub ebp, ecx
    jmp .L00104f20
.L00104f64:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104fd0
    js .L00104fc4
    inc esi
    nop
.L00104f70:
    {load} cmp ecx, edi
    jge .L00104f90
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    {load} mov eax, eax
.L00104f80:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L00104f80
    jmp .L00104f64
.L00104f90:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    nop
.L00104fa0:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L00104fa0
    pop ebp
    {load} add esi, ebp
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00104fb4:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00104fd0
    js .L00104fc0
    {load} add esi, ecx
.L00104fc0:
    inc esi
    jmp .L00104fb4
    .byte 0x90
.L00104fc4:
    inc esi
    neg cl
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00104fb4
    jmp .L00104f64
    .byte 0x90
.L00104fd0:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L00104f10
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00104fe4:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L00104fec:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L00105040
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00104ffc:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001050ac
    js .L00105028
    inc esi
    {load} cmp ecx, ebp
    jle .L00105020
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    .byte 0x0f; .byte 0x84; .long .L00105090-.-4  # je 0x105090
    jmp .L0010504c
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L00105020:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L00104ffc
    .byte 0x8b; .byte 0xc0
.L00105028:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L0010503c
    {load} sub ecx, ebp
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00105090
    jmp .L00105040
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010503c:
    {load} sub ebp, ecx
    jmp .L00104ffc
.L00105040:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001050ac
    js .L001050a0
    inc esi
    nop
.L0010504c:
    {load} cmp ecx, edi
    jge .L0010506c
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    {load} mov eax, eax
.L0010505c:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L0010505c
    jmp .L00105040
.L0010506c:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    nop
.L0010507c:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    inc edx
    dec ecx
    jne .L0010507c
    pop ebp
    {load} add esi, ebp
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00105090:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001050ac
    js .L0010509c
    {load} add esi, ecx
.L0010509c:
    inc esi
    jmp .L00105090
    .byte 0x90
.L001050a0:
    inc esi
    neg cl
    {load} add edx, ecx
    {load} sub edi, ecx
    jle .L00105090
    jmp .L00105040
    .byte 0x90
.L001050ac:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L00104fec
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L001050c0:
    push edx
    {load} xor edx, edx
    mov dword ptr [esp + 8], edx
    mov ecx, dword ptr [_D_00188c44]
    {load} cmp eax, ecx
    jge .L0010786c
    {load} xor edx, edx
    mov dl, byte ptr [ebx + 4]
    dec eax
    {load} add eax, edx
    jl .L0010786c
    mov dword ptr [esp + 4], edx
    {load} cmp eax, ecx
    jge .L00107520
    {load} cmp eax, edx
    jl .L00107520
    pop edx
    add eax, dword ptr [_D_00188c3c]
    add edx, dword ptr [_D_00188c40]
    mov ecx, dword ptr ["W?research_names$npnpna"]
    imul edx, ecx
    {load} add edx, eax
    add edx, dword ptr ["_lbDisplay"+0x4]
    {load} mov edi, edx
    {load} xor eax, eax
    {load} xor ebx, ebx
    mov bx, word ptr [_D_00188c74]
    test ebx, 4
    jne .L001074a0
    test ebx, 8
    jne .L001074e0
    {load} mov ebx, ebp
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L00105148
    .byte 0x8b; .byte 0xc0
.L00105144:
    neg cl
    {load} sub edx, ecx
.L00105148:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L00105144
    je .L0010748c
    {load} mov eax, ecx
    shl eax, 4
    {load} mov ebp, edx
    and ebp, 3
    jmp dword ptr [eax + ebp*4 + .L00108278]
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xeb; .byte .L00105148-.-1
.L0010516c:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105176:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105186:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xeb; .byte .L00105148-.-1
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xeb; .byte .L00105148-.-1
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L001051cf:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L0010521a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105239:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L00105277:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105296:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001052bb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001052e6:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001052f6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010530b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105326:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105347:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010535d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105378:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105399:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001053c0:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001053dc:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001053fd:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105424:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105451:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105473:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010549a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov al, byte ptr [esi + 6]
    mov byte ptr [edx - 6], al
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001054c7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov al, byte ptr [esi + 7]
    mov byte ptr [edx - 7], al
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001054fa:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105512:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010552f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105552:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010557b:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105599:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001055bc:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001055e5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105614:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105638:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105661:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105690:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001056c5:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov al, byte ptr [esi + 8]
    mov byte ptr [edx - 8], al
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001056ef:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov al, byte ptr [esi + 9]
    mov byte ptr [edx - 9], al
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010571e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov al, byte ptr [esi + 0xa]
    mov byte ptr [edx - 0xa], al
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105753:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov al, byte ptr [esi + 0xb]
    mov byte ptr [edx - 0xb], al
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010578e:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001057ae:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001057d3:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001057fe:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010582f:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105855:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105880:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001058b1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001058e8:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105914:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105945:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010597c:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001059b9:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov al, byte ptr [esi + 0xc]
    mov byte ptr [edx - 0xc], al
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001059eb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov al, byte ptr [esi + 0xd]
    mov byte ptr [edx - 0xd], al
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105a22:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov al, byte ptr [esi + 0xe]
    mov byte ptr [edx - 0xe], al
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105a5f:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov al, byte ptr [esi + 0xf]
    mov byte ptr [edx - 0xf], al
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105aa2:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105aca:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105af7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105b2a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105b63:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105b91:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105bc4:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105bfd:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105c3c:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105c70:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105ca9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105ce8:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105d2d:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov al, byte ptr [esi + 0x10]
    mov byte ptr [edx - 0x10], al
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105d67:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov al, byte ptr [esi + 0x11]
    mov byte ptr [edx - 0x11], al
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105da6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov al, byte ptr [esi + 0x12]
    mov byte ptr [edx - 0x12], al
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105deb:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov al, byte ptr [esi + 0x13]
    mov byte ptr [edx - 0x13], al
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105e36:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105e66:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105e9b:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105ed6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105f17:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105f4d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105f88:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00105fc9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106010:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010604c:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010608d:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001060d4:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106121:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov al, byte ptr [esi + 0x14]
    mov byte ptr [edx - 0x14], al
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106163:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov al, byte ptr [esi + 0x15]
    mov byte ptr [edx - 0x15], al
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001061aa:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov al, byte ptr [esi + 0x16]
    mov byte ptr [edx - 0x16], al
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001061f7:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov al, byte ptr [esi + 0x17]
    mov byte ptr [edx - 0x17], al
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010624a:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106282:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001062bf:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106302:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov eax, dword ptr [esi + 0x17]
    bswap eax
    mov dword ptr [edx - 0x1a], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010634b:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106389:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001063cc:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106415:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov eax, dword ptr [esi + 0x17]
    bswap eax
    mov dword ptr [edx - 0x1a], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106464:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001064a8:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001064f1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106540:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov eax, dword ptr [esi + 0x17]
    bswap eax
    mov dword ptr [edx - 0x1a], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106595:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov al, byte ptr [esi + 0x18]
    mov byte ptr [edx - 0x18], al
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001065df:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov al, byte ptr [esi + 0x19]
    mov byte ptr [edx - 0x19], al
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010662e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    mov al, byte ptr [esi + 0x1a]
    mov byte ptr [edx - 0x1a], al
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106683:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov eax, dword ptr [esi + 0x17]
    bswap eax
    mov dword ptr [edx - 0x1a], eax
    mov al, byte ptr [esi + 0x1b]
    mov byte ptr [edx - 0x1b], al
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx - 0x1d], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001066de:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov eax, dword ptr [esi + 0x18]
    bswap eax
    mov dword ptr [edx - 0x1b], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L0010671e:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov eax, dword ptr [esi + 0x19]
    bswap eax
    mov dword ptr [edx - 0x1c], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106763:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    mov eax, dword ptr [esi + 0x1a]
    bswap eax
    mov dword ptr [edx - 0x1d], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001067ae:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov eax, dword ptr [esi + 3]
    bswap eax
    mov dword ptr [edx - 6], eax
    mov eax, dword ptr [esi + 7]
    bswap eax
    mov dword ptr [edx - 0xa], eax
    mov eax, dword ptr [esi + 0xb]
    bswap eax
    mov dword ptr [edx - 0xe], eax
    mov eax, dword ptr [esi + 0xf]
    bswap eax
    mov dword ptr [edx - 0x12], eax
    mov eax, dword ptr [esi + 0x13]
    bswap eax
    mov dword ptr [edx - 0x16], eax
    mov eax, dword ptr [esi + 0x17]
    bswap eax
    mov dword ptr [edx - 0x1a], eax
    mov eax, dword ptr [esi + 0x1b]
    bswap eax
    mov dword ptr [edx - 0x1e], eax
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L001067ff:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov eax, dword ptr [esi + 0x18]
    bswap eax
    mov dword ptr [edx - 0x1b], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106845:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov eax, dword ptr [esi + 0x19]
    bswap eax
    mov dword ptr [edx - 0x1c], eax
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx - 0x1d], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106890:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov eax, dword ptr [esi + 2]
    bswap eax
    mov dword ptr [edx - 5], eax
    mov eax, dword ptr [esi + 6]
    bswap eax
    mov dword ptr [edx - 9], eax
    mov eax, dword ptr [esi + 0xa]
    bswap eax
    mov dword ptr [edx - 0xd], eax
    mov eax, dword ptr [esi + 0xe]
    bswap eax
    mov dword ptr [edx - 0x11], eax
    mov eax, dword ptr [esi + 0x12]
    bswap eax
    mov dword ptr [edx - 0x15], eax
    mov eax, dword ptr [esi + 0x16]
    bswap eax
    mov dword ptr [edx - 0x19], eax
    mov eax, dword ptr [esi + 0x1a]
    bswap eax
    mov dword ptr [edx - 0x1d], eax
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx - 0x1e], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfa
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf6
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf2
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xee
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xea
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe6
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L00106938:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov eax, dword ptr [esi + 0x18]
    bswap eax
    mov dword ptr [edx - 0x1b], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx - 0x1d], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
.L00106984:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov eax, dword ptr [esi + 1]
    bswap eax
    mov dword ptr [edx - 4], eax
    mov eax, dword ptr [esi + 5]
    bswap eax
    mov dword ptr [edx - 8], eax
    mov eax, dword ptr [esi + 9]
    bswap eax
    mov dword ptr [edx - 0xc], eax
    mov eax, dword ptr [esi + 0xd]
    bswap eax
    mov dword ptr [edx - 0x10], eax
    mov eax, dword ptr [esi + 0x11]
    bswap eax
    mov dword ptr [edx - 0x14], eax
    mov eax, dword ptr [esi + 0x15]
    bswap eax
    mov dword ptr [edx - 0x18], eax
    mov eax, dword ptr [esi + 0x19]
    bswap eax
    mov dword ptr [edx - 0x1c], eax
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx - 0x1d], al
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx - 0x1e], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfb
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf7
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf3
    .byte 0x8b; .byte 0x46; .byte 0x0e
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xef
    .byte 0x8b; .byte 0x46; .byte 0x12
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xeb
    .byte 0x8b; .byte 0x46; .byte 0x16
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe7
    .byte 0x8b; .byte 0x46; .byte 0x1a
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe3
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfa
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf6
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf2
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xee
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xea
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe6
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0xe0
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L00106a89:
    mov eax, dword ptr [esi]
    bswap eax
    mov dword ptr [edx - 3], eax
    mov eax, dword ptr [esi + 4]
    bswap eax
    mov dword ptr [edx - 7], eax
    mov eax, dword ptr [esi + 8]
    bswap eax
    mov dword ptr [edx - 0xb], eax
    mov eax, dword ptr [esi + 0xc]
    bswap eax
    mov dword ptr [edx - 0xf], eax
    mov eax, dword ptr [esi + 0x10]
    bswap eax
    mov dword ptr [edx - 0x13], eax
    mov eax, dword ptr [esi + 0x14]
    bswap eax
    mov dword ptr [edx - 0x17], eax
    mov eax, dword ptr [esi + 0x18]
    bswap eax
    mov dword ptr [edx - 0x1b], eax
    mov al, byte ptr [esi + 0x1c]
    mov byte ptr [edx - 0x1c], al
    mov al, byte ptr [esi + 0x1d]
    mov byte ptr [edx - 0x1d], al
    mov al, byte ptr [esi + 0x1e]
    mov byte ptr [edx - 0x1e], al
    {load} add esi, ecx
    {load} sub edx, ecx
    jmp .L00105148
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8b; .byte 0x46; .byte 0x01
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfc
    .byte 0x8b; .byte 0x46; .byte 0x05
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf8
    .byte 0x8b; .byte 0x46; .byte 0x09
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf4
    .byte 0x8b; .byte 0x46; .byte 0x0d
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf0
    .byte 0x8b; .byte 0x46; .byte 0x11
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xec
    .byte 0x8b; .byte 0x46; .byte 0x15
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe8
    .byte 0x8b; .byte 0x46; .byte 0x19
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe4
    .byte 0x8a; .byte 0x46; .byte 0x1d
    .byte 0x88; .byte 0x42; .byte 0xe3
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8b; .byte 0x46; .byte 0x02
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfb
    .byte 0x8b; .byte 0x46; .byte 0x06
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf7
    .byte 0x8b; .byte 0x46; .byte 0x0a
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf3
    .byte 0x8b; .byte 0x46; .byte 0x0e
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xef
    .byte 0x8b; .byte 0x46; .byte 0x12
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xeb
    .byte 0x8b; .byte 0x46; .byte 0x16
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe7
    .byte 0x8b; .byte 0x46; .byte 0x1a
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe3
    .byte 0x8a; .byte 0x46; .byte 0x1e
    .byte 0x88; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0xe0
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
    .byte 0x8a; .byte 0x06
    .byte 0x88; .byte 0x02
    .byte 0x8a; .byte 0x46; .byte 0x01
    .byte 0x88; .byte 0x42; .byte 0xff
    .byte 0x8a; .byte 0x46; .byte 0x02
    .byte 0x88; .byte 0x42; .byte 0xfe
    .byte 0x8b; .byte 0x46; .byte 0x03
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xfa
    .byte 0x8b; .byte 0x46; .byte 0x07
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf6
    .byte 0x8b; .byte 0x46; .byte 0x0b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xf2
    .byte 0x8b; .byte 0x46; .byte 0x0f
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xee
    .byte 0x8b; .byte 0x46; .byte 0x13
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xea
    .byte 0x8b; .byte 0x46; .byte 0x17
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe6
    .byte 0x8b; .byte 0x46; .byte 0x1b
    .byte 0x0f; .byte 0xc8
    .byte 0x89; .byte 0x42; .byte 0xe2
    .byte 0x8a; .byte 0x46; .byte 0x1f
    .byte 0x88; .byte 0x42; .byte 0xe1
    .byte 0x8a; .byte 0x46; .byte 0x20
    .byte 0x88; .byte 0x42; .byte 0xe0
    .byte 0x8a; .byte 0x46; .byte 0x21
    .byte 0x88; .byte 0x42; .byte 0xdf
    .byte 0x03; .byte 0xf1
    .byte 0x2b; .byte 0xd1
    .byte 0xe9; .long .L00105148-.-4
.L00106bf2:
    sub ecx, 0
    shr ecx, 2
    je .L00106c5c
.L00106bfa:
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106c5c
    jmp .L00106bfa
.L00106c5c:
    jmp .L00105148
.L00106c61:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00106cd5
.L00106c6d:
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106cd5
    jmp .L00106c6d
.L00106cd5:
    add esi, 1
    sub edx, 1
    jmp .L00105148
.L00106ce0:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00106d5a
.L00106cf2:
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106d5a
    jmp .L00106cf2
.L00106d5a:
    add esi, 2
    sub edx, 2
    jmp .L00105148
.L00106d65:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00106de5
.L00106d7d:
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106de5
    jmp .L00106d7d
.L00106de5:
    add esi, 3
    sub edx, 3
    jmp .L00105148
.L00106df0:
    sub ecx, 0
    shr ecx, 2
    je .L00106e5a
.L00106df8:
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00106e5a
    jmp .L00106df8
.L00106e5a:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    add esi, 1
    sub edx, 1
    jmp .L00105148
.L00106e69:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00106edd
.L00106e75:
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00106edd
    jmp .L00106e75
.L00106edd:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    add esi, 2
    sub edx, 2
    jmp .L00105148
.L00106eee:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    sub ecx, 2
    shr ecx, 2
    je .L00106f68
.L00106f00:
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L00106f68
    jmp .L00106f00
.L00106f68:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    add esi, 3
    sub edx, 3
    jmp .L00105148
.L00106f79:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00106ff9
.L00106f91:
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00106ff9
    jmp .L00106f91
.L00106ff9:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    add esi, 4
    sub edx, 4
    jmp .L00105148
.L0010700a:
    sub ecx, 0
    shr ecx, 2
    je .L00107074
.L00107012:
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L00107074
    jmp .L00107012
.L00107074:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    add esi, 2
    sub edx, 2
    jmp .L00105148
.L00107089:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L001070fd
.L00107095:
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L001070fd
    jmp .L00107095
.L001070fd:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    add esi, 3
    sub edx, 3
    jmp .L00105148
.L00107114:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    sub ecx, 2
    shr ecx, 2
    je .L0010718e
.L00107126:
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L0010718e
    jmp .L00107126
.L0010718e:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    add esi, 4
    sub edx, 4
    jmp .L00105148
.L001071a5:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00107225
.L001071bd:
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107225
    jmp .L001071bd
.L00107225:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    add esi, 5
    sub edx, 5
    jmp .L00105148
.L0010723c:
    sub ecx, 0
    shr ecx, 2
    je .L001072a6
.L00107244:
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    mov eax, dword ptr [esi]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 3], eax
    sub edx, 4
    dec ecx
    je .L001072a6
    jmp .L00107244
.L001072a6:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    add esi, 3
    sub edx, 3
    jmp .L00105148
.L001072c1:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    sub ecx, 1
    shr ecx, 2
    je .L00107335
.L001072cd:
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    mov eax, dword ptr [esi + 1]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 4], eax
    sub edx, 4
    dec ecx
    je .L00107335
    jmp .L001072cd
.L00107335:
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    add esi, 4
    sub edx, 4
    jmp .L00105148
.L00107352:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    sub ecx, 2
    shr ecx, 2
    je .L001073cc
.L00107364:
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    mov eax, dword ptr [esi + 2]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 5], eax
    sub edx, 4
    dec ecx
    je .L001073cc
    jmp .L00107364
.L001073cc:
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    add esi, 5
    sub edx, 5
    jmp .L00105148
.L001073e9:
    mov al, byte ptr [esi]
    mov byte ptr [edx], al
    mov al, byte ptr [esi + 1]
    mov byte ptr [edx - 1], al
    mov al, byte ptr [esi + 2]
    mov byte ptr [edx - 2], al
    sub ecx, 3
    shr ecx, 2
    je .L00107469
.L00107401:
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    mov eax, dword ptr [esi + 3]
    bswap eax
    add esi, 4
    mov dword ptr [edx - 6], eax
    sub edx, 4
    dec ecx
    je .L00107469
    jmp .L00107401
.L00107469:
    mov al, byte ptr [esi + 3]
    mov byte ptr [edx - 3], al
    mov al, byte ptr [esi + 4]
    mov byte ptr [edx - 4], al
    mov al, byte ptr [esi + 5]
    mov byte ptr [edx - 5], al
    add esi, 6
    sub edx, 6
    jmp .L00105148
    .byte 0xe9; .long .L00105148-.-4
    .byte 0x90
.L0010748c:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L00105148
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L001074a0:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L001074b4
    .byte 0x8b; .byte 0xc0
.L001074b0:
    neg cl
    {load} sub edx, ecx
.L001074b4:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L001074b0
    je .L001074d0
    {load} xor eax, eax
    nop
.L001074c0:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L001074c0
    jmp .L001074b4
.L001074d0:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L001074b4
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L001074e0:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor ecx, ecx
    {load} mov edx, edi
    jmp .L001074f4
    .byte 0x8b; .byte 0xc0
.L001074f0:
    neg cl
    {load} sub edx, ecx
.L001074f4:
    mov cl, byte ptr [esi]
    inc esi
    {load} or cl, cl
    js .L001074f0
    je .L00107510
    {load} xor eax, eax
    nop
.L00107500:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L00107500
    jmp .L001074f4
.L00107510:
    add edi, dword ptr [esp + 8]
    {load} mov edx, edi
    dec ebx
    jne .L001074f4
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00107520:
    {load} cmp eax, ecx
    jl .L00107530
    {load} sub eax, ecx
    inc eax
    mov dword ptr [esp + 8], eax
    {load} mov eax, ecx
    dec eax
    {load} mov eax, eax
.L00107530:
    {load} cmp eax, edx
    jge .L0010753c
    inc eax
    mov dword ptr [esp + 4], eax
    dec eax
    {load} mov eax, eax
.L0010753c:
    pop edx
    add eax, dword ptr [_D_00188c3c]
    add edx, dword ptr [_D_00188c40]
    mov ecx, dword ptr ["W?research_names$npnpna"]
    imul edx, ecx
    {load} add edx, eax
    add edx, dword ptr ["_lbDisplay"+0x4]
    {load} mov edi, edx
    {load} xor eax, eax
    {load} mov ebx, ebp
    {load} xor ebx, ebx
    mov bx, word ptr [_D_00188c74]
    test ebx, 4
    jne .L001076b4
    test ebx, 8
    jne .L00107790
    {load} mov ebx, ebp
.L00107583:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L001075d8
.L00107590:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001076a0
    js .L001075bc
    inc esi
    {load} cmp ecx, ebp
    jle .L001075b4
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    je .L00107680
    jmp .L001075ec
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001075b4:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L00107590
    .byte 0x8b; .byte 0xc0
.L001075bc:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L001075d4
    {load} sub ecx, ebp
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L00107680
    jmp .L001075d8
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001075d4:
    {load} sub ebp, ecx
    jmp .L00107590
.L001075d8:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001076a0
    js .L00107690
    inc esi
    nop
.L001075ec:
    {load} cmp ecx, edi
    jge .L00107634
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L001075f8:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L00107630
    jmp .L001075f8
.L00107630:
    jmp .L001075d8
    .byte 0x8b; .byte 0xc0
.L00107634:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00107644:
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    mov al, byte ptr [esi]
    inc esi
    mov byte ptr [edx], al
    dec edx
    dec ecx
    je .L0010767c
    jmp .L00107644
.L0010767c:
    pop ebp
    {load} add esi, ebp
    nop
.L00107680:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L001076a0
    js .L0010768c
    {load} add esi, ecx
.L0010768c:
    inc esi
    jmp .L00107680
    .byte 0x90
.L00107690:
    inc esi
    neg cl
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L00107680
    jmp .L001075d8
    .byte 0x8b; .byte 0xc0
.L001076a0:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L00107583
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L001076b4:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L001076bc:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L00107710
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L001076cc:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L0010777c
    js .L001076f8
    inc esi
    {load} cmp ecx, ebp
    jle .L001076f0
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    .byte 0x0f; .byte 0x84; .long .L00107760-.-4  # je 0x107760
    jmp .L0010771c
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001076f0:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L001076cc
    .byte 0x8b; .byte 0xc0
.L001076f8:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L0010770c
    {load} sub ecx, ebp
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L00107760
    jmp .L00107710
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L0010770c:
    {load} sub ebp, ecx
    jmp .L001076cc
.L00107710:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L0010777c
    js .L00107770
    inc esi
    nop
.L0010771c:
    {load} cmp ecx, edi
    jge .L0010773c
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    {load} mov eax, eax
.L0010772c:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L0010772c
    jmp .L00107710
.L0010773c:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    nop
.L0010774c:
    mov al, byte ptr [edx]
    mov ah, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L0010774c
    pop ebp
    {load} add esi, ebp
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L00107760:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L0010777c
    js .L0010776c
    {load} add esi, ecx
.L0010776c:
    inc esi
    jmp .L00107760
    .byte 0x90
.L00107770:
    inc esi
    neg cl
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L00107760
    jmp .L00107710
    .byte 0x90
.L0010777c:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L001076bc
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00107790:
    {load} mov ebx, ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
.L00107798:
    push edx
    mov edi, dword ptr [esp + 4]
    mov ebp, dword ptr [esp + 8]
    {load} or ebp, ebp
    je .L001077ec
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L001077a8:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00107858
    js .L001077d4
    inc esi
    {load} cmp ecx, ebp
    jle .L001077cc
    {load} add esi, ebp
    {load} sub ecx, ebp
    {load} or edi, edi
    .byte 0x0f; .byte 0x84; .long .L0010783c-.-4  # je 0x10783c
    jmp .L001077f8
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001077cc:
    {load} add esi, ecx
    {load} sub ebp, ecx
    jmp .L001077a8
    .byte 0x8b; .byte 0xc0
.L001077d4:
    inc esi
    neg cl
    {load} cmp ecx, ebp
    jl .L001077e8
    {load} sub ecx, ebp
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L0010783c
    jmp .L001077ec
    .byte 0x2e; .byte 0x8b; .byte 0xc0
.L001077e8:
    {load} sub ebp, ecx
    jmp .L001077a8
.L001077ec:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00107858
    js .L0010784c
    inc esi
    nop
.L001077f8:
    {load} cmp ecx, edi
    jge .L00107818
    {load} sub edi, ecx
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    {load} mov eax, eax
.L00107808:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L00107808
    jmp .L001077ec
.L00107818:
    {load} mov ebp, ecx
    {load} sub ebp, edi
    {load} mov ecx, edi
    push ebp
    mov ebp, dword ptr ["_lbDisplay"+0x8]
    {load} xor eax, eax
    nop
.L00107828:
    mov ah, byte ptr [edx]
    mov al, byte ptr [esi]
    inc esi
    mov al, byte ptr [eax + ebp]
    mov byte ptr [edx], al
    dec edx
    dec ecx
    jne .L00107828
    pop ebp
    {load} add esi, ebp
    .byte 0x2e; .byte 0x8b; .byte 0xc0  # mov eax, eax
.L0010783c:
    {load} xor ecx, ecx
    mov cl, byte ptr [esi]
    {load} or cl, cl
    je .L00107858
    js .L00107848
    {load} add esi, ecx
.L00107848:
    inc esi
    jmp .L0010783c
    .byte 0x90
.L0010784c:
    inc esi
    neg cl
    {load} sub edx, ecx
    {load} sub edi, ecx
    jle .L0010783c
    jmp .L001077ec
    .byte 0x90
.L00107858:
    pop edx
    inc esi
    add edx, dword ptr [esp + 8]
    dec ebx
    jne .L00107798
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L0010786c:
    add esp, 0x10
    .byte 0x61  # popal
    ret
.L00107871:
    add esp, 0xc
    .byte 0x61  # popal
    ret
    .byte 0x8b; .byte 0xc0
.L00107878:
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .long .L00102e34
    .long .L00102e34
    .long .L00102e34
    .long .L00102e34
    .long .L00102e3e
    .long .L00102e3e
    .long .L00102e3e
    .long .L00102e3e
    .long .L00102e4e
    .long .L00102e4e
    .long .L00102e4e
    .long .L00102e4e
    .long .L00102fae
    .long .L00102e97
    .long .L00102ee2
    .long .L00102f3f
    .long .L00103006
    .long .L00102f01
    .long .L00102f5e
    .long .L00102fbb
    .long .L00103076
    .long .L00102f83
    .long .L00102fce
    .long .L00103019
    .long .L001030fe
    .long .L00102fe7
    .long .L00103032
    .long .L0010308f
    .long .L0010319e
    .long .L00103051
    .long .L001030ae
    .long .L0010311d
    .long .L0010320e
    .long .L001030d3
    .long .L00103142
    .long .L001031b1
    .long .L00103296
    .long .L0010316d
    .long .L001031ca
    .long .L00103227
    .long .L00103336
    .long .L001031e9
    .long .L00103246
    .long .L001032b5
    .long .L001033ee
    .long .L0010326b
    .long .L001032da
    .long .L0010335b
    .long .L00103476
    .long .L00103305
    .long .L00103386
    .long .L00103407
    .long .L00103516
    .long .L001033b7
    .long .L00103426
    .long .L00103495
    .long .L001035ce
    .long .L0010344b
    .long .L001034ba
    .long .L0010353b
    .long .L0010369e
    .long .L001034e5
    .long .L00103566
    .long .L001035f9
    .long .L0010373e
    .long .L00103597
    .long .L0010362a
    .long .L001036bd
    .long .L001037f6
    .long .L00103661
    .long .L001036e2
    .long .L00103763
    .long .L001038c6
    .long .L0010370d
    .long .L0010378e
    .long .L00103821
    .long .L001039ae
    .long .L001037bf
    .long .L00103852
    .long .L001038f7
    .long .L00103a66
    .long .L00103889
    .long .L0010392e
    .long .L001039d3
    .long .L00103b36
    .long .L0010396b
    .long .L001039fe
    .long .L00103a91
    .long .L00103c1e
    .long .L00103a2f
    .long .L00103ac2
    .long .L00103b67
    .long .L00103d1e
    .long .L00103af9
    .long .L00103b9e
    .long .L00103c55
    .long .L00103dee
    .long .L00103bdb
    .long .L00103c92
    .long .L00103d49
    .long .L00103ed6
    .long .L00103cd5
    .long .L00103d7a
    .long .L00103e1f
    .long .L00103fd6
    .long .L00103db1
    .long .L00103e56
    .long .L00103f0d
    .long .L001040ee
    .long .L00103e93
    .long .L00103f4a
    .long .L00104013
    .long .L001041d6
    .long .L00103f8d
    .long .L00104056
    .long .L0010411f
    .long .L001042d6
    .long .L0010409f
    .long .L00104156
    .long .L0010420d
    .long .L001043ee
    .long .L00104193
    .long .L0010424a
    .long .L00104313
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
    .long .L0010451e
    .long .L00104845
    .long .L001049b6
    .long .L00104b39
    .long .L001046e6
    .long .L00104a3b
    .long .L00104bbe
    .long .L0010457b
    .long .L001048ca
    .long .L00104c49
    .long .L001045ee
    .long .L0010474d
    .long .L00104ac6
    .long .L00104667
    .long .L001047c6
    .long .L00104937
.L00108278:
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .byte 0x00; .byte 0x00
    .long .L0010516c
    .long .L0010516c
    .long .L0010516c
    .long .L0010516c
    .long .L00105176
    .long .L00105176
    .long .L00105176
    .long .L00105176
    .long .L00105186
    .long .L00105186
    .long .L00105186
    .long .L00105186
    .long .L00105277
    .long .L0010521a
    .long .L001051cf
    .long .L001052e6
    .long .L001052f6
    .long .L00105296
    .long .L00105239
    .long .L00105347
    .long .L0010535d
    .long .L0010530b
    .long .L001052bb
    .long .L001053c0
    .long .L001053dc
    .long .L00105378
    .long .L00105326
    .long .L00105451
    .long .L00105473
    .long .L001053fd
    .long .L00105399
    .long .L001054fa
    .long .L00105512
    .long .L0010549a
    .long .L00105424
    .long .L0010557b
    .long .L00105599
    .long .L0010552f
    .long .L001054c7
    .long .L00105614
    .long .L00105638
    .long .L001055bc
    .long .L00105552
    .long .L001056c5
    .long .L001056ef
    .long .L00105661
    .long .L001055e5
    .long .L0010578e
    .long .L001057ae
    .long .L0010571e
    .long .L00105690
    .long .L0010582f
    .long .L00105855
    .long .L001057d3
    .long .L00105753
    .long .L001058e8
    .long .L00105914
    .long .L00105880
    .long .L001057fe
    .long .L001059b9
    .long .L001059eb
    .long .L00105945
    .long .L001058b1
    .long .L00105aa2
    .long .L00105aca
    .long .L00105a22
    .long .L0010597c
    .long .L00105b63
    .long .L00105b91
    .long .L00105af7
    .long .L00105a5f
    .long .L00105c3c
    .long .L00105c70
    .long .L00105bc4
    .long .L00105b2a
    .long .L00105d2d
    .long .L00105d67
    .long .L00105ca9
    .long .L00105bfd
    .long .L00105e36
    .long .L00105e66
    .long .L00105da6
    .long .L00105ce8
    .long .L00105f17
    .long .L00105f4d
    .long .L00105e9b
    .long .L00105deb
    .long .L00106010
    .long .L0010604c
    .long .L00105f88
    .long .L00105ed6
    .long .L00106121
    .long .L00106163
    .long .L0010608d
    .long .L00105fc9
    .long .L0010624a
    .long .L00106282
    .long .L001061aa
    .long .L001060d4
    .long .L0010634b
    .long .L00106389
    .long .L001062bf
    .long .L001061f7
    .long .L00106464
    .long .L001064a8
    .long .L001063cc
    .long .L00106302
    .long .L00106595
    .long .L001065df
    .long .L001064f1
    .long .L00106415
    .long .L001066de
    .long .L0010671e
    .long .L0010662e
    .long .L00106540
    .long .L001067ff
    .long .L00106845
    .long .L00106763
    .long .L00106683
    .long .L00106938
    .long .L00106984
    .long .L00106890
    .long .L001067ae
    .long .L00106a89
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
    .long .L001072c1
    .long .L00107114
    .long .L00106f79
    .long .L00106bf2
    .long .L00106c61
    .long .L00107352
    .long .L001071a5
    .long .L00106df0
    .long .L00106e69
    .long .L00106ce0
    .long .L001073e9
    .long .L0010700a
    .long .L00107089
    .long .L00106eee
    .long .L00106d65
    .long .L0010723c
.size "LbSpriteDraw_",.-"LbSpriteDraw_"
