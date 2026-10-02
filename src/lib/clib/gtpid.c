#include "clib.h"

extern int _psp;

int getpid(void);

// getpid: process id (the PSP selector).
// FUNCTION: 0xfbac5
int getpid(void)
{
    return _psp;
}
