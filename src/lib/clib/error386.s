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
.size "__doserror_",.-"__doserror_"
