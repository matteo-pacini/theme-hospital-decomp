.intel_syntax noprefix

// __FDFS: convert the double in edx:eax to a float in eax (rounded half up in magnitude); +0 for zero, denormals and underflow, signed infinity on overflow and for infinities and NaNs.
// FUNCTION: 0x000fc6d6

.section _TEXT,"ax",@progbits,unique,1033942
.globl "__FDFS"
.type "__FDFS",@function
"__FDFS":
    push ebx
    test edx, 0x7ff00000
    je .L000fc71e
    sub ebx, ebx
    add eax, eax
    adc edx, edx
    rcr ebx, 1
    add eax, 0x20000000
    adc edx, 0
    je .L000fc715
    cmp edx, 0x8fe00000
    jae .L000fc715
    cmp edx, 0x70200000
    jb .L000fc71e
    sub edx, 0x70000000
    add eax, eax
    adc edx, edx
    add eax, eax
    adc edx, edx
    or edx, ebx
    mov eax, edx
    pop ebx
    ret
.L000fc715:
    mov eax, 0x7f800000
    or eax, ebx
    pop ebx
    ret
.L000fc71e:
    sub eax, eax
    pop ebx
    ret
.size "__FDFS",.-"__FDFS"
