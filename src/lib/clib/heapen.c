#include "clib.h"

int __heap_enabled = 1;

// _heapenable: set whether the heap may grow (new); returns the previous setting.
// FUNCTION: 0xf46dd
int _heapenable(int new)
{
    int old;

    old = __heap_enabled;
    __heap_enabled = new;
    return old;
}
