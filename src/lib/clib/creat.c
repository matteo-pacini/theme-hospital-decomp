#include "clib.h"

// creat: create or truncate file name for read/write with permission pmode; returns handle or -1.
// FUNCTION: 0xd0ba8
int creat(const char *name, int pmode)
{
    return open(name, O_RDWR | O_CREAT | O_TRUNC, pmode);
}
