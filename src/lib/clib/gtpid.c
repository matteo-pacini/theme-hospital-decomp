#include "clib.h"

extern int _psp;

int getpid(void);
/* config/functions.txt names getpid after the preceding default_handler. */
#pragma aux getpid "default_handler_"

// FUNCTION: 0xfbac5
int getpid(void)
{
    return _psp;
}
