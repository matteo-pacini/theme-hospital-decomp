#include "clib.h"

// FUNCTION: 0xd00dc
int strncmp(const char *s, const char *t, size_t n)
{
    for (;;) {
        if (n == 0)
            return 0;
        if (*s != *t)
            return (unsigned char)*s - (unsigned char)*t;
        if (*s == '\0')
            return 0;
        ++s;
        ++t;
        --n;
    }
}
