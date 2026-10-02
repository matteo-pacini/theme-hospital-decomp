#include "clib.h"

// tell: current file position of handle, or -1.
// FUNCTION: 0xd0b97
long tell(int handle)
{
    return lseek(handle, 0L, SEEK_CUR);
}
