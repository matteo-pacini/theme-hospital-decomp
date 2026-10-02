.intel_syntax noprefix

// inp: read a byte from I/O port port.
// FUNCTION: 0x000cd1bf

.section _TEXT,"ax",@progbits,unique,840127
.globl "inp_"
.type "inp_",@function
"inp_":
    push edx
    mov edx, eax
    sub eax, eax
    in al, dx
    pop edx
    ret
.size "inp_",.-"inp_"
