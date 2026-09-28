#include "clib.h"

extern const unsigned char _Bits[8];

void *memset(void *s, int c, size_t n);

// FUNCTION: 0xeb769
void __setbits(unsigned char *vector, const char *charset)
{
    unsigned char c;

    memset(vector, 0, 32);
    for (; (c = *charset) != '\0'; ++charset)
        vector[(int)c >> 3] |= _Bits[c & 7];
}
