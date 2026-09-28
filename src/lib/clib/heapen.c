#include "clib.h"

extern int __heap_enabled;

// FUNCTION: 0xf46dd
int fn_000f46dd(int new)
{
    int old;

    old = __heap_enabled;
    __heap_enabled = new;
    return old;
}
