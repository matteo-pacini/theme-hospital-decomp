.intel_syntax noprefix

// The real-mode interrupt handler dosipx copies into DOS memory and installs
// on a free vector. Called with BX = 0xFFFF, it makes the AX IPX calls
// batched at 0x90, 6 bytes each (function for BX, ECB offset for SI, ECB
// segment for ES); otherwise it passes the call on unchanged. Either way it
// far-calls the IPX entry point dosipx stores in ipx_entry. It runs from a
// different segment than it was linked in, so only offsets below 0x1000 of
// its own are meaningful.

.code16
.section CODE16,"ax",@progbits
.globl ipx_start_
.globl ipx_entry
.globl idebugColors
.globl ipx_end_
ipx_start_:
    jmp 1f
ipx_entry:
    .long 0
1:
    push cs
    pop ds
    push bx
    push cx
    push es
    push ds
    push si
    push di
    push bp
    cmp bx, -1
    jne 3f
    lea bp, [0x90]
2:
    cmp ax, 0
    je 4f
    push bx
    push ax
    push bp
    mov bx, cs:[bp]
    mov si, cs:[bp + 2]
    mov es, cs:[bp + 4]
    mov di, offset ipx_entry
    and di, 0xfff
    call dword ptr [di]
    pop bp
    pop ax
    pop bx
    dec ax
    add bp, 6
    jmp 2b
3:
    mov di, offset ipx_entry
    and di, 0xfff
    call dword ptr [di]
4:
    pop bp
    pop di
    pop si
    pop ds
    pop es
    pop cx
    pop bx
    iret

// Steps the VGA overscan colour, for watching the handler run.
idebugColors:
    push ax
    push dx
    mov dx, 0x3da
    in al, dx
    mov dx, 0x3c0
    mov al, 0x31
    out dx, al
    jmp 5f
5:
    inc dx
    in al, dx
    dec dx
    inc al
    and al, 0xf
    out dx, al
    pop dx
    pop ax
    ret
ipx_end_:
