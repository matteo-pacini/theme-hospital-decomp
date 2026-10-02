#include "clib.h"

const unsigned char _Bits[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

void *memset(void *s, int c, size_t n);

// __setbits: build a 256-bit membership vector from the characters of charset (scanf's %[ sets).
// FUNCTION: 0xeb769
void __setbits(unsigned char *vector, const char *charset)
{
    unsigned char c;

    memset(vector, 0, 32);
    for (; (c = *charset) != '\0'; ++charset)
        vector[(int)c >> 3] |= _Bits[c & 7];
}
