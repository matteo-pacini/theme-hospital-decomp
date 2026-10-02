.intel_syntax noprefix

// __sys_init_387_emulator: install the 387 emulator if no FPU answers (or ebp is nonzero on entry), then initialise the FPU state with _8087cw.
// FUNCTION: 0x000ef0a8

.section _TEXT,"ax",@progbits,unique,979112
.globl "__sys_init_387_emulator"
.type "__sys_init_387_emulator",@function
"__sys_init_387_emulator":
    push es
    push ecx
    push ebx
    push edx
    fninit
    push eax
    fnstcw word ptr [esp]
    pop eax
    cmp ah, 3
    je .L000ef0b9
    inc ebp
.L000ef0b9:
    or ebp, ebp
    je .L000ef0c2
    call "emu_install_int7"
.L000ef0c2:
    wait
    fninit
    fldcw word ptr ["__8087cw"]
    fldz
    fldz
    fldz
    fldz
    pop edx
    pop ebx
    pop ecx
    pop es
    ret
.size "__sys_init_387_emulator",.-"__sys_init_387_emulator"

// emu_install_int7: install the emulator's INT 7 (device not available) handler and enable FPU emulation, by the method the DOS extender needs.
// FUNCTION: 0x000ef0d8

.section _TEXT,"ax",@progbits,unique,979160
.globl "emu_install_int7"
.type "emu_install_int7",@function
"emu_install_int7":
    mov byte ptr [_D_00150814], 3
    mov byte ptr [_D_00151b54], 1
    .byte 0x66; .byte 0x0f; .byte 0x01; .byte 0x25; .long _D_00151b52  # smsw word ptr [0x31b52]
    and word ptr [_D_00151b52], 6
    sub esp, 8
    sidt [esp]
    mov ebx, dword ptr [esp + 2]
    add ebx, 0x38
    add esp, 8
    cmp byte ptr ["__X32VM"], 0
    je .L000ef12a
    call "emu_int7_gate"
    .byte 0x66; .byte 0x8e; .byte 0x05; .long _D_0014d5da  # mov es, word ptr [0x2d5da]
    mov dword ptr es:[ebx], edx
    mov dword ptr es:[ebx + 4], ecx
    call "pharlap_set_cr0_em"
    jmp .L000ef1a2
.L000ef12a:
    cmp byte ptr ["__Extender"], 0
    jne .L000ef14e
    mov ax, 0x2507
    push ds
    mov cx, cs
    mov ds, ecx
    lea edx, ["__int7_X32VM"+0x330]
    int 0x21
    pop ds
    mov al, 4
    mov ah, 0xf3
    int 0x21
    jmp .L000ef1a2
.L000ef14e:
    cmp byte ptr ["__Extender"], 9
    jne .L000ef176
    call "emu_int7_gate"
    mov dword ptr [ebx], edx
    mov dword ptr [ebx + 4], ecx
    mov eax, 0xe02
    mov ebx, 4
    mov ecx, dword ptr ["__GDAptr"]
    call dword ptr [ecx + 0x30]
    jmp .L000ef1a2
.L000ef176:
    cmp byte ptr ["__Extender"], 1
    jne .L000ef18f
    mov dx, word ptr ["__D16Infoseg"]
    sub eax, eax
    call "__hook387"
    jmp .L000ef1a2
.L000ef18f:
    sub dx, dx
    sub eax, eax
    call "__hook387"
    cmp al, 1
    je .L000ef1a2
    call "pharlap_install_emu"
.L000ef1a2:
    ret
.size "emu_install_int7",.-"emu_install_int7"

// emu_int7_gate: build an interrupt-gate descriptor (edx:ecx) for the emulator's INT 7 handler in the current code segment.
// FUNCTION: 0x000ef1a3

.section _TEXT,"ax",@progbits,unique,979363
.globl "emu_int7_gate"
.type "emu_int7_gate",@function
"emu_int7_gate":
    lea ecx, ["__int7_X32VM"+0x330]
    mov dx, cs
    shl edx, 0x10
    mov dx, cx
    mov cx, cs
    and cl, 3
    shl cx, 0xd
    or ch, 0x8e
    ret
.size "emu_int7_gate",.-"emu_int7_gate"

// pharlap_install_emu: Phar Lap: save the INT 7 vector, point it at the emulator and set CR0.EM (directly or through 2535h).
// FUNCTION: 0x000ef1c0

.section _TEXT,"ax",@progbits,unique,979392
.globl "pharlap_install_emu"
.type "pharlap_install_emu",@function
"pharlap_install_emu":
    mov cl, 7
    mov ax, 0x2502
    int 0x21
    mov dword ptr [_D_00151b4c], ebx
    .byte 0x66; .byte 0x8c; .byte 0x05; .long _D_00151b50  # mov word ptr [0x31b50], es
    cmp byte ptr ["__Extender"], 3
    mov cl, 7
    mov ax, 0x2504
    push ds
    push cs
    pop ds
    jl .L000ef1f7
    lea edx, ["__int7_X32VM"+0x2c0]
    int 0x21
    pop ds
    call "pharlap_set_cr0_em"
    jmp .L000ef20c
.L000ef1f7:
    lea edx, ["__int7_X32VM"+0x330]
    int 0x21
    pop ds
    mov ecx, cr0
    or ecx, 4
    and ecx, 0xfffffffd
    mov cr0, ecx
.L000ef20c:
    ret
.size "pharlap_install_emu",.-"pharlap_install_emu"

// pharlap_set_cr0_em: Phar Lap 2535h: read CR0, set EM and clear MP, write it back.
// FUNCTION: 0x000ef20d

.section _TEXT,"ax",@progbits,unique,979469
.globl "pharlap_set_cr0_em"
.type "pharlap_set_cr0_em",@function
"pharlap_set_cr0_em":
    xor ebx, ebx
    lea edx, [_D_00151b0c]
    mov ax, 0x2535
    int 0x21
    or dword ptr [_D_00151b0c], 4
    and dword ptr [_D_00151b0c], 0xfffffffd
    inc ebx
    int 0x21
    ret
.size "pharlap_set_cr0_em",.-"pharlap_set_cr0_em"

// __sys_fini_387_emulator: if the emulator was installed, undo the INT 7 hook and CR0 changes for the DOS extender in use.
// FUNCTION: 0x000ef22d

.section _TEXT,"ax",@progbits,unique,979501
.globl "__sys_fini_387_emulator"
.type "__sys_fini_387_emulator",@function
"__sys_fini_387_emulator":
    cmp byte ptr [_D_00151b54], 1
    je .L000ef237
    ret
.L000ef237:
    push ecx
    push ebx
    push edx
    mov al, byte ptr ["__Extender"]
    cmp byte ptr ["__X32VM"], 0
    je .L000ef24f
    call "pharlap_restore_cr0"
    jmp .L000ef29c
.L000ef24f:
    cmp al, 9
    jne .L000ef268
    mov eax, 0xe02
    mov ebx, 0
    mov ecx, dword ptr ["__GDAptr"]
    call dword ptr [ecx + 0x30]
    jmp .L000ef29c
.L000ef268:
    cmp al, 0
    jne .L000ef278
    mov ax, word ptr [_D_00151b52]
    mov ah, 0xf3
    int 0x21
    jmp .L000ef29c
.L000ef278:
    cmp al, 1
    jne .L000ef28c
    mov dx, word ptr ["__D16Infoseg"]
    sub eax, eax
    call "__unhook387"
    jmp .L000ef29c
.L000ef28c:
    sub edx, edx
    call "__unhook387"
    cmp al, 1
    je .L000ef29c
    call "pharlap_remove_emu"
.L000ef29c:
    mov byte ptr [_D_00151b54], 0
    pop edx
    pop ebx
    pop ecx
    ret
.size "__sys_fini_387_emulator",.-"__sys_fini_387_emulator"

// pharlap_remove_emu: Phar Lap: restore the saved INT 7 vector and CR0's original EM/MP bits.
// FUNCTION: 0x000ef2a7

.section _TEXT,"ax",@progbits,unique,979623
.globl "pharlap_remove_emu"
.type "pharlap_remove_emu",@function
"pharlap_remove_emu":
    mov cl, 7
    mov ax, 0x2504
    mov edx, dword ptr [_D_00151b4c]
    push ds
    .byte 0x66; .byte 0x8e; .byte 0x1d; .long _D_00151b50  # mov ds, word ptr [0x31b50]
    int 0x21
    pop ds
    cmp byte ptr ["__Extender"], 3
    jl .L000ef2ce
    call "pharlap_restore_cr0"
    jmp .L000ef2e1
.L000ef2ce:
    mov ecx, cr0
    and ecx, 0xfffffff9
    mov dx, word ptr [_D_00151b52]
    or cx, dx
    mov cr0, ecx
.L000ef2e1:
    ret
.size "pharlap_remove_emu",.-"pharlap_remove_emu"

// pharlap_restore_cr0: Phar Lap 2535h: read the system registers, set the CR0 slot to just the saved EM/MP bits and write them back.
// FUNCTION: 0x000ef2e2

.section _TEXT,"ax",@progbits,unique,979682
.globl "pharlap_restore_cr0"
.type "pharlap_restore_cr0",@function
"pharlap_restore_cr0":
    xor ebx, ebx
    lea edx, [_D_00151b0c]
    mov ax, 0x2535
    int 0x21
    xor ecx, ecx
    mov cx, word ptr [_D_00151b52]
    mov dword ptr [_D_00151b0c], ecx
    inc ebx
    int 0x21
    ret
    .byte 0x9b
    .byte 0xdd; .byte 0x30
    .byte 0x9b
    .byte 0xc3
    .byte 0xdd; .byte 0x20
    .byte 0x9b
    .byte 0xc3
.size "pharlap_restore_cr0",.-"pharlap_restore_cr0"

// __init_8087_: with a real FPU, install the fsave/frstor routines as __Save8087/__Rest8087; then initialise the FPU with _8087cw (__init_80x87).
// FUNCTION: 0x000ef30c

.section _TEXT,"ax",@progbits,unique,979724
.globl "__init_8087_"
.type "__init_8087_",@function
"__init_8087_":
    push ebx
    push edx
    cmp byte ptr [__real87], 0
    je .L000ef32d
    .byte 0xba; .long "pharlap_restore_cr0"+0x21  # mov edx, 0xdf303
    .byte 0xbb; .long "pharlap_restore_cr0"+0x26  # mov ebx, 0xdf308
    mov dword ptr ["___Save8087"], edx
    mov dword ptr ["___Rest8087"], ebx
.L000ef32d:
    xor eax, eax
    mov ax, word ptr ["__8087cw"]
    call "__init_80x87"
    pop edx
    pop ebx
    ret
    .byte 0x80; .byte 0x3d; .long __real87; .byte 0x00
    .byte 0x75; .byte 0xc6
    .byte 0xc3
.L000ef347:
    .byte 0x52
    .byte 0x8a; .byte 0x25; .long _D_00150814
    .byte 0x84; .byte 0xe4
    .byte 0x75; .byte .L000ef38b-.-1
    .byte 0x88; .byte 0x25; .long __real87
    .byte 0x2b; .byte 0xc0
    .byte 0x50
    .byte 0xdb; .byte 0xe3
    .byte 0xd9; .byte 0x3c; .byte 0x24
    .byte 0x58
    .byte 0x8a; .byte 0xc4
    .byte 0x30; .byte 0xd2
    .byte 0x25; .byte 0xff; .byte 0x00; .byte 0x00; .byte 0x00
    .byte 0x83; .byte 0xf8; .byte 0x03
    .byte 0x75; .byte .L000ef376-.-1
    .byte 0xe8; .long -4; .reloc .-4, R_386_PC32, "__init_8087_"
    .byte 0x88; .byte 0xc2
.L000ef376:
    .byte 0x80; .byte 0x3d; .long "__no87"; .byte 0x00
    .byte 0x75; .byte .L000ef38b-.-1
    .byte 0x88; .byte 0x15; .long _D_00150814
    .byte 0x88; .byte 0x15; .long __real87
.L000ef38b:
    .byte 0x5a
    .byte 0xc3
.size "__init_8087_",.-"__init_8087_"

// _8087386's XI entry: __chk8087 (0xef347) at priority 2.
.section XI,"aw",@progbits
    .byte 0x00, 0x02
    .long .L000ef347
