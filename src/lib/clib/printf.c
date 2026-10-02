#include "clib.h"

// printf: formatted output to stdout; returns chars written or -1.
// FUNCTION: 0xcf40f
int printf(const char *format, ...)
{
    va_list args;

    args[0] = (char *)&format + sizeof(format);
    return __fprtf(stdout, format, args);
}
