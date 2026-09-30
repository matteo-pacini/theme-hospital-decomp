.intel_syntax noprefix

// FUNCTION: 0x000fbcb2

.section _TEXT,"ax",@progbits,unique,1031346
.globl "_chain_intr_"
.type "_chain_intr_",@function
"_chain_intr_":
    mov ecx, eax
    mov eax, edx
    mov esp, ebp
    xchg dword ptr [ebp + 0x28], ecx
    xchg dword ptr [ebp + 0x2c], eax
    pop gs
    pop fs
    pop es
    pop ds
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop ebx
    pop edx
    retf
.size "_chain_intr_",.-"_chain_intr_"
