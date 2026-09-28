#include "clib.h"

extern int _psp;

int getpid(void);

// FUNCTION: 0xfbac5
int getpid(void)
{
    return _psp;
}
