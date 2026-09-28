#include "clib.h"

int __close(int handle);

// FUNCTION: 0xd0c61
int close(int handle)
{
    return __close(handle);
}
