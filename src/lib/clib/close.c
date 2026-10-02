#include "clib.h"

int __close(int handle);

// close: close file handle; returns 0, or -1 with errno set.
// FUNCTION: 0xd0c61
int close(int handle)
{
    return __close(handle);
}
