#include "clib.h"

void *memcpy(void *dst, const void *src, size_t n);
#pragma intrinsic(memcpy)

/* The runtime's memcpy; config/functions.txt names it LbDrawBoxCoords_. */
void *clib_memcpy(void *dst, const void *src, size_t n);
#pragma aux clib_memcpy "LbDrawBoxCoords_"

// FUNCTION: 0xc21e6
void *clib_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}
