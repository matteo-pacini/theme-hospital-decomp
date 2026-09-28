#include "clib.h"

// FUNCTION: 0xce4ed
char *strncat(char *dst, const char *t, size_t n)
{
    char *s;

    s = memchr(dst, '\0', ~0);
    while (n != 0) {
        *s = *t;
        if (*s == '\0')
            break;
        ++s;
        ++t;
        --n;
    }
    *s = '\0';
    return dst;
}
