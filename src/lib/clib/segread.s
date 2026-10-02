.intel_syntax noprefix

// segread: store the current segment registers in *segregs.
// FUNCTION: 0x000d0b1b

.section _TEXT,"ax",@progbits,unique,854811
.globl "segread_"
.type "segread_",@function
"segread_":
    push edx
    mov edx, eax
    mov eax, cs
    mov word ptr [edx + 2], ax
    mov eax, ds
    mov word ptr [edx + 6], ax
    mov eax, es
    mov word ptr [edx], ax
    mov eax, ss
    mov word ptr [edx + 4], ax
    mov eax, fs
    mov word ptr [edx + 8], ax
    mov eax, gs
    mov word ptr [edx + 0xa], ax
    pop edx
    ret
.size "segread_",.-"segread_"
