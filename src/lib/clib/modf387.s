.intel_syntax noprefix

// FUNCTION: 0x000fcf6c

.section _TEXT,"ax",@progbits,unique,1036140
.globl "modf_"
.type "modf_",@function
"modf_":
    push eax
    fld qword ptr [esp + 8]
    fld st(0)
    call "__CHP"
    fsub st(1), st(0)
    mov eax, dword ptr [esp + 0x10]
    fstp qword ptr [eax]
    pop eax
    wait
    ret 0xc
.size "modf_",.-"modf_"
