#include "clib.h"

// FUNCTION: 0xcf40f
int printf(const char *format, ...)
{
    va_list args;

    args[0] = (char *)&format + sizeof(format);
    return __fprtf(stdout, format, args);
}
