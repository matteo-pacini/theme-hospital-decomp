#include "clib.h"

extern int __heap_enabled;

// FUNCTION: 0xf46dd
int _heapenable(int new)
{
    int old;

    old = __heap_enabled;
    __heap_enabled = new;
    return old;
}
