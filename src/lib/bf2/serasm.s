.intel_syntax noprefix

// The serial port interrupt handler dosser copies into DOS memory, with
// com_port set to the port's base I/O address. Each received byte goes into
// a 0x800-byte ring buffer at 0x896 whose write index is at 0x109E. It runs
// from the segment it is copied to, so its own data is addressed relative to
// ser_start_ and needs no fixup.

.code16
.section CODE16,"ax",@progbits
.globl ser_start_
.globl com_port
.globl sdebugColors
.globl ser_end_
ser_start_:
    jmp 1f
com_port:
    .word 0x3f8
1:
    push bx
    push cx
    push es
    push ds
    push si
    push di
    push ax
    push dx
    mov bx, cs
    mov ds, bx
    mov dx, cs:[com_port - ser_start_]
    add dx, 2
    in al, dx
    {load} mov cl, al
    and al, 7
    and al, 4
    cmp al, 0
    je 2f
    mov dx, cs:[com_port - ser_start_]
    add dx, 5
    in al, dx
    and al, 1
    cmp al, 0
    je 2f
    mov dx, cs:[com_port - ser_start_]
    in al, dx
    mov bx, 0x896
    add bx, cs:[0x109e]
    mov cs:[bx], al
    add word ptr cs:[0x109e], 1
    mov bx, cs:[0x109e]
    cmp bx, 0x800
    jl 2f
    mov word ptr cs:[0x109e], 0
2:
    and cl, 8
    cmp cl, 0
    je 3f
    mov dx, cs:[com_port - ser_start_]
    add dx, 5
    in al, dx
3:
    {load} xor ax, ax
    mov dx, 0x20
    {load} mov al, dl
    out dx, al
    pop dx
    pop ax
    pop di
    pop si
    pop ds
    pop es
    pop cx
    pop bx
    iret

// Steps the VGA overscan colour, for watching the handler run.
sdebugColors:
    push ax
    push dx
    mov dx, 0x3da
    in al, dx
    mov dx, 0x3c0
    mov al, 0x31
    out dx, al
    jmp 4f
4:
    inc dx
    in al, dx
    dec dx
    inc al
    and al, 0xf
    out dx, al
    pop dx
    pop ax
    ret
ser_end_:
