#include "clib.h"

// FUNCTION: 0xcaa79
int vprintf(const char *format, va_list arg)
{
    return __fprtf(stdout, format, arg);
}
