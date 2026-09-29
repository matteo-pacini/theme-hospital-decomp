#include "clib.h"

typedef char __far *FAR_STRING;

extern FAR_STRING (*__EFG_printf)();
extern void (*__EFG_scanf)();

FAR_STRING _EFG_Format();
void __cnvs2d();
/* config/functions.txt has no name for __cnvs2d. */
#pragma aux __cnvs2d "fn_000f758e_"

void __setEFGfmt(void);
/* config/functions.txt has no name for __setEFGfmt. */
#pragma aux __setEFGfmt "fn_000ef08c_"

// FUNCTION: 0xef08c
void __setEFGfmt(void)
{
    __EFG_printf = _EFG_Format;
    __EFG_scanf = __cnvs2d;
}
