.intel_syntax noprefix

// FUNCTION: 0x000ed2ca

.section _TEXT,"ax",@progbits,unique,971466
.globl "__doserror_"
.type "__doserror_",@function
"__doserror_":
    jae .L000ed2da
    and eax, 0xffff
    push eax
    call "__set_errno_dos_"
    pop eax
    jmp .L000ed2dc
.L000ed2da:
    sub eax, eax
.L000ed2dc:
    ret
    .byte 0x52
    .byte 0x89; .byte 0xc2
    .byte 0xb4; .byte 0x41
    .byte 0xcd; .byte 0x21
    .byte 0xd1; .byte 0xd0
    .byte 0xd1; .byte 0xc8
    .byte 0x89; .byte 0xc2
    .byte 0x85; .byte 0xc0
    .byte 0x7d; .byte .L000ed2fa-.-1
    .byte 0x31; .byte 0xc0
    .byte 0x66; .byte 0x89; .byte 0xd0
    .byte 0xe8; .long "__set_errno_dos_"-.-4
    .byte 0x5a
    .byte 0xc3
.L000ed2fa:
    .byte 0x31; .byte 0xc0
    .byte 0x5a
    .byte 0xc3
.size "__doserror_",.-"__doserror_"
