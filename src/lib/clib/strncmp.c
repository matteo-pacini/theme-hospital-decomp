#include "clib.h"

// strncmp: compare at most n chars of s1 and s2; <0, 0 or >0.
// FUNCTION: 0xd00dc
int strncmp(const char *s1, const char *s2, size_t n)
{
    for (;;) {
        if (n == 0)
            return 0;
        if (*s1 != *s2)
            return (unsigned char)*s1 - (unsigned char)*s2;
        if (*s1 == '\0')
            return 0;
        ++s1;
        ++s2;
        --n;
    }
}
