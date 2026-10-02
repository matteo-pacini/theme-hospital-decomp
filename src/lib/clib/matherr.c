#include "clib.h"

struct exception;

// matherr: default user math error hook; returns 0 (not handled).
// FUNCTION: 0xfe02f
int matherr(struct exception *excp)
{
    return 0;
}
