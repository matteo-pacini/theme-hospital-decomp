#include "clib.h"

// FUNCTION: 0xcaa8a
int fflush(FILE *fp)
{
    if (fp == NULL) {
        flushall();
        return 0;
    }
    return __flush(fp);
}
