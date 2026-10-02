.intel_syntax noprefix

// __doserror: after a DOS call, return 0 if carry is clear, else set errno from DOS error ax and return that error code.
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
