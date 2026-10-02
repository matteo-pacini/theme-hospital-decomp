.intel_syntax noprefix

// IF@LOG: natural log of st(0) (also the shared body of IF@LOG2/IF@LOG10, function code in al); x <= 0 goes to __log87_err.
// FUNCTION: 0x000fc9ce

.section _TEXT,"ax",@progbits,unique,1034702
.globl "IF@LOG"
.type "IF@LOG",@function
"IF@LOG":
    mov al, 0xa
    ftst
    push ebp
    mov ebp, esp
    sub esp, 0x10
    wait
    fnstsw word ptr [ebp - 0x10]
    wait
    mov ah, byte ptr [ebp - 0xf]
    sahf
    ja .L000fc9f5
    mov dword ptr [ebp - 8], eax
    fstp qword ptr [ebp - 0x10]
    mov dword ptr [ebp - 4], edx
    call "__log87_err_"
    mov al, 1
    jmp .L000fca0d
.L000fc9f5:
    cmp al, 0xb
    jne .L000fc9fd
    fldlg2
    jmp .L000fca07
.L000fc9fd:
    cmp al, 9
    jne .L000fca05
    fld1
    jmp .L000fca07
.L000fca05:
    fldln2
.L000fca07:
    fxch st(1)
    fyl2x
    mov al, 0
.L000fca0d:
    mov esp, ebp
    pop ebp
    ret
.size "IF@LOG",.-"IF@LOG"

// IF@LOG2: base-2 log of st(0).
// FUNCTION: 0x000fca11

.section _TEXT,"ax",@progbits,unique,1034769
.globl "IF@LOG2"
.type "IF@LOG2",@function
"IF@LOG2":
    mov al, 9
    .byte 0xeb; .byte 0xbb  # jmp 0xfc9d0
.size "IF@LOG2",.-"IF@LOG2"

// IF@LOG10: base-10 log of st(0).
// FUNCTION: 0x000fca15

.section _TEXT,"ax",@progbits,unique,1034773
.globl "IF@LOG10"
.type "IF@LOG10",@function
"IF@LOG10":
    mov al, 0xb
    .byte 0xeb; .byte 0xb7  # jmp 0xfc9d0
.size "IF@LOG10",.-"IF@LOG10"

// log: natural logarithm of x.
// FUNCTION: 0x000fca19

.section _TEXT,"ax",@progbits,unique,1034777
.globl "log_"
.type "log_",@function
"log_":
    fld qword ptr [esp + 4]
    call "IF@LOG"
    ret 8
.size "log_",.-"log_"

// log10: base-10 logarithm of x.
// FUNCTION: 0x000fca25

.section _TEXT,"ax",@progbits,unique,1034789
.globl "log10_"
.type "log10_",@function
"log10_":
    fld qword ptr [esp + 4]
    call "IF@LOG10"
    ret 8
.size "log10_",.-"log10_"

// log2: base-2 logarithm of x.
// FUNCTION: 0x000fca31

.section _TEXT,"ax",@progbits,unique,1034801
.globl "log2_"
.type "log2_",@function
"log2_":
    fld qword ptr [esp + 4]
    call "IF@LOG2"
    ret 8
.size "log2_",.-"log2_"
