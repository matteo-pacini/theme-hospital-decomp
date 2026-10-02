#include "clib.h"

// strncat: append at most n chars of src to dst, always NUL-terminated; returns dst.
// FUNCTION: 0xce4ed
char *strncat(char *dst, const char *src, size_t n)
{
    char *s;

    s = memchr(dst, '\0', ~0);
    while (n != 0) {
        *s = *src;
        if (*s == '\0')
            break;
        ++s;
        ++src;
        --n;
    }
    *s = '\0';
    return dst;
}
