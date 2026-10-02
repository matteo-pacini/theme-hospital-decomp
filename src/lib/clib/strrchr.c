#include "clib.h"

// strrchr: last occurrence of (char)c in s (the terminator included), or NULL.
// FUNCTION: 0xce52c
char *strrchr(const char *s, int c)
{
    const char *p;
    char cc = c;

    p = NULL;
    do {
        if (*s == cc)
            p = s;
    } while (*s++ != '\0');
    return (char *)p;
}
