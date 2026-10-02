#include "clib.h"

// fflush: flush stream fp, or all streams if fp is NULL; returns 0 or EOF.
// FUNCTION: 0xcaa8a
int fflush(FILE *fp)
{
    if (fp == NULL) {
        flushall();
        return 0;
    }
    return __flush(fp);
}
