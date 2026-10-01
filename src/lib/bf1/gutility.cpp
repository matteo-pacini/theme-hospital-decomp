#include "bflib.h"

extern "C" {
void fast_memcpy(void *dst, void *src, u32 size);
#pragma aux fast_memcpy = 0x57 0x89 0xc8 0xc1 0xe9 0x02 0xf2 0xa5 0x8a 0xc8 0x80 0xe1 0x03 0xf2 0xa4 0x5f \
    parm [edi] [esi] [ecx] modify [eax ecx esi edi];

// Copies size bytes from src to dst (dwords, then the remaining bytes).
// FUNCTION: 0x000bc0d0
void LbMemoryCopy(void *dst, void *src, u32 size)
{
    fast_memcpy(dst, src, size);
}

// Fills size bytes at dst with value.
// FUNCTION: 0x000bc104
void LbMemorySet(void *dst, u8 value, u32 size)
{
    memset(dst, value, size);
}

}
