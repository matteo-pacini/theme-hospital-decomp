extern char __WD_Present;

void EnterDebuggerWithMessage(const char __far *msg);
#pragma aux EnterDebuggerWithMessage = \
    0xcc 0xeb 0x06 0x57 0x56 0x49 0x44 0x45 0x4f \
    parm caller [] \
    modify [];

// __EnterWVIDEO: if the Watcom debugger is present, break into it showing msg and return 1; else return 0.
// FUNCTION: 0xf2e6b
int __EnterWVIDEO(const char __far *msg)
{
    if (__WD_Present) {
        EnterDebuggerWithMessage(msg);
        return 1;
    }
    return 0;
}
