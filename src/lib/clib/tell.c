#include "clib.h"

// FUNCTION: 0xd0b97
long tell(int handle)
{
    return lseek(handle, 0L, SEEK_CUR);
}
