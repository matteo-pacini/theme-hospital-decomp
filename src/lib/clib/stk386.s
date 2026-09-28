.intel_syntax noprefix

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

// FUNCTION: 0x000ed29a

.section _TEXT,"ax",@progbits,unique,971418
.globl "__GRO"
.type "__GRO",@function
"__GRO":
    ret 4
.size "__GRO",.-"__GRO"

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
    cmp ax, word ptr ["___umaskval"+0x4]
    .byte 0x74; .byte 0x01  # je 0xed2bb
    ret
.size "__STK",.-"__STK"

// FUNCTION: 0x000ed2bb

.section _TEXT,"ax",@progbits,unique,971451
.globl "__STKOVERFLOW_"
.type "__STKOVERFLOW_",@function
"__STKOVERFLOW_":
    .byte 0xb8; .long "___umaskval"+0x6  # mov eax, 0x31a62
    mov edx, 1
    call "__fatal_runtime_error_"
.size "__STKOVERFLOW_",.-"__STKOVERFLOW_"
