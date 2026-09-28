#include "clib.h"

// FUNCTION: 0xd0ba8
int creat(const char *name, int pmode)
{
    return open(name, O_RDWR | O_CREAT | O_TRUNC, pmode);
}
