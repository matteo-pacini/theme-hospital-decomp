#include "clib.h"

// vprintf: formatted output to stdout with a va_list; returns chars written or -1.
// FUNCTION: 0xcaa79
int vprintf(const char *format, va_list arg)
{
    return __fprtf(stdout, format, arg);
}
