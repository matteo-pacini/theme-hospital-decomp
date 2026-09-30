.intel_syntax noprefix

// The runtime's startup globals: a data-only module.
.section _DATA,"aw",@progbits
.globl "__dynend"
"__dynend":
    .long 0
.globl "__curbrk"
"__curbrk":
    .long 0
.globl "__LpCmdLine"
"__LpCmdLine":
    .long 0
.globl "__LpPgmName"
"__LpPgmName":
    .long 0
.globl "__psp"
"__psp":
    .long 0
.globl "__STACKLOW"
"__STACKLOW":
    .long 0
.globl "__STACKTOP"
"__STACKTOP":
    .long 0
.globl "__ASTACKSIZ"
"__ASTACKSIZ":
    .long 0
.globl "__ASTACKPTR"
"__ASTACKPTR":
    .long 0
.globl "__cbyte"
"__cbyte":
    .long 0
.globl "__cbyte2"
"__cbyte2":
    .long 0
.globl "__child"
"__child":
    .long 0
.globl "__no87"
"__no87":
    .word 0
.globl "__Extender"
"__Extender":
    .byte 0
.globl "__ExtenderSubtype"
"__ExtenderSubtype":
    .byte 0
.globl "__X32VM"
"__X32VM":
    .byte 0
.globl "__Envptr"
"__Envptr":
    .long 0
.globl "__Envseg"
"__Envseg":
    .word 0
.globl "__osmajor"
"__osmajor":
    .byte 0
.globl "__osminor"
"__osminor":
    .byte 0
.globl "__FPE_handler"
"__FPE_handler":
    .long "__null_FPE_rtn"
