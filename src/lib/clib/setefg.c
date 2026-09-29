#include "clib.h"

typedef char __far *FAR_STRING;

extern FAR_STRING (*__EFG_printf)();
extern void (*__EFG_scanf)();

FAR_STRING _EFG_Format();
void __cnvs2d();

void __setEFGfmt(void);

// FUNCTION: 0xef08c
void __setEFGfmt(void)
{
    __EFG_printf = _EFG_Format;
    __EFG_scanf = __cnvs2d;
}
