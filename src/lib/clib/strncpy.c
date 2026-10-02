#include "clib.h"

// strncpy: copy at most n chars of src to dst, padding with NULs to n; returns dst.
// FUNCTION: 0xce414
char *strncpy(char *dst, const char *src, size_t n)
{
    char *ret;

    ret = dst;
    for (; n; --n) {
        if (*src == '\0')
            break;
        *dst++ = *src++;
    }
    while (n != 0) {
        *dst++ = '\0';
        --n;
    }
    return ret;
}
