#include "clib.h"

extern const unsigned char _Bits[8];

void __setbits(unsigned char *vector, const char *charset);

// FUNCTION: 0xf4b1a
char *strpbrk(const char *str, const char *charset)
{
    char tc;
    unsigned char vector[32];

    __setbits(vector, charset);
    for (; tc = *str; ++str) {
        if (vector[(int)tc >> 3] & _Bits[tc & 7])
            return (char *)str;
    }
    return NULL;
}
