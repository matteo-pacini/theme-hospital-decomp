#include "clib.h"

// FUNCTION: 0xcd702
char *strchr(const char *s, int c)
{
    char cc = c;

    do {
        if (*s == cc)
            return (char *)s;
    } while (*s++ != '\0');
    return NULL;
}
