.intel_syntax noprefix

// FUNCTION: 0x000bcc9d

.section _TEXT,"ax",@progbits,unique,773277
.globl "__do_exit_with_msg__"
.type "__do_exit_with_msg__",@function
"__do_exit_with_msg__":
    push edx
    push eax
    .byte 0xba; .long "_cstart_"+0x74  # mov edx, 0xaca80
    mov ax, 0x3d01
    int 0x21
    mov bx, ax
    pop edx
    mov esi, edx
    cld
.L000bccb1:
    lodsb al, byte ptr [esi]
    cmp al, 0
    jne .L000bccb1
    mov ecx, esi
    sub ecx, edx
    dec ecx
    mov ah, 0x40
    int 0x21
    pop eax
    push eax
    mov eax, 0
    mov edx, 0xff
    call "__FiniRtns"
    pop eax
    mov ah, 0x4c
    int 0x21
    .byte 0x8d; .byte 0x40; .byte 0x00  # lea eax, [eax]
.size "__do_exit_with_msg__",.-"__do_exit_with_msg__"

// FUNCTION: 0x000bccf0

.section _TEXT,"ax",@progbits,unique,773360
.globl "___GETDSEnd_"
.type "___GETDSEnd_",@function
"___GETDSEnd_":
    push ebp
    mov ebp, esp
    sub esp, 4
    mov dword ptr [ebp - 4], eax
    mov eax, dword ptr [ebp - 4]
    mov dword ptr [_D_0014d5e4], eax
    mov esp, ebp
    pop ebp
    ret
    .byte 0x8d; .byte 0x40; .byte 0x00
    .byte 0x55
    .byte 0x89; .byte 0xe5
    .byte 0x83; .byte 0xec; .byte 0x04
    .byte 0x89; .byte 0x45; .byte 0xfc
    .byte 0x8b; .byte 0x45; .byte 0xfc
    .byte 0x89; .byte 0xec
    .byte 0x5d
    .byte 0xc3
.size "___GETDSEnd_",.-"___GETDSEnd_"
