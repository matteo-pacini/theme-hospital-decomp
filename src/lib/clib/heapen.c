#include "clib.h"

int __heap_enabled = 1;

// FUNCTION: 0xf46dd
int _heapenable(int new)
{
    int old;

    old = __heap_enabled;
    __heap_enabled = new;
    return old;
}
