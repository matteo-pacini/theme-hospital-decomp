.intel_syntax noprefix

// FUNCTION: 0x000cd1c7

.section _TEXT,"ax",@progbits,unique,840135
.globl "fn_000cd1c7_"
.type "fn_000cd1c7_",@function
"fn_000cd1c7_":
    push ebx
    mov ebx, eax
    mov al, dl
    mov edx, ebx
    out dx, al
    pop ebx
    ret
.size "fn_000cd1c7_",.-"fn_000cd1c7_"
