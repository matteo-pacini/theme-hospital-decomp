#include "clib.h"

extern void (*__FPE_handler_exit)(void);
extern void (*__FPE_handler)(int fpe_type);
#pragma aux __FPE_handler "__FPE_handler"
void __sigfpe_handler(int fpe_type);
/* __Init_FPE_handler; __Fini_FPE_handler is an entry 0x5d bytes into it. */
void fn_000fd8d8(void);
void fn_000fd992(void);

void __GrabFP87(void);
#pragma aux __GrabFP87 "__GrabFP87"

// FUNCTION: 0xfcefe
void __GrabFP87(void)
{
    if (__FPE_handler_exit != (void (*)(void))((char *)fn_000fd8d8 + 0x5d)) {
        __FPE_handler = __sigfpe_handler;
        fn_000fd8d8();
        __FPE_handler_exit = (void (*)(void))((char *)fn_000fd8d8 + 0x5d);
    }
    fn_000fd992();
}
