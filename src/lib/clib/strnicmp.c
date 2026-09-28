#include "clib.h"

// FUNCTION: 0xeb6ae
// NONMATCHING: config/functions.txt extends strnicmp_ over two following
// functions (0xeb701, 0xeb733); strnicmp itself matches
int strnicmp(const char *s, const char *t, size_t n)
{
    char c1;
    char c2;

    for (;;) {
        if (n == 0)
            return 0;
        c1 = *s;
        c2 = *t;
        if ((int)c1 >= 'A' && (int)c1 <= 'Z')
            c1 += 'a' - 'A';
        if ((int)c2 >= 'A' && (int)c2 <= 'Z')
            c2 += 'a' - 'A';
        if (c1 != c2)
            return (unsigned char)c1 - (unsigned char)c2;
        if (c1 == '\0')
            return 0;
        ++s;
        ++t;
        --n;
    }
}
