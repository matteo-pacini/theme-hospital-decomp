#include "clib.h"

// FUNCTION: 0xcd72f
char *strstr(const char *s1, const char *s2)
{
    char *end_of_s1;
    size_t s1len;
    size_t s2len;

    if (s2[0] == '\0') {
        return (char *)s1;
    } else if (s2[1] == '\0') {
        return strchr(s1, s2[0]);
    }
    end_of_s1 = memchr(s1, '\0', ~0);
    s2len = strlen(s2);
    for (;;) {
        s1len = end_of_s1 - s1;
        if (s1len < s2len)
            break;
        s1 = memchr(s1, *s2, s1len);
        if (s1 == NULL)
            break;
        if (memcmp(s1, s2, s2len) == 0)
            return (char *)s1;
        ++s1;
    }
    return NULL;
}
