#include "clib.h"

extern int _psp;

int getpid(void);
/* config/functions.txt names getpid after the preceding default_handler. */

// FUNCTION: 0xfbac5
int getpid(void)
{
    return _psp;
}
