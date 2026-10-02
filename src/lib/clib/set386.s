.intel_syntax noprefix

// memset: fill n bytes at s with (unsigned char)c; returns s.
// FUNCTION: 0x000bc0b0

.section _TEXT,"ax",@progbits,unique,770224
.globl "memset_"
.type "memset_",@function
"memset_":
    push eax
    push ecx
    mov dh, dl
    shl edx, 8
    mov dl, dh
    shl edx, 8
    mov dl, dh
    mov ecx, ebx
    call "__STOSB"
    pop ecx
    pop eax
    ret
.size "memset_",.-"memset_"
