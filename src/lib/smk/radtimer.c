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

extern void (__pascal *SmackTimerDoneAddr)(void);
extern u32 (__far *snaddr)(void);

void vtdentry(void);
#pragma aux vtdentry = \
    "push es" \
    "mov ax,1684h" \
    "mov bx,5" \
    "xor di,di" \
    "mov es,di" \
    "int 2fh" \
    "mov word ptr snaddr,di" \
    "mov word ptr snaddr+2,0" \
    "mov word ptr snaddr+4,es" \
    "pop es" \
    modify [eax ebx edi];

u32 vtdread(void);
#pragma aux vtdread = \
    "mov eax,101h" \
    "call fword ptr snaddr" \
    value [eax];

void __pascal DOSSmackTimerDone(void);
#pragma aux (__pascal) DOSSmackTimerDone "DOSSMACKTIMERDONE";
u32 __pascal VTDSmackTimerRead(void);
#pragma aux (__pascal) VTDSmackTimerRead "VTDSMACKTIMERREAD";
u32 __pascal BIOSSmackTimerRead(void);
#pragma aux (__pascal) BIOSSmackTimerRead "BIOSSMACKTIMERREAD";
void __pascal DOSSmackTimerSetup(void);
#pragma aux (__pascal) DOSSmackTimerSetup "DOSSMACKTIMERSETUP";

u32 (__pascal *SmackTimerReadAddr)(void) = 0;
static u32 timeradjust = 0;
static u32 lasttimerread = 0;
void (__pascal *SmackTimerSetupAddr)(void) = DOSSmackTimerSetup;

// BIOSSmackTimerRead: milliseconds from the BIOS tick count at 0:46Ch (ticks * 2746 / 50), kept increasing across its midnight reset.
// FUNCTION: 0xf2fa0
u32 __pascal BIOSSmackTimerRead(void)
{
    u32 ticks;

    ticks = *(u32 *)0x46c;
    if (ticks < lasttimerread)
        timeradjust += lasttimerread - ticks;
    lasttimerread = ticks;
    return mult64anddiv(ticks + timeradjust, 2746, 50);
}

// VTDSmackTimerRead: milliseconds from the Windows virtual timer device (VTD); never goes backwards.
// FUNCTION: 0xf2f80
u32 __pascal VTDSmackTimerRead(void)
{
    u32 ticks;

    ticks = vtdread();
    if (ticks < lasttimerread)
        return lasttimerread;
    lasttimerread = ticks;
    return ticks;
}

// DOSSmackTimerDone: timer shutdown hook; nothing to undo.
// FUNCTION: 0xf2f70
void __pascal DOSSmackTimerDone(void)
{
}

// DOSSmackTimerSetup: on first use pick the timer: the Windows VTD if INT 2Fh 1684h finds it, else the BIOS tick count with the PIT reset to its standard rate.
// FUNCTION: 0xf2ec0
void __pascal DOSSmackTimerSetup(void)
{
    if (timeradjust == 0) {
        SmackTimerDoneAddr = DOSSmackTimerDone;
        vtdentry();
        if (snaddr != 0) {
            SmackTimerReadAddr = VTDSmackTimerRead;
            timeradjust = 1;
            return;
        }
        timeradjust = *(u32 *)0x46c;
        SmackTimerReadAddr = BIOSSmackTimerRead;
        timeradjust = -timeradjust;
        setpit(0);
    }
}
