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

void __pascal DEFSmackTimerDone(void);
#pragma aux (__pascal) DEFSmackTimerDone "DEFSMACKTIMERDONE";
u32 __pascal DEFWINSmackTimerRead(void);
#pragma aux (__pascal) DEFWINSmackTimerRead "DEFWINSMACKTIMERREAD";
u32 __pascal DEFSmackTimerRead(void);
#pragma aux (__pascal) DEFSmackTimerRead "DEFSMACKTIMERREAD";
void __pascal DEFSmackTimerSetup(void);
#pragma aux (__pascal) DEFSmackTimerSetup "DEFSMACKTIMERSETUP";

u32 (__pascal *SmackTimerReadAddr)(void) = 0;
static u32 timeradjust = 0;
static u32 lasttimerread = 0;
void (__pascal *SmackTimerSetupAddr)(void) = DEFSmackTimerSetup;

// DEFSmackTimerRead: milliseconds since DEFSmackTimerSetup from the BIOS tick count at 0:46Ch (ticks * 2746 / 50), counted on from the last reading when the count resets at midnight.
// FUNCTION: 0xf2fa0
u32 __pascal DEFSmackTimerRead(void)
{
    u32 ticks;

    ticks = *(u32 *)0x46c;
    if (ticks < lasttimerread)
        timeradjust += lasttimerread - ticks;
    lasttimerread = ticks;
    return mult64anddiv(ticks + timeradjust, 2746, 50);
}

// DEFWINSmackTimerRead: milliseconds from the Windows virtual timer device (VTD); never goes backwards.
// FUNCTION: 0xf2f80
u32 __pascal DEFWINSmackTimerRead(void)
{
    u32 ticks;

    ticks = vtdread();
    if (ticks < lasttimerread)
        return lasttimerread;
    lasttimerread = ticks;
    return ticks;
}

// DEFSmackTimerDone: timer shutdown hook; nothing to undo.
// FUNCTION: 0xf2f70
void __pascal DEFSmackTimerDone(void)
{
}

// DEFSmackTimerSetup: on first use pick the timer: the Windows VTD if INT 2Fh 1684h finds it, else the BIOS tick count with the PIT reset to its standard rate.
// FUNCTION: 0xf2ec0
void __pascal DEFSmackTimerSetup(void)
{
    if (timeradjust == 0) {
        SmackTimerDoneAddr = DEFSmackTimerDone;
        vtdentry();
        if (snaddr != 0) {
            SmackTimerReadAddr = DEFWINSmackTimerRead;
            timeradjust = 1;
            return;
        }
        timeradjust = *(u32 *)0x46c;
        SmackTimerReadAddr = DEFSmackTimerRead;
        timeradjust = -timeradjust;
        setpit(0);
    }
}
