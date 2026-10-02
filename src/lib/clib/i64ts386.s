.intel_syntax noprefix

// __CmpBigInt: number of decimal digits of the 64-bit integer at edx minus eax (0 if it has exactly eax digits).
// FUNCTION: 0x000fcf86

.section _TEXT,"ax",@progbits,unique,1036166
.globl "__CmpBigInt"
.type "__CmpBigInt",@function
"__CmpBigInt":
    push edi
    push ecx
    call "pow10_table_addr"
    inc eax
    lea edi, [edi + eax*8]
    mov ecx, dword ptr [edx]
    mov edx, dword ptr [edx + 4]
    sub eax, eax
.L000fcf98:
    cmp edx, dword ptr cs:[edi]
    jne .L000fcfa1
    cmp ecx, dword ptr cs:[edi + 4]
.L000fcfa1:
    jb .L000fcfa9
    add edi, 8
    inc eax
    jmp .L000fcf98
.L000fcfa9:
    sub edi, 8
.L000fcfac:
    cmp edx, dword ptr cs:[edi]
    jne .L000fcfb5
    cmp ecx, dword ptr cs:[edi + 4]
.L000fcfb5:
    jae .L000fcfbd
    sub edi, 8
    dec eax
    jmp .L000fcfac
.L000fcfbd:
    pop ecx
    pop edi
    ret
.size "__CmpBigInt",.-"__CmpBigInt"

// __Rnd2Int: round the non-negative double at eax to a 64-bit integer (half to even) stored at edx.
// FUNCTION: 0x000fcfc0

.section _TEXT,"ax",@progbits,unique,1036224
.globl "__Rnd2Int"
.type "__Rnd2Int",@function
"__Rnd2Int":
    push ebx
    push ecx
    push edx
    push ebp
    push esi
    push edx
    mov ebp, eax
    mov ecx, dword ptr [ebp]
    mov ebx, dword ptr [ebp + 4]
    mov ebp, ebx
    and ebp, 0xfff00000
    xor ebx, ebp
    xor ebx, 0x100000
    shr ebp, 0x14
    sub ebp, 0x433
    je .L000fd01e
    jbe .L000fcff4
.L000fcfeb:
    shl ecx, 1
    rcl ebx, 1
    dec ebp
    jne .L000fcfeb
    jmp .L000fd01e
.L000fcff4:
    sub eax, eax
    sub esi, esi
.L000fcff8:
    shr ebx, 1
    rcr ecx, 1
    rcr eax, 1
    adc esi, esi
    inc ebp
    jne .L000fcff8
    cmp eax, 0x80000000
    jb .L000fd01e
    jne .L000fd018
    or esi, esi
    jne .L000fd018
    test ecx, 1
    je .L000fd01e
.L000fd018:
    add ecx, 1
    adc ebx, 0
.L000fd01e:
    pop ebp
    mov dword ptr [ebp], ecx
    mov dword ptr [ebp + 4], ebx
    pop esi
    pop ebp
    pop edx
    pop ecx
    pop ebx
    ret
.size "__Rnd2Int",.-"__Rnd2Int"

// __Bin2String: write the low ebx decimal digits of the 64-bit integer at eax (zero-padded) to the buffer at edx, NUL-terminated.
// FUNCTION: 0x000fd02b

.section _TEXT,"ax",@progbits,unique,1036331
.globl "__Bin2String"
.type "__Bin2String",@function
"__Bin2String":
    push ebp
    push edi
    push ecx
    push ebx
    mov ebp, eax
    mov ecx, dword ptr [ebp]
    mov eax, dword ptr [ebp + 4]
    mov ebp, edx
    add ebp, ebx
    mov byte ptr [ebp], 0
    push eax
.L000fd040:
    pop eax
    mov edi, 0x2710
    sub edx, edx
    or eax, eax
    jne .L000fd053
    or ecx, ecx
    jne .L000fd055
    push eax
    jmp .L000fd06b
.L000fd053:
    div edi
.L000fd055:
    xchg ecx, eax
    div edi
    xchg ecx, eax
    push eax
    mov eax, edx
    mov dl, 0x64
    div dl
    mov dl, ah
    mov ah, 0
    aam 0xa
    xchg edx, eax
    mov ah, 0
    aam 0xa
.L000fd06b:
    add ax, 0x3030
    add dx, 0x3030
    sub ebp, 4
    mov byte ptr [ebp + 3], al
    dec ebx
    je .L000fd08f
    mov byte ptr [ebp + 2], ah
    dec ebx
    je .L000fd08f
    mov byte ptr [ebp + 1], dl
    dec ebx
    je .L000fd08f
    mov byte ptr [ebp], dh
    dec ebx
    jne .L000fd040
.L000fd08f:
    pop eax
    pop ebx
    pop ecx
    pop edi
    pop ebp
    ret
.size "__Bin2String",.-"__Bin2String"


// pow10_table_addr: return in edi the address of the 64-bit powers-of-ten table that follows the call.
// FUNCTION: 0x000fd095

.section _TEXT,"ax",@progbits,unique,1036437
.globl "pow10_table_addr"
.type "pow10_table_addr",@function
"pow10_table_addr":
    call "get_return_addr"
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0xa0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x80, 0x96, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0xf5, 0x05
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0xca, 0x9a, 0x3b, 0x02, 0x00, 0x00, 0x00, 0x00, 0xe4, 0x0b, 0x54
    .byte 0x17, 0x00, 0x00, 0x00, 0x00
    .byte 0xe8, 0x76, 0x48, 0xe8, 0x00
    .byte 0x00, 0x00, 0x00, 0x10, 0xa5, 0xd4, 0x18, 0x09, 0x00, 0x00, 0x00, 0xa0, 0x72, 0x4e, 0xf3, 0x5a
    .byte 0x00, 0x00, 0x00, 0x40, 0x7a, 0x10, 0x7e, 0x8d, 0x03, 0x00, 0x00, 0x80, 0xc6, 0xa4, 0xf2, 0x86
    .byte 0x23, 0x00, 0x00, 0x00, 0xc1, 0x6f, 0x78, 0x45, 0x63, 0x01, 0x00, 0x00, 0x8a, 0x5d, 0xb3, 0xb6
    .byte 0xe0, 0x0d, 0x00, 0x00, 0x64, 0xa7, 0x04, 0x23, 0xc7, 0x8a, 0x00, 0x00, 0xe8, 0x89, 0xff, 0xff
    .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
.size "pow10_table_addr",.-"pow10_table_addr"

// get_return_addr: return the caller's return address in edi.
// FUNCTION: 0x000fd14a

.section _TEXT,"ax",@progbits,unique,1036618
.globl "get_return_addr"
.type "get_return_addr",@function
"get_return_addr":
    pop edi
    ret
.size "get_return_addr",.-"get_return_addr"
