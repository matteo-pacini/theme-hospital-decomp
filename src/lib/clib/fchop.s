.intel_syntax noprefix

// FUNCTION: 0x000fca7a

.section _TEXT,"ax",@progbits,unique,1034874
.globl "__CHP"
.type "__CHP",@function
"__CHP":
    push eax
    wait
    fnstcw word ptr [esp]
    wait
    push dword ptr [esp]
    mov byte ptr [esp + 1], 0x1f
    fldcw word ptr [esp]
    frndint
    fldcw word ptr [esp + 4]
    wait
    lea esp, [esp + 8]
    ret
.size "__CHP",.-"__CHP"
