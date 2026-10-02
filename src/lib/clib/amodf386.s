.intel_syntax noprefix

// __ModF: split the double at eax: integral part stored at edx, fractional part (with the sign) left at eax.
// FUNCTION: 0x000fcdd2

.section _TEXT,"ax",@progbits,unique,1035730
.globl "__ModF"
.type "__ModF",@function
"__ModF":
    push ebp
    push esi
    push edi
    push ecx
    push ebx
    mov esi, edx
    mov ebp, eax
    mov eax, dword ptr [ebp]
    mov edx, dword ptr [ebp + 4]
    xchg esi, ebp
    mov dword ptr [ebp], eax
    mov dword ptr [ebp + 4], edx
    mov ecx, edx
    and ecx, 0x7ff00000
    je .L000fcdfd
    cmp ecx, 0x43300000
    jb .L000fce0b
    xchg ebp, esi
.L000fcdfd:
    sub eax, eax
    mov dword ptr [ebp + 4], eax
    mov dword ptr [ebp], eax
    pop ebx
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret
.L000fce0b:
    mov edi, ecx
    shr ecx, 0x14
    sub cx, 0x3ff
    jb .L000fcdfd
    push edx
    sub eax, eax
    mov edx, 0xfff00000
    cmp cl, 0x14
    jbe .L000fce30
    sar edx, 0x15
    rcr eax, 1
    sub cl, 0x15
    sar eax, cl
    jmp .L000fce32
.L000fce30:
    sar edx, cl
.L000fce32:
    and dword ptr [ebp + 4], edx
    and dword ptr [ebp], eax
    not edx
    not eax
    mov ebp, esi
    and edx, dword ptr [ebp + 4]
    and eax, dword ptr [ebp]
    call "modf_pack_double"
    pop esi
    or edx, edx
    je .L000fce56
    and esi, 0x80000000
    or edx, esi
.L000fce56:
    mov dword ptr [ebp + 4], edx
    mov dword ptr [ebp], eax
    pop ebx
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret
.size "__ModF",.-"__ModF"

// __ZBuf2F: convert the decimal digit string at eax to the double at edx (exact up to 64 bits, then rounded).
// FUNCTION: 0x000fce62

.section _TEXT,"ax",@progbits,unique,1035874
.globl "__ZBuf2F"
.type "__ZBuf2F",@function
"__ZBuf2F":
    push ebp
    push esi
    push edi
    push ecx
    push ebx
    push edx
    mov esi, eax
    sub edx, edx
    sub ecx, ecx
    sub eax, eax
.L000fce70:
    mov al, byte ptr [esi]
    cmp al, 0
    je .L000fce94
    mov edi, edx
    mov ebx, ecx
    add ecx, ecx
    adc edx, edx
    add ecx, ecx
    adc edx, edx
    add ecx, ebx
    adc edx, edi
    add ecx, ecx
    adc edx, edx
    and al, 0xf
    add ecx, eax
    adc edx, 0
    inc esi
    jmp .L000fce70
.L000fce94:
    mov eax, ecx
    mov edi, 0x43300000
    call "modf_pack_double"
    pop ebp
    mov dword ptr [ebp + 4], edx
    mov dword ptr [ebp], eax
    pop ebx
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret
.size "__ZBuf2F",.-"__ZBuf2F"

// modf_pack_double: pack the 64-bit mantissa edx:eax with exponent field edi into a double in edx:eax, normalising and rounding; 0 stays 0.
// FUNCTION: 0x000fcead

.section _TEXT,"ax",@progbits,unique,1035949
.globl "modf_pack_double"
.type "modf_pack_double",@function
"modf_pack_double":
    sub esi, esi
    or esi, eax
    or esi, edx
    je .L000fcefd
    test edx, 0xfff00000
    jne .L000fced1
.L000fcebd:
    sub edi, 0x100000
    add eax, eax
    adc edx, edx
    test edx, 0xfff00000
    je .L000fcebd
    jmp .L000fcef5
.L000fced1:
    test edx, 0xffe00000
    je .L000fcef5
.L000fced9:
    add edi, 0x100000
    shr edx, 1
    rcr eax, 1
    rcr esi, 1
    test edx, 0xffe00000
    jne .L000fced9
    adc esi, esi
    adc eax, 0
    adc edx, 0
.L000fcef5:
    and edx, 0xfffff
    or edx, edi
.L000fcefd:
    ret
.size "modf_pack_double",.-"modf_pack_double"
