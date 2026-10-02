.intel_syntax noprefix

// Raises the timer lock count. While it is above zero the timer interrupt still
// counts due periods but defers the callbacks until AIL_unlock.
// FUNCTION: 0x000ddf88

.section _TEXT,"ax",@progbits,unique,909192
.globl "_AIL_API_lock"
.type "_AIL_API_lock",@function
"_AIL_API_lock":
    inc dword ptr [_D_001509fa]
    ret
.size "_AIL_API_lock",.-"_AIL_API_lock"

// Lowers the timer lock count raised by AIL_lock.
// FUNCTION: 0x000ddf8f

.section _TEXT,"ax",@progbits,unique,909199
.globl "_AIL_API_unlock"
.type "_AIL_API_unlock",@function
"_AIL_API_unlock":
    dec dword ptr [_D_001509fa]
    ret
.size "_AIL_API_unlock",.-"_AIL_API_unlock"

// Returns milliseconds since the timer system started, never less than the
// previous result. Uses the Windows VxD entry point when present, otherwise the
// interrupt count times the current PIT period plus the elapsed part of the
// current PIT countdown.
// FUNCTION: 0x000ddf96

.section _TEXT,"ax",@progbits,unique,909206
.globl "_AIL_ms_count"
.type "_AIL_ms_count",@function
"_AIL_ms_count":
    push ebx
    cmp dword ptr [_D_001509dc], 0
    jne .L000ddfaa
    cmp word ptr [_D_001509e0], 0
    je .L000ddfb7
.L000ddfaa:
    mov eax, 0x101
    .byte 0xff; .byte 0x1d; .long _D_001509dc  # call ptr [0x309dc]
    jmp .L000de018
.L000ddfb7:
    mov ecx, dword ptr [_D_001509cc]
    {load} xor al, al
    out 0x43, al
    jmp .L000ddfc3
.L000ddfc3:
    in al, 0x40
    {load} mov ah, al
    jmp .L000ddfc9
.L000ddfc9:
    in al, 0x40
    xchg ah, al
    cmp ecx, dword ptr [_D_001509cc]
    jne .L000ddfb7
    and eax, 0xffff
    je .L000de000
    mov ebx, dword ptr [_D_001509ee]
    cmp ebx, 0
    jne .L000ddfec
    mov ebx, 0x10000
.L000ddfec:
    {load} sub ebx, eax
    jae .L000ddff2
    {load} xor ebx, ebx
.L000ddff2:
    mov eax, 0x3e8
    mul ebx
    mov ebx, 0x1234dd
    div ebx
.L000de000:
    {load} mov ebx, eax
    mov eax, dword ptr [_D_001509f2]
    mul ecx
    mov ecx, 0x3e8
    div ecx
    {load} add eax, ebx
    add eax, dword ptr [_D_001509d4]
.L000de018:
    cmp eax, dword ptr [_D_001509d0]
    jae .L000de025
    mov eax, dword ptr [_D_001509d0]
.L000de025:
    mov dword ptr [_D_001509d0], eax
    pop ebx
    ret
.size "_AIL_ms_count",.-"_AIL_ms_count"

// After the PIT is reprogrammed, restarts the interrupt count and makes the
// current AIL_ms_count value its base, so the millisecond count carries on
// unbroken. Does nothing before the first AIL_ms_count call.
// FUNCTION: 0x000de02c

.section _TEXT,"ax",@progbits,unique,909356
.globl "_setup_ms_count"
.type "_setup_ms_count",@function
"_setup_ms_count":
    cmp dword ptr [_D_001509d0], 0
    je .L000de049
    call "_AIL_ms_count"
    mov dword ptr [_D_001509cc], 0
    mov dword ptr [_D_001509d4], eax
.L000de049:
    ret
.size "_setup_ms_count",.-"_setup_ms_count"

// INT 8 handler. Adds the time since the last interrupt (the PIT period, or
// the VxD's elapsed time capped at one second) to every running timer and
// counts each period that has elapsed, acknowledges the PIC and, unless
// AIL_lock is held, calls the callbacks of the 15 user timers once per elapsed
// period with their user values, on a private stack. The 16th timer stands
// for the BIOS clock: its elapsed periods chain to the previous INT 8 handler.
// A nested interrupt is only acknowledged.
// FUNCTION: 0x000de04a

.section _TEXT,"ax",@progbits,unique,909386
.globl "_API_timer"
.type "_API_timer",@function
"_API_timer":
    push ds
    .byte 0x66; .byte 0x2e; .byte 0x8e; .byte 0x1d; .long _D_001509fe  # mov ds, word ptr cs:[0x309fe]
    inc dword ptr [_D_001509cc]
    cmp dword ptr [_D_001509f6], 0
    ja .L000de1e5
    .byte 0x60  # pushal
    push es
    push fs
    push gs
    {load} mov ebp, esp
    cld
    .byte 0x66; .byte 0x8e; .byte 0x05; .long _D_001509fe  # mov es, word ptr [0x309fe]
    inc dword ptr [_D_001509f6]
    mov word ptr [_D_00151410], ss
    mov dword ptr [_D_00151414], esp
    mov ax, ds
    .byte 0x66; .byte 0x8e; .byte 0xd0  # mov ss, ax
    .byte 0xbc; .long _D_00151410  # mov esp, 0x31410
    cmp dword ptr [_D_001509dc], 0
    jne .L000de0a6
    cmp word ptr [_D_001509e0], 0
    je .L000de0cc
.L000de0a6:
    mov eax, 0x101
    .byte 0xff; .byte 0x1d; .long _D_001509dc  # call ptr [0x309dc]
    mov ecx, 0x3e8
    cmp dword ptr [_D_001509d8], 0
    jne .L000de0ed
    mov edx, dword ptr [_D_001509f2]
    mov dword ptr [_D_001509d8], eax
    jmp .L000de10d
.L000de0cc:
    mov eax, dword ptr [_D_001509cc]
    cmp dword ptr [_D_001509d8], 0
    jne .L000de0e7
    mov edx, dword ptr [_D_001509f2]
    mov dword ptr [_D_001509d8], eax
    jmp .L000de10d
.L000de0e7:
    mov ecx, dword ptr [_D_001509f2]
.L000de0ed:
    {load} mov edx, eax
    sub eax, dword ptr [_D_001509d8]
    mov dword ptr [_D_001509d8], edx
    {load} xor edx, edx
    mul ecx
    {load} mov edx, eax
    cmp eax, 0xf4240
    jb .L000de10d
    mov edx, 0xf4240
.L000de10d:
    mov edi, 0
.L000de112:
    cmp dword ptr [edi + _D_0015088c], 2
    jne .L000de13f
    mov eax, dword ptr [edi + _D_001508cc]
    {load} add eax, edx
.L000de123:
    cmp eax, dword ptr [edi + _D_0015090c]
    jb .L000de139
    sub eax, dword ptr [edi + _D_0015090c]
    inc dword ptr [edi + _D_0015094c]
    jmp .L000de123
.L000de139:
    mov dword ptr [edi + _D_001508cc], eax
.L000de13f:
    add edi, 4
    cmp edi, 0x40
    jb .L000de112
    mov al, 0x20
    out 0x20, al
    sti
    cmp dword ptr [_D_001509fa], 0
    jg .L000de180
    mov edi, 0
.L000de15a:
    cmp dword ptr [edi + _D_0015094c], 0
    je .L000de178
    dec dword ptr [edi + _D_0015094c]
    mov eax, dword ptr [edi + _D_0015098c]
    push eax
    call dword ptr [edi + _D_0015084c]
    jmp .L000de15a
.L000de178:
    add edi, 4
    cmp edi, 0x3c
    jb .L000de15a
.L000de180:
    cmp dword ptr [_D_00150988], 0
    je .L000de1c9
    dec dword ptr [_D_00150988]
    mov ss, word ptr [_D_00151410]
    mov esp, dword ptr [_D_00151414]
    dec dword ptr [_D_001509f6]
    mov ecx, dword ptr [_D_001509e2]
    movzx eax, word ptr [_D_001509e6]
    {load} mov esp, ebp
    mov ds, word ptr [ebp + 0x2c]
    mov dword ptr [ebp + 0x2c], eax
    mov eax, dword ptr [ebp + 0x28]
    mov dword ptr [ebp + 0x28], ecx
    pop gs
    pop fs
    pop es
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop ebx
    pop edx
    pop ecx
    retf
.L000de1c9:
    mov ss, word ptr [_D_00151410]
    mov esp, dword ptr [_D_00151414]
    dec dword ptr [_D_001509f6]
    {load} mov esp, ebp
    pop gs
    pop fs
    pop es
    .byte 0x61  # popal
    pop ds
    iretd
.L000de1e5:
    push eax
    mov al, 0x20
    out 0x20, al
    pop eax
    pop ds
    iretd
.size "_API_timer",.-"_API_timer"

// Programs PIT channel 0 (rate generator) with `divisor` (0 = 65536) and
// records it for AIL_interrupt_divisor.
// FUNCTION: 0x000de1ed

.section _TEXT,"ax",@progbits,unique,909805
.globl "_set_PIT_divisor"
.type "_set_PIT_divisor",@function
"_set_PIT_divisor":
    push ebp
    {load} mov ebp, esp
    pushfd
    cli
    mov al, 0x34
    out 0x43, al
    mov eax, dword ptr [ebp + 8]
    mov dword ptr [_D_001509ee], eax
    jmp .L000de200
.L000de200:
    out 0x40, al
    {load} mov al, ah
    jmp .L000de206
.L000de206:
    out 0x40, al
    push ebp
    {load} mov ebp, esp
    test byte ptr [ebp + 5], 2
    cli
    je .L000de213
    sti
.L000de213:
    pop ebp
    popfd
    leave
    ret
.size "_set_PIT_divisor",.-"_set_PIT_divisor"

// Programs the PIT for an interrupt every `period` microseconds; 54925 or more
// gives the BIOS rate (divisor 65536).
// FUNCTION: 0x000de217

.section _TEXT,"ax",@progbits,unique,909847
.globl "_set_PIT_period"
.type "_set_PIT_period",@function
"_set_PIT_period":
    push ebp
    {load} mov ebp, esp
    push ebx
    mov eax, 0
    cmp dword ptr [ebp + 8], 0xd68d
    jae .L000de23a
    mov eax, dword ptr [ebp + 8]
    mov ebx, 0x20bc
    mov ecx, 0x2710
    mul ecx
    div ebx
.L000de23a:
    push eax
    call "_set_PIT_divisor"
    add esp, 4
    pop ebx
    leave
    ret
.size "_set_PIT_period",.-"_set_PIT_period"

// Sets the PIT to the shortest period among the allocated timers, if that
// changed, then clears every timer's accumulated time and rebases the
// millisecond count.
// FUNCTION: 0x000de246

.section _TEXT,"ax",@progbits,unique,909894
.globl "_program_timers"
.type "_program_timers",@function
"_program_timers":
    push edi
    push es
    call "_AIL_lock"
    push ds
    pop es
    cld
    mov ecx, 0xffffffff
    mov edi, 0
.L000de25a:
    cmp dword ptr [edi + _D_0015088c], 0
    je .L000de26f
    mov eax, dword ptr [edi + _D_0015090c]
    {load} cmp eax, ecx
    jae .L000de26f
    {load} mov ecx, eax
.L000de26f:
    add edi, 4
    cmp edi, 0x40
    jb .L000de25a
    cmp ecx, dword ptr [_D_001509f2]
    je .L000de2a4
    mov dword ptr [_D_001509f2], ecx
    push ecx
    call "_set_PIT_period"
    add esp, 4
    mov eax, 0
    .byte 0xbf; .long _D_001508cc  # mov edi, 0x308cc
    mov ecx, 0x10
    rep stosd dword ptr es:[edi], eax
    call "_setup_ms_count"
.L000de2a4:
    call "_AIL_unlock"
    pop es
    pop edi
    ret
.size "_program_timers",.-"_program_timers"

// Starts the timer system: locks its memory, clears the timer table, looks up
// the Windows VxD entry point (INT 2Fh AX=1684h), saves the real- and
// protected-mode INT 8 vectors, installs the timer interrupt handler and runs
// the 16th timer at the BIOS clock period so the BIOS still gets its ticks.
// FUNCTION: 0x000de2ac

.section _TEXT,"ax",@progbits,unique,909996
.globl "_AILA_startup"
.type "_AILA_startup",@function
"_AILA_startup":
    push ebx
    push edi
    push es
    pushfd
    cli
    push ds
    pop es
    cld
    .byte 0x66; .byte 0x8c; .byte 0x1d; .long _D_001509fe  # mov word ptr [0x309fe], ds
    .byte 0x66; .byte 0x8e; .byte 0x05; .long _D_001509fe  # mov es, word ptr [0x309fe]
    call "_AILA_VMM_lock"
    mov dword ptr [_D_001509f6], 0
    mov dword ptr [_D_001509fa], 0
    mov dword ptr [_D_001509f2], 0xffffffff
    mov dword ptr [_D_00150a02], 0xffffffff
    mov eax, 0
    .byte 0xbf; .long _D_0015088c  # mov edi, 0x3088c
    mov ecx, 0x10
    rep stosd dword ptr es:[edi], eax
    .byte 0xbf; .long _D_001508cc  # mov edi, 0x308cc
    mov ecx, 0x10
    rep stosd dword ptr es:[edi], eax
    .byte 0xbf; .long _D_0015090c  # mov edi, 0x3090c
    mov ecx, 0x10
    rep stosd dword ptr es:[edi], eax
    .byte 0xbf; .long _D_0015094c  # mov edi, 0x3094c
    mov ecx, 0x10
    rep stosd dword ptr es:[edi], eax
    push es
    mov eax, 0x1684
    mov ebx, 5
    {load} xor edi, edi
    .byte 0x66; .byte 0x8e; .byte 0xc7  # mov es, di
    int 0x2f
    mov word ptr [_D_001509dc], di
    mov word ptr [_D_001509de], 0
    .byte 0x66; .byte 0x8c; .byte 0x05; .long _D_001509e0  # mov word ptr [0x309e0], es
    pop es
    mov eax, 8
    push eax
    {load} mov ebx, eax
    mov eax, 0x200
    int 0x31
    shl ecx, 0x10
    {load} mov cx, dx
    pop eax
    push ecx
    mov ah, 0x35
    push es
    int 0x21
    mov dx, es
    pop es
    pop ecx
    mov dword ptr [_D_001509e2], ebx
    mov word ptr [_D_001509e6], dx
    mov dword ptr [_D_001509ea], ecx
    mov eax, 8
    .byte 0xba; .long "_API_timer"  # mov edx, 0xce04a
    mov bx, cs
    mov ah, 0x25
    push ds
    .byte 0x66; .byte 0x8e; .byte 0xdb  # mov ds, bx
    int 0x21
    pop ds
    mov dword ptr [_D_001508c8], 2
    push 0xd68d
    push 0x3c
    call "_AIL_set_timer_period"
    add esp, 8
    push ebp
    {load} mov ebp, esp
    test byte ptr [ebp + 5], 2
    cli
    je .L000de3bc
    sti
.L000de3bc:
    pop ebp
    popfd
    pop es
    pop edi
    pop ebx
    ret
.size "_AILA_startup",.-"_AILA_startup"

// Stops the timer system: restores the PIT to the BIOS rate and the original
// protected-mode INT 8 vector.
// FUNCTION: 0x000de3c2

.section _TEXT,"ax",@progbits,unique,910274
.globl "_AILA_shutdown"
.type "_AILA_shutdown",@function
"_AILA_shutdown":
    push ebx
    pushfd
    cli
    push 0
    call "_set_PIT_divisor"
    add esp, 4
    mov eax, 8
    mov edx, dword ptr [_D_001509e2]
    movzx ebx, word ptr [_D_001509e6]
    mov ah, 0x25
    push ds
    .byte 0x66; .byte 0x8e; .byte 0xdb  # mov ds, bx
    int 0x21
    pop ds
    push ebp
    {load} mov ebp, esp
    test byte ptr [ebp + 5], 2
    cli
    je .L000de3f5
    sti
.L000de3f5:
    pop ebp
    popfd
    pop ebx
    ret
.size "_AILA_shutdown",.-"_AILA_shutdown"

// Returns real-mode interrupt vector `vectnum` as a segment:offset pair
// (segment in the high word).
// FUNCTION: 0x000de3f9

.section _TEXT,"ax",@progbits,unique,910329
.globl "_AIL_API_get_real_vect"
.type "_AIL_API_get_real_vect",@function
"_AIL_API_get_real_vect":
    push ebp
    {load} mov ebp, esp
    push ebx
    mov eax, dword ptr [ebp + 8]
    push eax
    {load} mov ebx, eax
    mov eax, 0x200
    int 0x31
    shl ecx, 0x10
    {load} mov cx, dx
    pop eax
    push ecx
    mov ah, 0x35
    push es
    int 0x21
    mov dx, es
    pop es
    pop ecx
    {load} mov eax, ecx
    pop ebx
    leave
    ret
.size "_AIL_API_get_real_vect",.-"_AIL_API_get_real_vect"

// Sets real-mode interrupt vector `vectnum` to the segment:offset pair
// `real_ptr` (segment in the high word).
// FUNCTION: 0x000de421

.section _TEXT,"ax",@progbits,unique,910369
.globl "_AIL_API_set_real_vect"
.type "_AIL_API_set_real_vect",@function
"_AIL_API_set_real_vect":
    push ebp
    {load} mov ebp, esp
    push ebx
    mov eax, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebp + 0xc]
    {load} mov ecx, ebx
    shr ecx, 0x10
    {load} mov edx, ebx
    and edx, 0xffff
    {load} mov ebx, eax
    mov eax, 0x201
    int 0x31
    pop ebx
    leave
    ret
.size "_AIL_API_set_real_vect",.-"_AIL_API_set_real_vect"

// Template for the protected-mode stub AIL_set_USE16_ISR installs: its 0x9999
// placeholders are patched with the driver's selectors, a private stack and the
// ISR's address. It switches to those, far-returns into the 16-bit driver ISR
// with an interrupt frame that leads back into its own tail (bytes after the
// retf), which restores the caller's stack and registers and irets.
// FUNCTION: 0x000de444

.section _TEXT,"ax",@progbits,unique,910404
.globl "_thunk_proc"
.type "_thunk_proc",@function
"_thunk_proc":
    push ds
    push es
    push eax
    push ebx
    push ecx
    mov ax, 0x9999
    .byte 0x66; .byte 0x8e; .byte 0xd8  # mov ds, ax
    .byte 0x66; .byte 0x8e; .byte 0xc0  # mov es, ax
    mov bx, ss
    {load} mov ecx, esp
    mov ax, 0x9999
    .byte 0x66; .byte 0x8e; .byte 0xd0  # mov ss, ax
    mov esp, 0x99999999
    .byte 0x66; .byte 0x9c  # pushf
    pop ax
    mov word ptr [esp - 2], ax
    mov word ptr [esp - 4], cs
    mov word ptr [esp - 6], 0x9999
    sub esp, 6
    mov word ptr [esp - 2], 0x9999
    mov word ptr [esp - 4], 0x9999
    sub esp, 4
    .byte 0x66; .byte 0xcb  # retf
    .byte 0x66; .byte 0x8e; .byte 0xd3
    .byte 0x8b; .byte 0xe1
    .byte 0x59
    .byte 0x5b
    .byte 0x58
    .byte 0x07
    .byte 0x1f
    .byte 0xcf
.size "_thunk_proc",.-"_thunk_proc"

// Routes hardware interrupt `IRQ` in protected mode to the 16-bit driver ISR at
// offset `ISR_offset` of the real-mode segment in `real_base`'s high word:
// creates code, data and stack descriptors, patches thunk_proc with them and
// installs it, saving the old vector. Only one such ISR can be installed;
// further calls do nothing until AIL_restore_USE16_ISR.
// FUNCTION: 0x000de499

.section _TEXT,"ax",@progbits,unique,910489
.globl "_AIL_API_set_USE16_ISR"
.type "_AIL_API_set_USE16_ISR",@function
"_AIL_API_set_USE16_ISR":
    push ebp
    {load} mov ebp, esp
    add esp, -0x1c
    push ebx
    push esi
    push edi
    push es
    cmp dword ptr [_D_00150a02], -1
    jne .L000de6a9
    mov edx, dword ptr [ebp + 0xc]
    {load} mov eax, edx
    and edx, 0xffff
    shr eax, 0x10
    shl eax, 4
    {load} add eax, edx
    mov dword ptr [ebp - 4], eax
    mov eax, 0
    mov ecx, 1
    int 0x31
    and eax, 0xffff
    mov dword ptr [ebp - 8], eax
    mov eax, 7
    mov ebx, dword ptr [ebp - 8]
    mov cx, word ptr [ebp - 2]
    mov dx, word ptr [ebp - 4]
    int 0x31
    mov eax, 8
    mov ebx, dword ptr [ebp - 8]
    mov ecx, 0
    mov edx, 0xffff
    int 0x31
    mov eax, 9
    mov ebx, dword ptr [ebp - 8]
    mov edx, ds
    lar ecx, edx
    shr ecx, 8
    or cl, 8
    mov ch, 0
    int 0x31
    mov eax, 0
    mov ecx, 1
    int 0x31
    and eax, 0xffff
    mov dword ptr [ebp - 0xc], eax
    mov eax, 7
    mov ebx, dword ptr [ebp - 0xc]
    mov cx, word ptr [ebp - 2]
    mov dx, word ptr [ebp - 4]
    int 0x31
    mov eax, 8
    mov ebx, dword ptr [ebp - 0xc]
    mov ecx, 0
    mov edx, 0xffff
    int 0x31
    mov eax, 9
    mov ebx, dword ptr [ebp - 0xc]
    mov edx, ds
    lar ecx, edx
    shr ecx, 8
    mov ch, 0
    int 0x31
    mov eax, 0
    mov ecx, 1
    int 0x31
    and eax, 0xffff
    mov dword ptr [ebp - 0x10], eax
    .byte 0xb9; .long _D_00150a10  # mov ecx, 0x30a10
    and ecx, 0xfffffff0
    .byte 0xb8; .long _D_00150a10  # mov eax, 0x30a10
    {load} sub eax, ecx
    mov dword ptr [ebp - 0x14], eax
    mov eax, 7
    mov ebx, dword ptr [ebp - 0x10]
    {load} mov edx, ecx
    and edx, 0xffff
    shr ecx, 0x10
    int 0x31
    mov eax, 8
    mov ebx, dword ptr [ebp - 0x10]
    mov ecx, 0
    mov edx, 0xffff
    int 0x31
    mov eax, 9
    mov ebx, dword ptr [ebp - 0x10]
    mov edx, ds
    lar ecx, edx
    shr ecx, 8
    mov ch, 0
    int 0x31
    mov eax, 0
    mov ecx, 1
    int 0x31
    and eax, 0xffff
    mov dword ptr [ebp - 0x18], eax
    .byte 0xb9; .long "_thunk_proc"  # mov ecx, 0xce444
    and ecx, 0xfffffff0
    .byte 0xb8; .long "_thunk_proc"  # mov eax, 0xce444
    {load} sub eax, ecx
    mov dword ptr [ebp - 0x1c], eax
    mov eax, 7
    mov ebx, dword ptr [ebp - 0x18]
    {load} mov edx, ecx
    and edx, 0xffff
    shr ecx, 0x10
    int 0x31
    mov eax, 8
    mov ebx, dword ptr [ebp - 0x18]
    mov ecx, 0xffff
    mov edx, 0xffff
    int 0x31
    mov eax, 9
    mov ebx, dword ptr [ebp - 0x18]
    mov edx, ds
    lar ecx, edx
    shr ecx, 8
    or cl, 8
    mov ch, 0xc0
    int 0x31
    mov eax, dword ptr [ebp - 0x14]
    add eax, 0x200
    mov dword ptr ["_thunk_proc"+0x1c], eax
    mov eax, dword ptr [ebp - 0x10]
    mov word ptr ["_thunk_proc"+0x16], ax
    mov eax, dword ptr [ebp - 0xc]
    mov word ptr ["_thunk_proc"+0x7], ax
    mov eax, dword ptr [ebp - 8]
    mov word ptr ["_thunk_proc"+0x3c], ax
    mov eax, dword ptr [ebp + 0x10]
    mov word ptr ["_thunk_proc"+0x43], ax
    mov eax, 0x4a
    add eax, dword ptr [ebp - 0x1c]
    mov word ptr ["_thunk_proc"+0x32], ax
    mov ebx, dword ptr [ebp + 8]
    mov dword ptr [_D_00150a02], ebx
    cmp ebx, 8
    jb .L000de67e
    add ebx, 0x60
.L000de67e:
    add ebx, 8
    push ebx
    mov eax, 0x204
    int 0x31
    and ecx, 0xffff
    mov dword ptr [_D_00150a06], ecx
    mov dword ptr [_D_00150a0a], edx
    pop ebx
    mov eax, 0x205
    mov ecx, dword ptr [ebp - 0x18]
    mov edx, dword ptr [ebp - 0x1c]
    int 0x31
.L000de6a9:
    pop es
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AIL_API_set_USE16_ISR",.-"_AIL_API_set_USE16_ISR"

// Restores the protected-mode vector of `IRQ` saved by AIL_set_USE16_ISR, if
// that is the IRQ currently installed.
// FUNCTION: 0x000de6af

.section _TEXT,"ax",@progbits,unique,911023
.globl "_AIL_API_restore_USE16_ISR"
.type "_AIL_API_restore_USE16_ISR",@function
"_AIL_API_restore_USE16_ISR":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    push es
    mov ebx, dword ptr [ebp + 8]
    cmp ebx, -1
    je .L000de6ee
    cmp ebx, dword ptr [_D_00150a02]
    jne .L000de6ee
    cmp ebx, 8
    jb .L000de6ce
    add ebx, 0x60
.L000de6ce:
    add ebx, 8
    mov eax, 0x205
    mov ecx, dword ptr [_D_00150a06]
    mov edx, dword ptr [_D_00150a0a]
    int 0x31
    mov dword ptr [_D_00150a02], 0xffffffff
.L000de6ee:
    pop es
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AIL_API_restore_USE16_ISR",.-"_AIL_API_restore_USE16_ISR"

// Disables interrupts; returns the EFLAGS value from before, for
// AIL_restore_interrupts.
// FUNCTION: 0x000de6f4

.section _TEXT,"ax",@progbits,unique,911092
.globl "_AIL_API_disable_interrupts"
.type "_AIL_API_disable_interrupts",@function
"_AIL_API_disable_interrupts":
    pushfd
    pop eax
    cli
    ret
.size "_AIL_API_disable_interrupts",.-"_AIL_API_disable_interrupts"

// Restores EFLAGS (and so the interrupt flag) from `flags`, as returned by
// AIL_disable_interrupts.
// FUNCTION: 0x000de6f8

.section _TEXT,"ax",@progbits,unique,911096
.globl "_AIL_API_restore_interrupts"
.type "_AIL_API_restore_interrupts",@function
"_AIL_API_restore_interrupts":
    push ebp
    {load} mov ebp, esp
    push dword ptr [ebp + 8]
    push ebp
    {load} mov ebp, esp
    test byte ptr [ebp + 5], 2
    cli
    je .L000de709
    sti
.L000de709:
    pop ebp
    popfd
    leave
    ret
.size "_AIL_API_restore_interrupts",.-"_AIL_API_restore_interrupts"

// Moves the caller onto a new stack in DS ending at `base` + `size` (bit 2 of
// the end cleared): stores SS, ESP and EBP through `ss_save`, `esp_save` and
// `ebp_save`, points EBP at the new top with room for the caller's locals below
// it, and returns. Nothing is copied from the old frame. Undone by
// AIL_restore_stack.
// FUNCTION: 0x000de70d

.section _TEXT,"ax",@progbits,unique,911117
.globl "_AIL_switch_stack"
.type "_AIL_switch_stack",@function
"_AIL_switch_stack":
    mov eax, dword ptr [esp + 0x10]
    mov dword ptr [eax], esp
    mov eax, dword ptr [esp + 0x14]
    mov dword ptr [eax], ebp
    mov eax, dword ptr [esp + 0xc]
    mov word ptr [eax], ss
    mov eax, dword ptr [esp + 4]
    add eax, dword ptr [esp + 8]
    and eax, 0xfffffffb
    mov edx, dword ptr [esp]
    add esp, 0x18
    {load} sub ebp, esp
    mov ecx, ds
    mov ss, ecx
    {load} mov esp, eax
    {load} mov eax, ebp
    {load} mov ebp, esp
    {load} sub esp, eax
    sub esp, 0x14
    jmp edx
.size "_AIL_switch_stack",.-"_AIL_switch_stack"

// Returns the caller to the stack saved by AIL_switch_stack, given the saved
// `ss`, `esp` and `ebp`.
// FUNCTION: 0x000de743

.section _TEXT,"ax",@progbits,unique,911171
.globl "_AIL_restore_stack"
.type "_AIL_restore_stack",@function
"_AIL_restore_stack":
    push ebp
    {load} mov ebp, esp
    mov ecx, dword ptr [esp + 4]
    mov eax, dword ptr [ebp + 8]
    mov edx, dword ptr [ebp + 0xc]
    mov ebp, dword ptr [ebp + 0x10]
    add edx, 0x18
    mov ss, eax
    {load} mov esp, edx
    sub esp, 0xc
    jmp ecx
.size "_AIL_restore_stack",.-"_AIL_restore_stack"

// Calls driver `drvr` through real-mode INT 66h with AX = `fn` and BX = its
// driver number. CX, DX, SI and DI come from `in` (zero if NULL); if `out` is
// not NULL it receives AX, BX, CX, DX, SI and DI afterwards. Runs with AIL_lock
// held. Returns AX, sign-extended.
// FUNCTION: 0x000de75f

.section _TEXT,"ax",@progbits,unique,911199
.globl "_AIL_API_call_driver"
.type "_AIL_API_call_driver",@function
"_AIL_API_call_driver":
    push ebp
    {load} mov ebp, esp
    add esp, -0x3c
    push ebx
    push esi
    push edi
    push ds
    push es
    call "_AIL_lock"
    cld
    mov ecx, 0x32
    push ds
    pop es
    lea edi, [ebp - 0x3a]
    mov eax, 0
    push ecx
    and ecx, 3
    rep stosb byte ptr es:[edi], al
    pop ecx
    shr ecx, 2
    rep stosd dword ptr es:[edi], eax
    mov word ptr [ebp - 0x16], 0
    mov word ptr [ebp - 0x18], 0
    .byte 0x66; .byte 0x9c  # pushf
    pop ax
    mov word ptr [ebp - 0x1a], ax
    mov eax, dword ptr [ebp + 0xc]
    mov word ptr [ebp - 0x1e], ax
    mov ebx, dword ptr [ebp + 0x10]
    cmp ebx, 0
    je .L000de7ce
    mov cx, word ptr [ebx + 4]
    mov dx, word ptr [ebx + 6]
    mov si, word ptr [ebx + 8]
    mov di, word ptr [ebx + 0xa]
    mov word ptr [ebp - 0x22], cx
    mov word ptr [ebp - 0x26], dx
    mov word ptr [ebp - 0x36], si
    mov word ptr [ebp - 0x3a], di
.L000de7ce:
    mov ebx, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebx + 8]
    movsx ebx, word ptr [ebx + 0x32]
    mov word ptr [ebp - 0x2a], bx
    mov eax, 0x300
    mov ebx, 0x66
    mov ecx, 0
    lea edi, [ebp - 0x3a]
    int 0x31
    mov ebx, dword ptr [ebp + 0x14]
    cmp ebx, 0
    je .L000de827
    mov ax, word ptr [ebp - 0x1e]
    mov cx, word ptr [ebp - 0x22]
    mov dx, word ptr [ebp - 0x26]
    mov si, word ptr [ebp - 0x36]
    mov di, word ptr [ebp - 0x3a]
    mov word ptr [ebx], ax
    mov word ptr [ebx + 4], cx
    mov word ptr [ebx + 6], dx
    mov word ptr [ebx + 8], si
    mov word ptr [ebx + 0xa], di
    mov ax, word ptr [ebp - 0x2a]
    mov word ptr [ebx + 2], ax
.L000de827:
    movsx eax, word ptr [ebp - 0x1e]
    push eax
    call "_AIL_unlock"
    pop eax
    pop es
    pop ds
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AIL_API_call_driver",.-"_AIL_API_call_driver"

// Returns nonzero while called from inside the timer interrupt.
// FUNCTION: 0x000de839

.section _TEXT,"ax",@progbits,unique,911417
.globl "_AIL_API_background"
.type "_AIL_API_background",@function
"_AIL_API_background":
    mov eax, dword ptr [_D_001509f6]
    ret
.size "_AIL_API_background",.-"_AIL_API_background"

// Allocates one of the 15 user timers, stopped, with `callback` as its
// callback. Returns the handle (the timer's byte offset into the timer tables),
// or -1 if all are in use.
// FUNCTION: 0x000de83f

.section _TEXT,"ax",@progbits,unique,911423
.globl "_AIL_API_register_timer"
.type "_AIL_API_register_timer",@function
"_AIL_API_register_timer":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov eax, 0
.L000de84c:
    cmp dword ptr [eax + _D_0015088c], 0
    je .L000de864
    add eax, 4
    cmp eax, 0x3c
    jb .L000de84c
    mov eax, 0xffffffff
    jmp .L000de877
.L000de864:
    mov dword ptr [eax + _D_0015088c], 1
    mov ecx, dword ptr [ebp + 8]
    mov dword ptr [eax + _D_0015084c], ecx
.L000de877:
    push eax
    call "_AIL_unlock"
    pop eax
    leave
    ret
.size "_AIL_API_register_timer",.-"_AIL_API_register_timer"

// Sets the value passed to timer `timer`'s callback to `user`; returns the
// previous value. Handle -1 is ignored.
// FUNCTION: 0x000de880

.section _TEXT,"ax",@progbits,unique,911488
.globl "_AIL_API_set_timer_user"
.type "_AIL_API_set_timer_user",@function
"_AIL_API_set_timer_user":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov ecx, dword ptr [ebp + 8]
    cmp ecx, -1
    je .L000de899
    mov eax, dword ptr [ebp + 0xc]
    xchg dword ptr [ecx + _D_0015098c], eax
.L000de899:
    push eax
    call "_AIL_unlock"
    pop eax
    leave
    ret
.size "_AIL_API_set_timer_user",.-"_AIL_API_set_timer_user"

// Frees timer `timer`; -1 is ignored. The PIT rate is not recomputed.
// FUNCTION: 0x000de8a2

.section _TEXT,"ax",@progbits,unique,911522
.globl "_AIL_API_release_timer_handle"
.type "_AIL_API_release_timer_handle",@function
"_AIL_API_release_timer_handle":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov eax, dword ptr [ebp + 8]
    cmp eax, -1
    je .L000de8bc
    mov dword ptr [eax + _D_0015088c], 0
.L000de8bc:
    call "_AIL_unlock"
    leave
    ret
.size "_AIL_API_release_timer_handle",.-"_AIL_API_release_timer_handle"

// Frees the 15 user timers.
// FUNCTION: 0x000de8c3

.section _TEXT,"ax",@progbits,unique,911555
.globl "_AIL_API_release_all_timers"
.type "_AIL_API_release_all_timers",@function
"_AIL_API_release_all_timers":
    push esi
    call "_AIL_lock"
    mov esi, 0x38
.L000de8ce:
    push esi
    call "_AIL_release_timer_handle"
    add esp, 4
    sub esi, 4
    jge .L000de8ce
    call "_AIL_unlock"
    pop esi
    ret
.size "_AIL_API_release_all_timers",.-"_AIL_API_release_all_timers"

// Starts timer `timer` if it is allocated and stopped; -1 is ignored.
// FUNCTION: 0x000de8e3

.section _TEXT,"ax",@progbits,unique,911587
.globl "_AIL_API_start_timer"
.type "_AIL_API_start_timer",@function
"_AIL_API_start_timer":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov ecx, dword ptr [ebp + 8]
    cmp ecx, -1
    je .L000de906
    cmp dword ptr [ecx + _D_0015088c], 1
    jne .L000de906
    mov dword ptr [ecx + _D_0015088c], 2
.L000de906:
    call "_AIL_unlock"
    leave
    ret
.size "_AIL_API_start_timer",.-"_AIL_API_start_timer"

// Starts every allocated, stopped user timer.
// FUNCTION: 0x000de90d

.section _TEXT,"ax",@progbits,unique,911629
.globl "_AIL_API_start_all_timers"
.type "_AIL_API_start_all_timers",@function
"_AIL_API_start_all_timers":
    push esi
    call "_AIL_lock"
    mov esi, 0x38
.L000de918:
    push esi
    call "_AIL_start_timer"
    add esp, 4
    sub esi, 4
    jge .L000de918
    call "_AIL_unlock"
    pop esi
    ret
.size "_AIL_API_start_all_timers",.-"_AIL_API_start_all_timers"

// Stops timer `timer` if it is running; -1 is ignored.
// FUNCTION: 0x000de92d

.section _TEXT,"ax",@progbits,unique,911661
.globl "_AIL_API_stop_timer"
.type "_AIL_API_stop_timer",@function
"_AIL_API_stop_timer":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov ecx, dword ptr [ebp + 8]
    cmp ecx, -1
    je .L000de950
    cmp dword ptr [ecx + _D_0015088c], 2
    jne .L000de950
    mov dword ptr [ecx + _D_0015088c], 1
.L000de950:
    call "_AIL_unlock"
    leave
    ret
.size "_AIL_API_stop_timer",.-"_AIL_API_stop_timer"

// Stops every running user timer.
// FUNCTION: 0x000de957

.section _TEXT,"ax",@progbits,unique,911703
.globl "_AIL_API_stop_all_timers"
.type "_AIL_API_stop_all_timers",@function
"_AIL_API_stop_all_timers":
    push esi
    call "_AIL_lock"
    mov esi, 0x38
.L000de962:
    push esi
    call "_AIL_stop_timer"
    add esp, 4
    sub esi, 4
    jge .L000de962
    call "_AIL_unlock"
    pop esi
    ret
.size "_AIL_API_stop_all_timers",.-"_AIL_API_stop_all_timers"

// Sets timer `timer`'s period to `microseconds`, restarting its count, and
// reprograms the PIT for the shortest period in use.
// FUNCTION: 0x000de977

.section _TEXT,"ax",@progbits,unique,911735
.globl "_AIL_API_set_timer_period"
.type "_AIL_API_set_timer_period",@function
"_AIL_API_set_timer_period":
    push ebp
    {load} mov ebp, esp
    push ebx
    push esi
    push edi
    call "_AIL_lock"
    mov ecx, dword ptr [ebp + 8]
    mov eax, dword ptr [ebp + 0xc]
    mov dword ptr [ecx + _D_0015090c], eax
    mov dword ptr [ecx + _D_001508cc], 0
    call "_program_timers"
    call "_AIL_unlock"
    pop edi
    pop esi
    pop ebx
    leave
    ret
.size "_AIL_API_set_timer_period",.-"_AIL_API_set_timer_period"

// Sets timer `timer` to fire `hertz` times a second (period 1000000 / `hertz`
// microseconds).
// FUNCTION: 0x000de9a7

.section _TEXT,"ax",@progbits,unique,911783
.globl "_AIL_API_set_timer_frequency"
.type "_AIL_API_set_timer_frequency",@function
"_AIL_API_set_timer_frequency":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    mov edx, 0
    mov eax, 0xf4240
    mov ecx, dword ptr [ebp + 0xc]
    div ecx
    push eax
    push dword ptr [ebp + 8]
    call "_AIL_set_timer_period"
    add esp, 8
    call "_AIL_unlock"
    leave
    ret
.size "_AIL_API_set_timer_frequency",.-"_AIL_API_set_timer_frequency"

// Sets timer `timer`'s period to that of PIT divisor `PIT_divisor` (0 gives the
// BIOS rate, 54925 microseconds).
// FUNCTION: 0x000de9d1

.section _TEXT,"ax",@progbits,unique,911825
.globl "_AIL_API_set_timer_divisor"
.type "_AIL_API_set_timer_divisor",@function
"_AIL_API_set_timer_divisor":
    push ebp
    {load} mov ebp, esp
    call "_AIL_lock"
    cmp dword ptr [ebp + 0xc], 0
    jne .L000de9e6
    mov eax, 0xd68d
    jmp .L000de9fa
.L000de9e6:
    mov eax, 0x2710
    mov edx, 0
    mov ecx, 0x2e9c
    mul dword ptr [ebp + 0xc]
    div ecx
.L000de9fa:
    push eax
    push dword ptr [ebp + 8]
    call "_AIL_set_timer_period"
    add esp, 8
    call "_AIL_unlock"
    leave
    ret
.size "_AIL_API_set_timer_divisor",.-"_AIL_API_set_timer_divisor"

// Returns the divisor the PIT is currently programmed with (0 = 65536).
// FUNCTION: 0x000dea0d

.section _TEXT,"ax",@progbits,unique,911885
.globl "_AIL_API_interrupt_divisor"
.type "_AIL_API_interrupt_divisor",@function
"_AIL_API_interrupt_divisor":
    mov eax, dword ptr [_D_001509ee]
    ret
.size "_AIL_API_interrupt_divisor",.-"_AIL_API_interrupt_divisor"

// Opens file `filename` with DOS access mode `access` (0 read, 1 write, 2
// read/write). Returns the DOS handle, or -1 on failure.
// FUNCTION: 0x000dea13

.section _TEXT,"ax",@progbits,unique,911891
.globl "_AIL_fopen"
.type "_AIL_fopen",@function
"_AIL_fopen":
    push ebp
    {load} mov ebp, esp
    push ebx
    push edx
    mov ah, 0x3d
    mov al, byte ptr [ebp + 0xc]
    mov edx, dword ptr [ebp + 8]
    int 0x21
    jae .L000dea28
    mov ax, 0xffff
.L000dea28:
    movsx eax, ax
    pop edx
    pop ebx
    pop ebp
    ret
.size "_AIL_fopen",.-"_AIL_fopen"

// Creates (or truncates) file `filename` with normal attributes. Returns the
// DOS handle, or -1 on failure.
// FUNCTION: 0x000dea2f

.section _TEXT,"ax",@progbits,unique,911919
.globl "_AIL_fcreate"
.type "_AIL_fcreate",@function
"_AIL_fcreate":
    push ebp
    {load} mov ebp, esp
    push ebx
    push edx
    mov ah, 0x3c
    mov edx, dword ptr [ebp + 8]
    {load} xor ecx, ecx
    int 0x21
    jae .L000dea43
    mov ax, 0xffff
.L000dea43:
    movsx eax, ax
    pop edx
    pop ebx
    pop ebp
    ret
.size "_AIL_fcreate",.-"_AIL_fcreate"

// Opens file `filename` for writing positioned at its end, creating it if it
// does not exist. Returns the DOS handle, or -1 on failure.
// FUNCTION: 0x000dea4a

.section _TEXT,"ax",@progbits,unique,911946
.globl "_AIL_fappend"
.type "_AIL_fappend",@function
"_AIL_fappend":
    push ebp
    {load} mov ebp, esp
    push ebx
    push edx
    push ecx
    mov ax, 0x3d01
    mov edx, dword ptr [ebp + 8]
    int 0x21
    jae .L000dea75
    cmp ax, 2
    je .L000dea68
.L000dea61:
    mov eax, 0xffffffff
    jmp .L000dea84
.L000dea68:
    mov ah, 0x3c
    {load} xor ecx, ecx
    int 0x21
    jb .L000dea61
    movsx eax, ax
    jmp .L000dea84
.L000dea75:
    movsx ebx, ax
    {load} xor edx, edx
    {load} xor ecx, ecx
    mov ax, 0x4202
    int 0x21
    {load} mov eax, ebx
.L000dea84:
    pop ecx
    pop edx
    pop ebx
    pop ebp
    ret
.size "_AIL_fappend",.-"_AIL_fappend"

// Reads up to `bytes` bytes from `handle` into `buffer` (in 0xF000-byte pieces
// when over 64K), stopping at an error, end of file or a short read. Returns
// the number of bytes read.
// FUNCTION: 0x000dea89

.section _TEXT,"ax",@progbits,unique,912009
.globl "_AIL_fread"
.type "_AIL_fread",@function
"_AIL_fread":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    mov edx, dword ptr [ebp + 0xc]
.L000dea92:
    mov ah, 0x3f
    mov ecx, dword ptr [ebp + 0x10]
    cmp ecx, 0xffff
    jbe .L000deaa4
    mov ecx, 0xf000
.L000deaa4:
    mov ebx, dword ptr [ebp + 8]
    push ecx
    push edx
    int 0x21
    pop edx
    pop ecx
    jb .L000deac1
    and eax, 0xffff
    je .L000deac1
    {load} add edx, eax
    {load} cmp eax, ecx
    jb .L000deac1
    sub dword ptr [ebp + 0x10], eax
    jne .L000dea92
.L000deac1:
    sub edx, dword ptr [ebp + 0xc]
    {load} mov eax, edx
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret
.size "_AIL_fread",.-"_AIL_fread"

// Writes `bytes` bytes from `buffer` to `handle` (in 0xF000-byte pieces when
// over 64K), stopping at an error or a short write. Returns the number of bytes
// written.
// FUNCTION: 0x000deacb

.section _TEXT,"ax",@progbits,unique,912075
.globl "_AIL_fwrite"
.type "_AIL_fwrite",@function
"_AIL_fwrite":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    mov edx, dword ptr [ebp + 0xc]
.L000dead4:
    mov ah, 0x40
    mov ecx, dword ptr [ebp + 0x10]
    cmp ecx, 0xffff
    jbe .L000deae6
    mov ecx, 0xf000
.L000deae6:
    mov ebx, dword ptr [ebp + 8]
    push ecx
    push edx
    int 0x21
    pop edx
    pop ecx
    jb .L000deb03
    and eax, 0xffff
    je .L000deb03
    {load} add edx, eax
    {load} cmp eax, ecx
    jb .L000deb03
    sub dword ptr [ebp + 0x10], eax
    jne .L000dead4
.L000deb03:
    sub edx, dword ptr [ebp + 0xc]
    {load} mov eax, edx
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret
.size "_AIL_fwrite",.-"_AIL_fwrite"

// Moves `handle`'s file pointer to `offset` from the start; returns the new
// position. Falls through into ail_dos_seek.
// FUNCTION: 0x000deb0d

.section _TEXT,"ax",@progbits,unique,912141
.globl "_AIL_fseekbegin"
.type "_AIL_fseekbegin",@function
"_AIL_fseekbegin":
    mov ax, 0x4200
.size "_AIL_fseekbegin",.-"_AIL_fseekbegin"

// Shared body of the AIL_fseek functions, entered with the DOS seek function
// (AX = 4200h, 4201h or 4202h) already in AX: seeks `handle` by `offset` and
// returns the new position.
// FUNCTION: 0x000deb11

.section _TEXT,"ax",@progbits,unique,912145
.globl "ail_dos_seek"
.type "ail_dos_seek",@function
"ail_dos_seek":
    push ebp
    {load} mov ebp, esp
    push ebx
    push ecx
    push edx
    mov ebx, dword ptr [ebp + 8]
    movzx edx, word ptr [ebp + 0xc]
    movzx ecx, word ptr [ebp + 0xe]
    int 0x21
    shl edx, 0x10
    {load} mov dx, ax
    {load} mov eax, edx
    pop ecx
    pop edx
    pop ebx
    pop ebp
    ret
.size "ail_dos_seek",.-"ail_dos_seek"

// Moves `handle`'s file pointer by `offset` from its current position; returns
// the new position.
// FUNCTION: 0x000deb31

.section _TEXT,"ax",@progbits,unique,912177
.globl "_AIL_fseekcur"
.type "_AIL_fseekcur",@function
"_AIL_fseekcur":
    mov ax, 0x4201
    .byte 0xeb; .byte 0xda  # jmp 0xdeb11
.size "_AIL_fseekcur",.-"_AIL_fseekcur"

// Moves `handle`'s file pointer to `offset` from the end; returns the new
// position (the file size for offset 0).
// FUNCTION: 0x000deb37

.section _TEXT,"ax",@progbits,unique,912183
.globl "_AIL_fseekend"
.type "_AIL_fseekend",@function
"_AIL_fseekend":
    mov ax, 0x4202
    .byte 0xeb; .byte 0xd4  # jmp 0xdeb11
.size "_AIL_fseekend",.-"_AIL_fseekend"

// Closes DOS file `handle`. The Miles copyright string follows its code.
// FUNCTION: 0x000deb3d

.section _TEXT,"ax",@progbits,unique,912189
.globl "_AIL_fclose"
.type "_AIL_fclose",@function
"_AIL_fclose":
    push ebp
    {load} mov ebp, esp
    push ebx
    mov ah, 0x3e
    mov ebx, dword ptr [ebp + 8]
    int 0x21
    pop ebx
    pop ebp
    ret
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a; .byte 0x4d
    .byte 0x69; .byte 0x6c; .byte 0x65; .byte 0x73; .byte 0x20; .byte 0x53; .byte 0x6f; .byte 0x75
    .byte 0x6e
    .byte 0x64; .byte 0x20; .byte 0x53; .byte 0x79
    .byte 0x73; .byte 0x74
    .byte 0x65; .byte 0x6d
    .byte 0x0d; .byte 0x0a; .byte 0x43; .byte 0x6f; .byte 0x70
    .byte 0x79; .byte 0x72
    .byte 0x69; .byte 0x67; .byte 0x68; .byte 0x74; .byte 0x20; .byte 0x28; .byte 0x43
    .byte 0x29; .byte 0x20
    .byte 0x31; .byte 0x39
    .byte 0x39; .byte 0x31
    .byte 0x2d; .byte 0x39; .byte 0x36; .byte 0x20; .byte 0x52
    .byte 0x41
    .byte 0x44
    .byte 0x20; .byte 0x47; .byte 0x61
    .byte 0x6d
    .byte 0x65; .byte 0x20; .byte 0x54; .byte 0x6f; .byte 0x6f
    .byte 0x6c
    .byte 0x73; .byte 0x2c
    .byte 0x20; .byte 0x49; .byte 0x6e
    .byte 0x63; .byte 0x2e
    .byte 0x0d; .byte 0x0a; .byte 0x0d; .byte 0x0a
.size "_AIL_fclose",.-"_AIL_fclose"

// Locks this module's data (the timer tables up to AIL_driver_directory) and
// code (AIL_API_lock to here) against paging.
// FUNCTION: 0x000deb91

.section _TEXT,"ax",@progbits,unique,912273
.globl "_AILA_VMM_lock"
.type "_AILA_VMM_lock",@function
"_AILA_VMM_lock":
    .byte 0x68; .long "_AIL_driver_directory"  # push 0x31418
    .byte 0x68; .long _D_0015084c  # push 0x3084c
    call "_AIL_vmm_lock_range"
    add esp, 8
    .byte 0x68; .long "_AILA_VMM_lock"  # push 0xceb91
    .byte 0x68; .long "_AIL_API_lock"  # push 0xcdf88
    call "_AIL_vmm_lock_range"
    add esp, 8
    ret
.size "_AILA_VMM_lock",.-"_AILA_VMM_lock"
