.intel_syntax noprefix

// _dos_read: read up to count bytes from handle into a far buffer (DOS 3Fh), bytes read in *bytes; returns 0 or the DOS error.
// FUNCTION: 0x000fa918

.section _TEXT,"ax",@progbits,unique,1026328
.globl "_dos_read_"
.type "_dos_read_",@function
"_dos_read_":
    push 0x3f00
    push ds
    mov ds, ecx
    mov ecx, edx
    mov edx, ebx
    mov ebx, eax
    mov eax, dword ptr [esp + 4]
    int 0x21
    pop ds
    push ds
    jb .L000fa936
    mov ebx, dword ptr [esp + 0xc]
    mov dword ptr [ebx], eax
.L000fa936:
    call "__doserror_"
    pop ds
    add esp, 4
    ret 4
.size "_dos_read_",.-"_dos_read_"

// _dos_write: write count bytes from a far buffer to handle (DOS 40h), bytes written in *bytes; returns 0 or the DOS error. Shares _dos_read's body.
// FUNCTION: 0x000fa942

.section _TEXT,"ax",@progbits,unique,1026370
.globl "_dos_write_"
.type "_dos_write_",@function
"_dos_write_":
    push 0x4000
    .byte 0xeb; .byte 0xd4  # jmp 0xfa91d
.size "_dos_write_",.-"_dos_write_"
