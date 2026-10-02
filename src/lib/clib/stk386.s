.intel_syntax noprefix

// stk_save_ss: startup routine; record the program's stack segment for __STK.
// FUNCTION: 0x000ed282

.section _TEXT,"ax",@progbits,unique,971394
.globl "stk_save_ss"
.type "stk_save_ss",@function
"stk_save_ss":
    .byte 0x66; .byte 0x8c; .byte 0x15; .long .Lstk_ss  # mov word ptr [0x31a60], ss
    ret
.size "stk_save_ss",.-"stk_save_ss"

// __CHK: stack check for the size passed on the stack (__STK).
// FUNCTION: 0x000ed28a

.section _TEXT,"ax",@progbits,unique,971402
.globl "__CHK"
.type "__CHK",@function
"__CHK":
    xchg dword ptr [esp + 4], eax
    call "__STK"
    mov eax, dword ptr [esp + 4]
    ret 4
.size "__CHK",.-"__CHK"

// __GRO: stack-growth hook; does nothing.
// FUNCTION: 0x000ed29a

.section _TEXT,"ax",@progbits,unique,971418
.globl "__GRO"
.type "__GRO",@function
"__GRO":
    ret 4
.size "__GRO",.-"__GRO"

// __STK: check that eax bytes of stack remain above _STACKLOW; on the program's own stack, a shortfall is a fatal stack overflow.
// FUNCTION: 0x000ed29d

.section _TEXT,"ax",@progbits,unique,971421
.globl "__STK"
.type "__STK",@function
"__STK":
    cmp eax, esp
    jae .L000ed2ae
    sub eax, esp
    neg eax
    cmp eax, dword ptr ["__STACKLOW"]
    jbe .L000ed2ae
    ret
.L000ed2ae:
    mov ax, ss
    cmp ax, word ptr [.Lstk_ss]
    .byte 0x74; .byte 0x01  # je 0xed2bb
    ret
.size "__STK",.-"__STK"

// __STKOVERFLOW: fatal "Stack Overflow!" error.
// FUNCTION: 0x000ed2bb

.section _TEXT,"ax",@progbits,unique,971451
.globl "__STKOVERFLOW_"
.type "__STKOVERFLOW_",@function
"__STKOVERFLOW_":
    .byte 0xb8; .long .Lstk_msg  # mov eax, 0x31a62
    mov edx, 1
    call "__fatal_runtime_error_"
.size "__STKOVERFLOW_",.-"__STKOVERFLOW_"

.section _DATA,"aw",@progbits
.Lstk_ss:
    .word 0
.Lstk_msg:
    .asciz "Stack Overflow!\r\n"

.section XI,"aw",@progbits
    .byte 0x00, 0x20
    .long "stk_save_ss"
