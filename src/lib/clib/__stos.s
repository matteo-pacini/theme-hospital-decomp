.intel_syntax noprefix

// __STOSB: store ecx bytes of the repeated pattern in edx at eax (aligning, then by dwords); eax ends past the area.
// FUNCTION: 0x000cf520

.section _TEXT,"ax",@progbits,unique,849184
.globl "__STOSB"
.type "__STOSB",@function
"__STOSB":
    or ecx, ecx
    je .L000cf550
    cmp byte ptr [eax], dl
.L000cf526:
    test al, 3
    je .L000cf533
    mov byte ptr [eax], dl
    inc eax
    ror edx, 8
    dec ecx
    jne .L000cf526
.L000cf533:
    push ecx
    shr ecx, 2
    call "__STOSD"
    pop ecx
    and ecx, 3
    je .L000cf550
    mov byte ptr [eax], dl
    dec ecx
    je .L000cf550
    mov byte ptr [eax + 1], dh
    dec ecx
    je .L000cf550
    mov byte ptr [eax + 2], dl
.L000cf550:
    ret
.size "__STOSB",.-"__STOSB"
    .byte 0x90, 0x90, 0x90, 0x90, 0x90, 0x90

// __STOSD: store ecx copies of dword edx at eax (unrolled by 32 bytes); eax ends past the area.
// FUNCTION: 0x000cf557

.globl "__STOSD"
.type "__STOSD",@function
"__STOSD":
    or ecx, ecx
    je .L000cf5c2
.L000cf55b:
    test al, 0x1f
    je .L000cf567
    mov dword ptr [eax], edx
    lea eax, [eax + 4]
    dec ecx
    jne .L000cf55b
.L000cf567:
    push ecx
    shr ecx, 2
    je .L000cf5a7
    dec ecx
    je .L000cf599
.L000cf570:
    mov dword ptr [eax], edx
    mov dword ptr [eax + 4], edx
    dec ecx
    mov dword ptr [eax + 8], edx
    mov dword ptr [eax + 0xc], edx
    je .L000cf596
    cmp byte ptr [eax + 0x20], dl
    mov dword ptr [eax + 0x10], edx
    mov dword ptr [eax + 0x14], edx
    dec ecx
    mov dword ptr [eax + 0x18], edx
    mov dword ptr [eax + 0x1c], edx
    lea eax, [eax + 0x20]
    jne .L000cf570
    lea eax, [eax - 0x10]
.L000cf596:
    lea eax, [eax + 0x10]
.L000cf599:
    mov dword ptr [eax], edx
    mov dword ptr [eax + 4], edx
    mov dword ptr [eax + 8], edx
    mov dword ptr [eax + 0xc], edx
    lea eax, [eax + 0x10]
.L000cf5a7:
    pop ecx
    and ecx, 3
    je .L000cf5c2
    mov dword ptr [eax], edx
    lea eax, [eax + 4]
    dec ecx
    je .L000cf5c2
    mov dword ptr [eax], edx
    lea eax, [eax + 4]
    dec ecx
    je .L000cf5c2
    mov dword ptr [eax], edx
    lea eax, [eax + 4]
.L000cf5c2:
    ret
.size "__STOSD",.-"__STOSD"
