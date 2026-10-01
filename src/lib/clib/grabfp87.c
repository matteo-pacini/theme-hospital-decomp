#include "clib.h"

extern void (*__FPE_handler_exit)(void);
extern void (*__FPE_handler)(int fpe_type);
#pragma aux __FPE_handler "__FPE_handler"
void __sigfpe_handler(int fpe_type);
/* __Init_FPE_handler; __Fini_FPE_handler is an entry 0x5d bytes into it. */
void __Init_FPE_handler(void);
void __Enable_FPE(void);

void __GrabFP87(void);
#pragma aux __GrabFP87 "__GrabFP87"

// FUNCTION: 0xfcefe
void __GrabFP87(void)
{
    if (__FPE_handler_exit != (void (*)(void))((char *)__Init_FPE_handler + 0x5d)) {
        __FPE_handler = __sigfpe_handler;
        __Init_FPE_handler();
        __FPE_handler_exit = (void (*)(void))((char *)__Init_FPE_handler + 0x5d);
    }
    __Enable_FPE();
}
