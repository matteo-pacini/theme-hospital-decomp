.intel_syntax noprefix

// outp: write byte value to I/O port port.
// FUNCTION: 0x000cd1c7

.section _TEXT,"ax",@progbits,unique,840135
.globl "outp_"
.type "outp_",@function
"outp_":
    push ebx
    mov ebx, eax
    mov al, dl
    mov edx, ebx
    out dx, al
    pop ebx
    ret
.size "outp_",.-"outp_"
