#include "clib.h"

void *memcpy(void *dst, const void *src, size_t n);
#pragma intrinsic(memcpy)

void *clib_memcpy(void *dst, const void *src, size_t n);
#pragma aux clib_memcpy "memcpy_"

// FUNCTION: 0xc21e6
void *clib_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}
