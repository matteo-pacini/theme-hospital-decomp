typedef unsigned long u32;

u32 mult64anddiv(u32 m1, u32 m2, u32 d);
#pragma aux mult64anddiv = \
    "mul ecx" \
    "div ebx" \
    parm [eax] [ecx] [ebx] \
    value [eax] \
    modify exact [eax edx];

void setpit(unsigned short divisor);
#pragma aux setpit = \
    "mov al,34h" \
    "out 43h,al" \
    0xeb 0x00 \
    "mov al,bl" \
    "out 40h,al" \
    0xeb 0x00 \
    "mov al,bh" \
    "out 40h,al" \
    parm [bx] \
    modify exact [eax];

extern u32 (__pascal *SmackTimerReadAddr)(void);
extern void (__pascal *SmackTimerDoneAddr)(void);
extern u32 D_00151b90;
extern u32 D_00151b94;
extern u32 (__far *D_0018d666)(void);

void vtdentry(void);
#pragma aux vtdentry = \
    "push es" \
    "mov ax,1684h" \
    "mov bx,5" \
    "xor di,di" \
    "mov es,di" \
    "int 2fh" \
    "mov word ptr D_0018d666,di" \
    "mov word ptr D_0018d666+2,0" \
    "mov word ptr D_0018d666+4,es" \
    "pop es" \
    modify [eax ebx edi];

u32 vtdread(void);
#pragma aux vtdread = \
    "mov eax,101h" \
    "call fword ptr D_0018d666" \
    value [eax];

void __pascal DOSSmackTimerDone(void);
#pragma aux (__pascal) DOSSmackTimerDone "fn_000f2f70_";
u32 __pascal VTDSmackTimerRead(void);
#pragma aux (__pascal) VTDSmackTimerRead "fn_000f2f80_";
u32 __pascal BIOSSmackTimerRead(void);
#pragma aux (__pascal) BIOSSmackTimerRead "fn_000f2fa0_";
void __pascal DOSSmackTimerSetup(void);
#pragma aux (__pascal) DOSSmackTimerSetup "fn_000f2ec0_";

// FUNCTION: 0xf2ec0
void __pascal DOSSmackTimerSetup(void)
{
    if (D_00151b90 == 0) {
        SmackTimerDoneAddr = DOSSmackTimerDone;
        vtdentry();
        if (D_0018d666 != 0) {
            SmackTimerReadAddr = VTDSmackTimerRead;
            D_00151b90 = 1;
            return;
        }
        D_00151b90 = *(u32 *)0x46c;
        SmackTimerReadAddr = BIOSSmackTimerRead;
        D_00151b90 = -D_00151b90;
        setpit(0);
    }
}

// FUNCTION: 0xf2f70
void __pascal DOSSmackTimerDone(void)
{
}

// FUNCTION: 0xf2f80
u32 __pascal VTDSmackTimerRead(void)
{
    u32 t;

    t = vtdread();
    if (t < D_00151b94)
        return D_00151b94;
    D_00151b94 = t;
    return t;
}

// FUNCTION: 0xf2fa0
u32 __pascal BIOSSmackTimerRead(void)
{
    u32 t;

    t = *(u32 *)0x46c;
    if (t < D_00151b94)
        D_00151b90 += D_00151b94 - t;
    D_00151b94 = t;
    return mult64anddiv(t + D_00151b90, 2746, 50);
}
