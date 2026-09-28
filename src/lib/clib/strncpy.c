#include "clib.h"

// FUNCTION: 0xce414
char *strncpy(char *dst, const char *src, size_t len)
{
    char *ret;

    ret = dst;
    for (; len; --len) {
        if (*src == '\0')
            break;
        *dst++ = *src++;
    }
    while (len != 0) {
        *dst++ = '\0';
        --len;
    }
    return ret;
}
