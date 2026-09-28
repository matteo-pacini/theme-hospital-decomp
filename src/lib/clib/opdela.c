#include "clib.h"

/* ::operator delete(void *), called from C through its C++ symbol. */
void __op_delete(void *p);
#pragma aux __op_delete "W?$dln(pnv)v"

// FUNCTION: 0xed344
void fn_000ed344(void *p)
{
    __op_delete(p);
}
